# Auditoría de deuda técnica — BetelineyLauncher (2026-08-20)

Auditor: Claude (claude.ai, chat web). Rol: solo relevamiento — nada del código fue modificado.

## Alcance

El repo tiene 841 archivos `.cpp/.h` en `launcher/` (122.622 líneas), en su gran mayoría heredados
de Prism Launcher sin modificar. `docs/AUDITORIA_MODULOS.md` ya cubre en profundidad la divergencia
contra el upstream (217 archivos con cambios reales, script `tools/dev/audit_file_divergence.sh`).
Esta auditoría no repite ese trabajo: se enfocó en **código propio** — los ~60 archivos con prefijo
`Beteliney*`, los archivos de features específicas del fork sin ese prefijo (`KonamiCode`,
`FavoriteServers`, `CommandPaletteDialog`), `tools/dev/`, `telemetry-worker/` y la correspondencia
entre esos archivos y `docs/`. El código vendorizado sin tocar quedó fuera de alcance, tal como pide
el encargo.

## Método aplicado

Inventario de archivos `Beteliney*` (find), triage por patrón (TODO/FIXME/XXX/placeholder/stub),
verificación cruzada de cada sospechoso con `grep` sobre todo el árbol + confirmación de inclusión
real en `launcher/CMakeLists.txt` (el repo no usa `file(GLOB)` para fuentes — todo se lista a mano,
así que "no está en CMakeLists" es evidencia real de "no compila", no una suposición), y `git log`/
`git blame` para fechar cada hallazgo y decidir si es deuda propia o heredada del commit de fork
(`09eb67f74`, 2026-06-10).

## Tabla resumen

| Archivo | Categoría | Severidad | Confianza |
|---|---|---|---|
| `launcher/BetelineyCode.h` | Muerto/huérfano + a medias | Baja | Alta |
| `launcher/ui/themes/BetelineyTheme.cpp.v3bak`, `.v4bak` | Anomalía (basura versionada) | Baja | Alta |
| `launcher/BetelineyTelemetry.cpp` (`kHeartbeatUrl`) | A medias (feature rota en runtime) | Media | Alta |
| `launcher/icons/BetelineyIcon.cpp:85` (`// FIXME: inject this.`) | Heredado, informativo | Muy baja | Alta |

## Detalle por hallazgo

### 1. `launcher/BetelineyCode.h` — clase declarada, nunca implementada ni conectada

**Qué es:** header con `class BetelineyCode : public QObject` (Q_OBJECT, señal `triggered()`,
método `input(QEvent*)`), con el comentario "Easter egg: escribe B-E-T-E en la ventana principal
para activarlo."

**Evidencia:**
- No existe `BetelineyCode.cpp` en ningún lado del árbol.
- No está en `launcher/CMakeLists.txt` (verificado contra los 462 headers que sí están listados
  explícitamente ahí — la ausencia es real, no un patrón de omisión normal para clases con Q_OBJECT).
- `grep -rn "BetelineyCode"` en todo `launcher/` y `tests/` no devuelve ninguna referencia fuera del
  propio header.
- `git log --follow` para este archivo tiene un único commit: `09eb67f74`, el commit inicial del
  fork (2026-06-10) — nunca se volvió a tocar.
- La easter egg que sí está viva y conectada es la heredada de upstream, sin renombrar:
  `launcher/KonamiCode.h/.cpp`, incluida en `MainWindow.cpp` (línea 131), instanciada en el
  constructor (línea 315) y conectada a `MainWindow::konamiTriggered()` (línea 571). Su interfaz
  (`QObject`, `input(QEvent*)`, señal `triggered()`, `m_progress`) es prácticamente idéntica a la de
  `BetelineyCode`, lo que sostiene la inferencia de abajo.

