// Beteliney Launcher — Worker de telemetria anonima.
//
// Endpoints:
//   POST /heartbeat  -> el launcher pega esto cada 4 minutos mientras esta abierto.
//   GET  /stats       -> JSON publico de solo lectura, cacheado 60s (para README/badges).
//
// Privacidad: nunca se guarda IP cruda, ni nombre de usuario, ni nada de la
// cuenta de Minecraft. Lo unico "identificable" es el UUID random que genera
// el cliente, mas el pais que resuelve Cloudflare (request.cf.country) — no
// hay geolocalizacion propia ni se toca la IP para nada mas que eso.

const MILESTONES = [100, 250, 500, 1000, 2500, 5000, 10000, 25000, 50000, 100000, 250000, 500000, 1000000];

// Rate limit anti-abuso: un mismo UUID no puede actualizar el estado mas
// seguido que esto. El cliente manda heartbeats cada 4 min, asi que 150s
// (2.5 min) da margen sin abrir la puerta a spam de heartbeats falsos.
const MIN_SECONDS_BETWEEN_HEARTBEATS = 150;

export default {
  async fetch(request, env, ctx) {
    const url = new URL(request.url);

    if (request.method === 'POST' && url.pathname === '/heartbeat') {
      return handleHeartbeat(request, env);
    }
    if (request.method === 'GET' && url.pathname === '/stats') {
      return handleStats(env, ctx);
    }
    return new Response('not found', { status: 404 });
  },
};

async function handleHeartbeat(request, env) {
  let body;
  try {
    body = await request.json();
  } catch {
    return new Response('bad json', { status: 400 });
  }

  const uuid = typeof body.uuid === 'string' ? body.uuid.trim() : '';
  if (!uuid || uuid.length > 64) {
    return new Response('missing or invalid uuid', { status: 400 });
  }

  const version = typeof body.version === 'string' ? body.version.slice(0, 32) : 'desconocida';
  const os = typeof body.os === 'string' ? body.os.slice(0, 128) : 'desconocido';
  const incomingPlaytime = toSafeSeconds(body.playtime_total_seconds);
  const incomingOpen = toSafeSeconds(body.open_total_seconds);
  const country = (request.cf && request.cf.country) || 'XX';
  const now = new Date().toISOString();

  const existing = await env.DB.prepare(
    'SELECT last_seen, last_playtime_seconds, last_open_seconds FROM devices WHERE uuid = ?'
  )
    .bind(uuid)
    .first();

  if (existing) {
    const secondsSinceLastSeen = (Date.now() - Date.parse(existing.last_seen)) / 1000;
    if (secondsSinceLastSeen < MIN_SECONDS_BETWEEN_HEARTBEATS) {
      // Demasiado seguido: probablemente replay o spam. Confirmamos sin tocar nada.
      return new Response('ok', { status: 200 });
    }

    const deltaPlaytime = Math.max(0, incomingPlaytime - existing.last_playtime_seconds);
    const deltaOpen = Math.max(0, incomingOpen - existing.last_open_seconds);

    await env.DB.batch([
      env.DB.prepare(
        'UPDATE devices SET last_seen = ?, version = ?, os = ?, country = ?, last_playtime_seconds = ?, last_open_seconds = ? WHERE uuid = ?'
      ).bind(now, version, os, country, incomingPlaytime, incomingOpen, uuid),
      env.DB.prepare('UPDATE totals SET value = value + ? WHERE key = ?').bind(deltaPlaytime, 'total_playtime_seconds'),
      env.DB.prepare('UPDATE totals SET value = value + ? WHERE key = ?').bind(deltaOpen, 'total_open_seconds'),
    ]);
  } else {
    // Primera vez que vemos este UUID: instalacion nueva. El playtime/open
    // time que ya traiga se usa como linea base (delta 0) para no inflar
    // de golpe los contadores globales con horas jugadas antes de tener
    // telemetria instalada. A partir de aqui, solo cuenta lo que sume desde ahora.
    await env.DB.prepare(
      'INSERT INTO devices (uuid, first_seen, last_seen, version, os, country, last_playtime_seconds, last_open_seconds) VALUES (?, ?, ?, ?, ?, ?, ?, ?)'
    )
      .bind(uuid, now, now, version, os, country, incomingPlaytime, incomingOpen)
      .run();

    await checkMilestone(env);
  }

  return new Response('ok', { status: 200 });
}

