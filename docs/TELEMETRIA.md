# Telemetria anonima (Beteliney Fase 4)

## Que es y por que existe

El launcher manda un "heartbeat" (latido) anonimo cada 4 minutos mientras
esta abierto, a un Worker propio de Cloudflare. El objetivo es poder
responder, con datos reales y no estimaciones, tres preguntas:

1. Cuanta gente usa el launcher (instalaciones historicas, para celebrar
   hitos con la comunidad).
2. Cuanta gente lo esta usando ahora mismo (online en este momento).
3. Cuanto tiempo, en total y sumado entre todos los usuarios, se ha jugado
   Minecraft a traves del launcher — y por separado, cuanto tiempo ha
   estado el launcher mismo abierto (dos metricas distintas, ver mas abajo).

Esta activado por defecto (con aviso una sola vez en el primer arranque) y
se puede apagar en **Configuracion > Launcher**.

## Que se manda y que NO se manda

Cada heartbeat es un POST JSON con exactamente esto:

```json
{
  "uuid": "b3f1...-...-...",
  "version": "8.4.0",
  "os": "Ubuntu 24.04",
  "playtime_total_seconds": 123456,
  "open_total_seconds": 7890
}
```

- `uuid`: generado localmente con `QUuid::createUuid()` la primera vez que
  arranca el launcher, guardado en la config local. Es random — no se
  deriva de hardware, ni de MAC, ni de nada identificable de la maquina.
- `version` / `os`: para saber que versiones siguen en uso y decidir cuando
  dejar de dar soporte a una version vieja.
- `playtime_total_seconds`: NO se manda la IP del usuario, ni su nombre de
  usuario de Minecraft, ni el nombre de sus instancias/modpacks, ni su
  cuenta. El Worker tampoco guarda la IP cruda en ningun lado — lo unico
  que se deriva de la conexion es el pais, que Cloudflare resuelve solo
  (`request.cf.country`) sin que el codigo del Worker toque la IP para nada.

## Las dos metricas de tiempo (y por que son distintas)

Esto es importante porque son cosas conceptualmente distintas, no la misma
metrica medida dos veces:

- **`playtime_total_seconds`** = tiempo jugado *dentro de Minecraft*. Es la
  suma de `totalTimePlayed()` de todas las instancias del usuario — un dato
  que el launcher ya guarda hace rato (`BaseInstance.cpp`), por cada
  instancia, cada vez que cierra una sesion de juego. Telemetria solo lo
  suma y lo reporta, no inventa nada nuevo del lado del juego.
- **`open_total_seconds`** = tiempo que el *launcher* ha estado abierto
  (viendo la lista de instancias, descargando modpacks, etc.), independiente
  de si hay una instancia de Minecraft corriendo o no. Este es un contador
  nuevo (`TelemetryTotalOpenSeconds`), que `Telemetry::accumulateOpenSeconds()`
  va sumando cada vez que se manda un heartbeat, comparando contra la ultima
  vez que se tomo el tiempo (`m_lastTick`). Se resetea el punto de referencia
  (no el contador acumulado) cada vez que se re-habilita telemetria tras
  haberla apagado, para no contarle al usuario como "abierto" el rato que
  tuvo telemetria desactivada.

Ambas viajan como **snapshot acumulado de por vida en esa maquina**, no como
delta. El delta lo calcula el Worker (ver abajo) comparando contra el ultimo
valor que le vio a ese UUID — asi un mismo heartbeat nunca duplica cuenta,
y si el usuario resetea el contador de tiempo jugado de una instancia
(`resetTimePlayed()`), el peor caso es que ese heartbeat sume delta 0, nunca
resta del total global.

## Arquitectura del backend

Cloudflare Workers + D1 (SQLite gestionado por Cloudflare). Elegido por
sobre un VPS propio porque no requiere mantener un servidor prendido, tiene
tier gratuito mas que suficiente para este volumen, y esta en el mismo
edge donde ya resuelve el pais sin trabajo extra.

Codigo fuente completo en `telemetry-worker/`:

```
telemetry-worker/
├── package.json      -> scripts npm (dev, deploy, migraciones de D1)
├── wrangler.toml      -> config del Worker + binding a la base D1
├── schema.sql          -> DDL: tablas devices y totals
└── src/index.js       -> los dos endpoints, /heartbeat y /stats
```

### Tabla `devices` (una fila por UUID)

| columna                  | que guarda                                          |
|--------------------------|------------------------------------------------------|
| uuid                     | PK, el UUID random del cliente                        |
| first_seen / last_seen   | timestamps ISO, para online-ahora y activos-por-ventana |
| version / os / country   | ultimo valor visto                                    |
| last_playtime_seconds    | ultimo snapshot de playtime que mando ese UUID         |
| last_open_seconds        | ultimo snapshot de tiempo-launcher-abierto             |

### Tabla `totals` (contadores globales, solo tres filas)

`total_playtime_seconds`, `total_open_seconds`, `last_milestone` — este
ultimo evita que el webhook de Discord dispare mas de una vez por hito.

### `POST /heartbeat`

1. Valida que venga `uuid` (si no, 400).
2. Rate limit anti-abuso: si ese UUID ya actualizo hace menos de 150s,
   responde 200 sin tocar nada — evita que alguien infle instalaciones o
   gaste las escrituras gratis de D1 pegandole al endpoint en loop.