**Inferencia (confianza alta):** el día del fork se empezó a reemplazar el Konami Code heredado por
una versión con marca propia ("B-E-T-E" en vez de la secuencia de flechas clásica), se declaró la
interfaz calcada de `KonamiCode`, y se abandonó antes de escribir el `.cpp` o de sacar el original.
El resultado es que hoy conviven un easter egg funcional pero sin rebrandear (`KonamiCode`) y un
easter egg rebrandeado pero inexistente en tiempo de ejecución (`BetelineyCode`).

**Impacto real:** ninguno — el archivo no se compila (ni MOC lo procesa), así que no hay riesgo de
build ni de comportamiento inesperado. El costo es solo confusión para quien lea el código y asuma
que la marca ya está aplicada al easter egg.

**Pendiente de decidir:** o se termina `BetelineyCode` (escribir el `.cpp`, sumarlo a CMakeLists,
reemplazar `KonamiCode` en `MainWindow`) o se borra el header y se deja el Konami Code heredado tal
cual está.

---

### 2. `BetelineyTheme.cpp.v3bak` / `.v4bak` — backups manuales versionados en git

**Qué es:** dos copias completas de una versión vieja de `BetelineyTheme.cpp` (731 y 735 líneas,
contra 844 del archivo actual), con extensión `.v3bak`/`.v4bak`, trackeadas en git
(`git ls-files` las lista).

**Evidencia:** `git log --oneline` para ambos archivos devuelve un único commit, `09eb67f74`
(commit de fork, 2026-06-10) — llegaron con el fork y nadie las tocó ni las borró en más de dos
meses de desarrollo activo (66+ sesiones documentadas en `ESTADO.md`). No aparecen referenciadas en
ningún script de build ni de packaging.

**Inferencia (confianza alta):** son backups manuales hechos antes de una edición grande de
`BetelineyTheme.cpp` (probablemente durante el trabajo de tokenización de colores mencionado en
`PENDIENTES.md` § 4), commiteados por costumbre o por accidente (`git add -A`) y nunca limpiados. Git
ya guarda el historial completo del archivo real — estas copias son estrictamente redundantes con
`git log`/`git show` de versiones anteriores.

**Impacto real:** ninguno funcional. Sí es ruido real: dos archivos de ~730 líneas cada uno inflando
el repo y confundiendo cualquier búsqueda por nombre de archivo (`BetelineyTheme.cpp*` da 3
resultados en vez de 1).

**Pendiente de decidir:** borrarlas (`git rm`) — no hay ningún motivo técnico para conservarlas que
no cumpla ya el historial de git.

---

### 3. `BetelineyTelemetry.cpp` — `kHeartbeatUrl` sigue en placeholder, feature rota en runtime hoy

**Qué es:** la URL a la que el cliente manda el heartbeat cada 4 minutos sigue siendo
`https://beteliney-telemetry.REEMPLAZAR.workers.dev/heartbeat` (línea 32), un dominio que no
resuelve.

**Evidencia:** `grep` confirma el literal `REEMPLAZAR` todavía en el código en esta fecha. El propio
`docs/TELEMETRIA.md` documenta esto honestamente como pendiente ("esta marcado con un `TODO`"), así
que no es un hallazgo nuevo — pero sigue activo y ya está compilado en el binario actual con
telemetría "activada por defecto" según el mismo doc.

**Por qué entra en esta auditoría igual:** aunque ya está documentado como pendiente, es el único
hallazgo de la lista con severidad real más allá de "ruido de repo" — mientras el Worker no esté
deployado, cada usuario que corra el launcher va a intentar un `POST` contra un dominio inexistente
cada 4 minutos, en silencio (el checkbox de Configuración dice "Enviar estadísticas de uso anónimas"
sin indicar que hoy no llega a ningún lado). No rompe nada visible, pero es exactamente el patrón de
"algo que aparenta funcionar y no funciona" que pide vigilar el encargo.

**Pendiente:** acción manual tuya (login de Cloudflare) ya descrita paso a paso en
`docs/TELEMETRIA.md` — no hace falta repetirla acá.

---