function toSafeSeconds(value) {
  if (typeof value !== 'number' || !Number.isFinite(value) || value < 0) return 0;
  return Math.floor(value);
}

async function checkMilestone(env) {
  const countRow = await env.DB.prepare('SELECT COUNT(*) as c FROM devices').first();
  const count = countRow ? countRow.c : 0;

  const lastRow = await env.DB.prepare("SELECT value FROM totals WHERE key = 'last_milestone'").first();
  const lastMilestone = lastRow ? lastRow.value : 0;

  const next = MILESTONES.find((m) => m > lastMilestone && count >= m);
  if (!next) return;

  await env.DB.prepare("UPDATE totals SET value = ? WHERE key = 'last_milestone'").bind(next).run();

  if (env.DISCORD_WEBHOOK_URL) {
    await fetch(env.DISCORD_WEBHOOK_URL, {
      method: 'POST',
      headers: { 'content-type': 'application/json' },
      body: JSON.stringify({
        content: `🎉 **Beteliney Launcher** acaba de llegar a **${next.toLocaleString('es-CO')} instalaciones historicas**. ¡Gracias a toda la comunidad!`,
      }),
    });
  }
}

async function handleStats(env, ctx) {
  const cache = caches.default;
  const cacheKey = new Request('https://beteliney-telemetry.internal/stats-cache');
  const cached = await cache.match(cacheKey);
  if (cached) return cached;

  const nowMs = Date.now();
  const onlineThreshold = new Date(nowMs - 5 * 60 * 1000).toISOString();
  const d1Threshold = new Date(nowMs - 24 * 60 * 60 * 1000).toISOString();
  const d7Threshold = new Date(nowMs - 7 * 24 * 60 * 60 * 1000).toISOString();
  const d30Threshold = new Date(nowMs - 30 * 24 * 60 * 60 * 1000).toISOString();

  const [installsTotal, onlineNow, active1d, active7d, active30d, totalsRows, byVersion, byCountry] = await Promise.all([
    env.DB.prepare('SELECT COUNT(*) as c FROM devices').first(),
    env.DB.prepare('SELECT COUNT(*) as c FROM devices WHERE last_seen > ?').bind(onlineThreshold).first(),
    env.DB.prepare('SELECT COUNT(*) as c FROM devices WHERE last_seen > ?').bind(d1Threshold).first(),
    env.DB.prepare('SELECT COUNT(*) as c FROM devices WHERE last_seen > ?').bind(d7Threshold).first(),
    env.DB.prepare('SELECT COUNT(*) as c FROM devices WHERE last_seen > ?').bind(d30Threshold).first(),
    env.DB.prepare("SELECT key, value FROM totals WHERE key IN ('total_playtime_seconds','total_open_seconds')").all(),
    env.DB.prepare('SELECT version, COUNT(*) as c FROM devices GROUP BY version ORDER BY c DESC LIMIT 10').all(),
    env.DB.prepare('SELECT country, COUNT(*) as c FROM devices GROUP BY country ORDER BY c DESC LIMIT 15').all(),
  ]);

  const totalsMap = Object.fromEntries(totalsRows.results.map((r) => [r.key, r.value]));

  const payload = {
    generated_at: new Date(nowMs).toISOString(),
    installs_total: installsTotal.c,
    online_now: onlineNow.c,
    active_last_24h: active1d.c,
    active_last_7d: active7d.c,
    active_last_30d: active30d.c,
    total_playtime_hours: Math.floor((totalsMap.total_playtime_seconds || 0) / 3600),
    total_launcher_open_hours: Math.floor((totalsMap.total_open_seconds || 0) / 3600),
    by_version: Object.fromEntries(byVersion.results.map((r) => [r.version || 'desconocida', r.c])),
    by_country: Object.fromEntries(byCountry.results.map((r) => [r.country || 'XX', r.c])),
  };

  const response = new Response(JSON.stringify(payload), {
    headers: {
      'content-type': 'application/json',
      'cache-control': 'public, max-age=60',
      'access-control-allow-origin': '*',
    },
  });

  ctx.waitUntil(cache.put(cacheKey, response.clone()));
  return response;
}