3. Si el UUID es nuevo: se inserta como instalacion nueva, usando el
   playtime/open-time que traiga como **linea base** (delta 0) — asi no se
   infla de golpe el contador global con horas jugadas antes de instalar
   esta version con telemetria. Dispara `checkMilestone()`.
4. Si ya existia: calcula `delta = nuevo - ultimo_conocido` (nunca negativo)
   para playtime y para open-time, los suma a `totals`, y actualiza la fila.

### `GET /stats` (publico, de solo lectura)

Devuelve JSON cacheado 60s (Cache API de Cloudflare, para no pegarle a D1
en cada refresh de un badge):

```json
{
  "installs_total": 812,
  "online_now": 14,
  "active_last_24h": 96,
  "active_last_7d": 210,
  "active_last_30d": 340,
  "total_playtime_hours": 15234,
  "total_launcher_open_hours": 21980,
  "by_version": { "8.4.0": 640, "8.3.2": 90 },
  "by_country": { "CO": 300, "MX": 120, "AR": 80 }
}
```

`installs_total` es la cifra de "instalaciones historicas" para celebrar
hitos. `active_last_30d` es la de "usuarios activos reales" — la que le
importa a un CPU si el proyecto crece o se estanca, distinta de
instalaciones-historicas porque esa solo sube.

### Webhook de Discord en hitos

Cuando `installs_total` cruza uno de estos numeros por primera vez — 100,
250, 500, 1000, 2500, 5000, 10000, 25000, 50000, 100000, 250000, 500000,
1000000 — el Worker mismo manda un mensaje al webhook configurado en
`DISCORD_WEBHOOK_URL` (secret, no queda en texto plano en ningun archivo).
`last_milestone` en la tabla `totals` evita que se repita el aviso.

## Deploy (pendiente, requiere tu cuenta de Cloudflare)

Esto es lo unico que no pude dejar hecho del todo porque necesita tu login
interactivo — no es algo que deba automatizarse sin que vos lo confirmes.
Los pasos, en orden, parado en `telemetry-worker/`:

```bash
cd telemetry-worker
npm install                      # instala wrangler como devDependency
npx wrangler login               # abre el navegador, autoriza tu cuenta

npx wrangler d1 create beteliney-telemetry
# copia el "database_id" que imprime y pegalo en wrangler.toml,
# reemplazando "REEMPLAZAR-DESPUES-DE-CREAR-LA-DB"

npm run db:migrate:remote        # crea las tablas en la D1 real
npx wrangler secret put DISCORD_WEBHOOK_URL   # pega la URL del webhook cuando lo pida

npm run deploy                   # deploya el Worker
```

El ultimo comando imprime la URL final, algo como
`https://beteliney-telemetry.TU-SUBDOMINIO.workers.dev`. Con esa URL:

1. Reemplazar `kHeartbeatUrl` en
   `launcher/BetelineyTelemetry.cpp` (esta marcado con un `TODO`).
2. Recompilar.
3. Para el badge del README, usar shields.io con un endpoint badge
   apuntando a `https://TU-WORKER.workers.dev/stats` (formato "endpoint",
   ver https://shields.io/badges/dynamic-json-badge).

Antes de deployar en serio, se puede probar local con `npm run dev`
(levanta el Worker + una D1 local en disco) y `npm run db:migrate:local`
para esa base local — no toca la base de produccion ni gasta cuota real.

## Pendiente para una proxima sesion (sugerido, no implementado aun)

- **`DELETE /device` al desactivar telemetria**: hoy, si el usuario apaga el
  switch en Configuracion, el launcher simplemente deja de mandar
  heartbeats, pero su fila en `devices` se queda para siempre en la D1. Lo
  coherente es que `Telemetry::setEnabled(false)` mande un ultimo request
  de borrado y el Worker elimine esa fila por completo (no solo dejar de
  contarla). Baja el numero de instalaciones historicas cuando alguien se
  da de baja, lo cual es correcto: ese numero debe reflejar gente que sigue
  de acuerdo con ser contada.
- **Cloudflare Rate Limiting Rules** (dashboard, no codigo): la proteccion
  anti-abuso que ya tiene el Worker (ignorar heartbeats del mismo UUID a
  menos de 150s) no frena a alguien mandando miles de UUIDs random
  distintos por segundo. Una regla nativa de Cloudflare tipo "mas de 20
  requests/min por IP a /heartbeat -> bloquear 10 min" es defensa en
  profundidad barata y se configura en un minuto sin tocar `index.js`.
- **Pagina HTML publica en `GET /`** del mismo Worker, que renderice el
  JSON de `/stats` en algo legible (instalaciones, online ahora, horas
  jugadas) — util como link directo para compartir en Discord sin armar un
  dashboard aparte.

Explicitamente descartado por ahora: tracking por instancia/modpack (cruza
la linea de "ya no es anonimo, es un perfil de uso") y cron de limpieza de
filas viejas (no hace falta hasta que el volumen sea mucho mayor).

## Validado hasta ahora

- Cliente (`BetelineyTelemetry.h/.cpp`, hook en `MainWindow.cpp`, checkbox
  en Configuracion > Launcher) compila limpio contra `build-dev`.
- Worker: `node --check` sobre `src/index.js` sin errores de sintaxis, y
  `wrangler deploy --dry-run` valida el binding a D1 y el bundle sin
  necesitar login.
- Pendiente de validar en caliente: el primer deploy real y un heartbeat
  de punta a punta contra el Worker ya deployado.