### 4. `launcher/icons/BetelineyIcon.cpp:85` — `// FIXME: inject this.` heredado

**Qué es:** comentario FIXME dentro de `BetelineyIcon::icon()` (clase renombrada de `MMCIcon` de
upstream).

**Evidencia:** `git blame` marca esta línea como del commit de fork `09eb67f74` — es decir, vino tal
cual de Prism Launcher junto con el resto de la clase, solo cambió el nombre del archivo/clase, no el
contenido de esa línea.

**Por qué entra igual, con severidad mínima:** el encargo pidió excluir código de terceros sin
modificar del alcance, pero este archivo sí fue "tocado" (renombrado y adaptado), así que lo dejo
anotado para que quede explícito que este FIXME específico no es deuda que generó este proyecto —
es deuda que heredó. No amerita acción de acá.

---

## Revisado sin hallazgos

Verificado con la misma metodología (grep de patrones sospechosos + cruce contra CMakeLists +
referencias en el resto del árbol), sin encontrar nada que reportar:

- **`launcher/BetelineyProfiles.h`** (258 líneas, sin `.cpp`): no está en CMakeLists, pero no tiene
  `Q_OBJECT` — es un header puro de constantes/funciones inline, incluido directamente desde
  `Application.cpp`, `ui/widgets/JavaSettingsWidget.h` y `tests/BetelineyProfiles_test.cpp`. La
  ausencia de entrada en CMakeLists es normal para este patrón, no un síntoma de nada.
- **`launcher/crash/BetelineyPanicHandler.{h,cpp}`**: incluido y usado en `main.cpp` y
  `Application.cpp`. Sin TODO/FIXME.
- **`launcher/logs/BetelineyLogAnalyzer.{h,cpp}`**: sin TODO/FIXME/placeholder.
- **`launcher/FavoriteServers.{h,cpp}`, `ui/dialogs/FavoriteServersDialog.*`,
  `ui/dialogs/CommandPaletteDialog.*`**: sin TODO/FIXME/placeholder en ninguno.
- **`launcher/updater/BetelineyExternalUpdater.*` + `launcher/updater/betelineyupdater/*`**: en un
  primer vistazo parecían dos implementaciones de updater duplicadas; no lo son —
  `BetelineyExternalUpdater` es la interfaz que usa la app principal (junto a `MacSparkleUpdater`,
  ambas heredan de `ExternalUpdater`), y `betelineyupdater/` es el código fuente de un *binario
  aparte* (tiene su propio `updater_main.cpp`, firmado Ed25519 vía `UpdateVerify.cpp`) que la app
  lanza como proceso externo. Arquitectura correcta, no duplicación. Sin TODO/FIXME.
- **`launcher/resources/beteliney/` vs `launcher/resources/beteliney-icons/`**: en un primer vistazo
  parecían recursos de íconos duplicados (nombres de archivo repetidos entre ambos: `star.svg`,
  `viewfolder.svg`, etc.). No lo son — `beteliney/` es el *tema* de íconos completo (`index.theme` +
  SVG escalables, registrado como `IconTheme` en `Application.cpp:656`) y `beteliney-icons/` es un
  set aparte de íconos legacy por instancia (bloques, mobs), referenciado directo por ruta
  (`:/icons/beteliney-icons/32x32/instances/...`, `Application.cpp:1020`). Ambos `.qrc` están
  compilados de verdad (`launcher/CMakeLists.txt:1360-1361`). Sin hallazgo, aunque la superposición
  de nombres entre ambos árboles puede confundir a alguien que no sepa que son dos sistemas
  distintos — no lo tipifico como bug, es una observación menor de legibilidad.
- **`launcher/modplatform/beteliney/*` y `ui/pages/modplatform/beteliney/*`** (pack browsing):
  sin TODO/FIXME/placeholder.
