# tools/dev — Herramientas de desarrollo de BetelineyLauncher

Cada script acá nace de un problema **real** documentado en `ESTADO.md`, no
de una idea genérica. Objetivo: que ese problema no vuelva a pasar, o que
cuando pase se detecte en segundos en vez de sesiones enteras de auditoría.

| Script | Problema real que resuelve | Sesión de origen |
|---|---|---|
| `build_fast.sh` | El build completo con LTO se cuelga en este hardware; `ninja` sin restringir targets bloqueaba la terminal sin dar señal de vida | 20, 27, 29, 31, 32, 33 |
| `check_build.sh` | Sondear un build en background sin bloquear, comparando timestamps binario vs. fuente (patrón que se venía haciendo a mano con `stat`) | 27, 28, 29, 31 |
| `verify_presets.py` | Los 5 mods de los presets built-in tenían URLs 404 en Modrinth; nadie lo detectó hasta una auditoría manual — 100% de instalaciones fallando en silencio | 26 |
| `check_ci.sh` | `gh` resolvía al repo equivocado (`PrismLauncher/PrismLauncher`) tras agregar el remote `upstream`, sin avisar | 20 |
| `check_meta_urls.sh` | `keep_files:false` en el workflow de `meta` borraba todo el contenido Beteliney (packs/malware/news) cada 6h por cron | 15 |
| `secret_scan.sh` + `install_hooks.sh` | La API key de CurseForge quedó commiteada en texto plano y expuesta públicamente | 15 |
| `estado_actual.sh` | `ESTADO.md` pasó las 1300+ líneas; el propio documento pide leer solo la sección "ESTADO ACTUAL" para arrancar, pero nadie automatizó extraerla | — |
| `claude_guard_git_push.sh` | La regla "`git push` requiere autorización explícita" vivía solo como texto en `CLAUDE.md`/cada agente — no determinístico. Hook `PreToolUse` de Claude Code (`.claude/settings.json`), no un git hook: fuerza el prompt de confirmación real ante cualquier `git push`, en este repo o en `meta` | 48 |
| `audit_upstream.sh` | `minecraft/auth/` y `modplatform/` (código 100% heredado de Prism) llevaban 0 y ~0 commits de revisión propia desde el fork respectivamente, sin que ningún documento lo distinguiera de "revisado" — ver `docs/AUDITORIA_MODULOS.md` | — |

## Uso rápido

```bash
# Arrancar una sesión nueva sin leer 900+ líneas
tools/dev/estado_actual.sh

# Compilar sin colgarse (elegí el modo según qué necesitás)
tools/dev/build_fast.sh tests      # BUILD_TESTING=ON, para poder correr ctest
tools/dev/build_fast.sh updater    # solo el updater (Ed25519, etc.)
tools/dev/build_fast.sh full       # launcher completo, en background
tools/dev/check_build.sh           # sondear sin bloquear

# Antes de tocar BetelineyPresets.h, o periódicamente
tools/dev/verify_presets.py

# Antes de asumir "CI está verde" o "gh apunta a donde creo"
tools/dev/check_ci.sh

# Después de tocar el workflow de meta, o para detectar la regresión del cron
tools/dev/check_meta_urls.sh

# Una sola vez, para que nunca más se filtre una key sin querer
tools/dev/install_hooks.sh

# Periódicamente, o cada vez que Prism saque un tag nuevo — para no
# repetir el hueco de auth/modplatform nunca auditados
tools/dev/audit_upstream.sh
```

## Notas de diseño

- Todos los scripts son standalone: no dependen entre sí (salvo
  `build_fast.sh`/`check_build.sh`, que comparten estado vía
  `tools/dev/.buildlogs/`, que está en `.gitignore`).
- `verify_presets.py` no asume nada de la estructura interna del struct
  C++: parsea bloques `{ }` balanceados y busca URL+SHA512 por proximidad,
  así sigue funcionando aunque se agreguen o reordenen campos del struct.
- `secret_scan.sh` corre sobre archivos staged si hay un commit en curso,
  o sobre todo el árbol trackeado si se corre manual — para poder usarlo
  tanto como hook como como auditoría puntual.
