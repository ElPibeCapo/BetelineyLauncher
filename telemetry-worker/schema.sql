-- Beteliney Launcher: telemetria anonima de uso.
-- Una fila por UUID (un UUID = una instalacion del launcher, generado
-- localmente por el cliente, nunca vinculado a una cuenta de Minecraft).

CREATE TABLE IF NOT EXISTS devices (
  uuid TEXT PRIMARY KEY,
  first_seen TEXT NOT NULL,
  last_seen TEXT NOT NULL,
  version TEXT,
  os TEXT,
  country TEXT,
  last_playtime_seconds INTEGER NOT NULL DEFAULT 0,
  last_open_seconds INTEGER NOT NULL DEFAULT 0
);

CREATE INDEX IF NOT EXISTS idx_devices_last_seen ON devices(last_seen);
CREATE INDEX IF NOT EXISTS idx_devices_version ON devices(version);
CREATE INDEX IF NOT EXISTS idx_devices_country ON devices(country);

-- Contadores globales que solo crecen (delta por heartbeat, nunca se recalculan desde cero).
CREATE TABLE IF NOT EXISTS totals (
  key TEXT PRIMARY KEY,
  value INTEGER NOT NULL DEFAULT 0
);

INSERT OR IGNORE INTO totals (key, value) VALUES
  ('total_playtime_seconds', 0),
  ('total_open_seconds', 0),
  ('last_milestone', 0);