- **`launcher/BetelineyAchievements.{h,cpp}`**: sin TODO/FIXME. No se verificó en esta pasada que el
  hook a `BaseInstance::setMinecraftRunning` esté intacto (documentado en `ESTADO.md`, no
  re-confirmado acá) — si se quiere blindar, es la siguiente pasada obvia.
- **`docs/TELEMETRIA.md` contra `BetelineyTelemetry.{h,cpp}`**: alineación verificada línea por
  línea en los puntos técnicos citados (`accumulateOpenSeconds()`, `TelemetryTotalOpenSeconds`,
  `m_lastTick`, `setEnabled()`) — el doc describe exactamente lo que hace el código, incluida la
  parte pendiente (`DELETE /device`, correctamente marcada como "no implementado aun" en el propio
  doc, no como hecho).
- **`tools/dev/*.sh`, `tools/dev/*.py`**: sin TODO/FIXME/HACK reales (los matches de `grep` fueron
  falsos positivos de palabras como "TODOS" o nombres de archivo con "FILOSOFIA").
- **`launcher/BetelineyCode.h` vs `launcher/BetelineyProfiles.h`** — verificado que ningún otro
  archivo con prefijo `Beteliney*` tiene el mismo patrón de "declarado, sin `.cpp`, sin CMakeLists,
  sin referencias" — `BetelineyCode.h` (hallazgo 1) es el único caso real de esta categoría.

---

## Fuera de alcance en esta pasada, explícito

No se re-auditó nada del código vendorizado sin modificar (la gran mayoría de los 841 archivos de
`launcher/`) — eso ya lo cubre `docs/AUDITORIA_MODULOS.md` con metodología propia (diff contra el
tag `11.0.3` de Prism Launcher) y no tenía sentido duplicarlo. Tampoco se bajó a nivel de línea en
`libraries/`, en los ~217 archivos con divergencia real ya catalogados ahí, ni en el hook de
`BetelineyAchievements` contra `BaseInstance::setMinecraftRunning` (mencionado arriba). Si en algún
momento se quiere una pasada de esta misma metodología pero sobre esos 217 archivos específicamente,
es la extensión natural de este documento.

## Cierre

No encontré nada nuevo con severidad alta — lo más filoso (el segfault de exit 139, path traversal
sin corregir en Flame/ATL) ya está identificado y trackeado en `ESTADO.md`/`PENDIENTES.md`, fuera del
alcance de código que pedía esta auditoría (esos dos ya tienen dueño y contexto propio). Los cuatro
hallazgos de acá son deuda de bajo impacto real, útil para limpiar cuando haya tiempo: dos archivos
de basura versionada sin costo de borrar (#1, #2), una feature ya conocida como incompleta pero cuyo
efecto en runtime (llamadas de red que fallan en silencio) vale la pena tener presente (#3), y un
FIXME heredado sin acción posible desde este proyecto (#4).

---

## Continuación de la auditoría (misma sesión, segunda pasada)

Seguí bajando con la misma metodología a áreas no cubiertas en la primera pasada: el resto de
features "Fase 3" (logros, sandboxing), `tools/dev/` completo (11 scripts), `packaging/`, `scripts/`,
`telemetry-worker/`, consistencia de versión entre `CMakeLists.txt`/tags/PKGBUILD, y los skills/agentes
de `.claude/`.

### Revisado sin hallazgos (ronda 2)

- **`BaseInstance::setMinecraftRunning` → `Beteliney::Achievements::instance().checkInstance(this)`**
  (línea 289 de `BaseInstance.cpp`): hook confirmado intacto, se dispara al terminar cada sesión de
  juego, justo después de actualizar `totalTimePlayed`. `AchievementToast` está instanciado en
  `MainWindow.cpp:321` y ambos (`AchievementToast.*`, `BubblewrapSandbox.*`) están en
  `CMakeLists.txt`. Sin TODO/FIXME en ninguno de los cuatro archivos.
- **`BubblewrapSandbox`**: usado de verdad en `LauncherPartLaunch.cpp`, `MinecraftInstance.cpp`,
  `Application.cpp` y `MinecraftSettingsWidget.cpp` — no es un feature a medias, está integrado en
  la cadena de lanzamiento real.
- **`telemetry-worker/`** (Cloudflare Worker en JS + esquema SQL): sin TODO/FIXME en el código del
  worker en sí — el pendiente real (deploy manual) vive del lado del cliente (hallazgo #3 de arriba),
  no en este directorio.
- **`tools/dev/`** (11 scripts + `verify_presets.py`): los tres matches de "TODO/FIXME" fueron falsos
  positivos de texto (`compila TODOS`, `borraba TODO el`), no marcadores reales. Verifiqué además que
  cada script está referenciado desde al menos otro lado del repo (otro script, un skill de `.claude/`,
  o `ESTADO.md`/`PENDIENTES.md`/`CLAUDE.md`) — ninguno quedó huérfano.
- **`packaging/aur/PKGBUILD`**: en un primer vistazo `pkgver=8.4.0.r0.g0000000` parecía un placeholder
  sin completar. No lo es — es la convención estándar de Arch para paquetes `-git` (la línea estática
  es solo declarativa; `pkgver()` la recalcula en cada build con `git describe`). El propio archivo
  ya trae una nota extensa explicando por qué el tag `v8.4.0` quedó huérfano (no es ancestro de
  `HEAD` tras un force-push documentado en `ESTADO.md`, Sesión 40) y por qué el fallback usa
  `git rev-list --count` en vez de romper. Sin hallazgo — es, si acaso, un ejemplo de cómo documentar
  bien una decisión no obvia.
- **`packaging/com.beteliney.BetelineyLauncher.json`** (manifest Flatpak): `runtime-version` en
  `6.6` corresponde al runtime KDE, no a la versión de la app — no hay conflicto con
  `Launcher_VERSION_MAJOR/MINOR` de `CMakeLists.txt` (8/4), son ejes distintos.
- **Consistencia de versión general**: `CMakeLists.txt` (8.4.x), último tag real (`v8.4.0`) y
  `ESTADO.md` (hasta Sesión 66) están alineados entre sí.
- **`.claude/skills/*/SKILL.md`** (5 skills) y **`.claude/agents/*.md`** (6 agentes): ningún link roto
  hacia scripts de `tools/dev/` — se verificó cada referencia contra el filesystem real.

### Nota fuera de alcance, no un hallazgo de código

`ESTADO.md` (Sesión 66 cont., 2026-08-20/21) documenta un pendiente propio sobre
`referencia-competencia/` (carpeta de investigación de launchers competidores, fuera de `source/` —
no es código del proyecto) que creció de 17 a 55 archivos con contenido de riesgo variable (un
cliente de cheats, posibles generadores de cuentas, duplicados). Ya está señalado ahí con contexto
completo, sin acción tomada a propósito ("el dueño lo va a trabajar en otra sesión aparte"). No
corresponde re-abrirlo en una auditoría de deuda técnica de código — lo dejo solo anotado acá para que
quede trazado que esta auditoría lo vio y decidió no tocarlo, no que lo pasó por alto.

### Pendientes que deja esta auditoría (para vos, ninguno urgente)

1. Decidir destino de `launcher/BetelineyCode.h` (hallazgo 1): terminarlo o borrarlo.
2. `git rm` de `BetelineyTheme.cpp.v3bak` y `.v4bak` (hallazgo 2) — cero motivo técnico para
   conservarlos, git ya tiene el historial.
3. Deploy del Worker de Cloudflare para sacar `kHeartbeatUrl` del placeholder (hallazgo 3) — ya
   documentado paso a paso en `docs/TELEMETRIA.md`, no hace falta nada nuevo acá.
4. Opcional, bajo impacto: si en algún momento se toca `BetelineyIcon.cpp`, aprovechar para resolver
   el FIXME heredado de la línea 85 (hallazgo 4) — no amerita una sesión dedicada solo por esto.
