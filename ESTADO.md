# ESTADO — BetelineyLauncher
> Documento único y autocontenido. Cualquier chat nuevo lee SOLO la sección '## ESTADO ACTUAL' de abajo y puede continuar. El resto es historial detallado por sesión: útil para auditar, no necesario para arrancar.
> Última reorganización: 2026-07-08 (sesión 33) — se consolidaron 7 bloques repetidos de 'ESTADO CONSOLIDADO' en uno solo, movido al tope. Ningún contenido técnico de ninguna sesión fue eliminado ni modificado, solo la repetición boilerplate entre bloques.

---

## IDENTIDAD DEL PROYECTO

| | |
|---|---|
| **Nombre** | BetelineyLauncher |
| **Versión actual en código** | 8.4.0 (`CMakeLists.txt` líneas 179-181) |
| **Última release** | `v8.4.0`, tageada y publicada en GitHub Releases (sesión 27, 2026-07-07) — https://github.com/ElPibeCapo/BetelineyLauncher/releases/tag/v8.4.0. El tag en sí solo tiene el bump de versión; los commits reales posteriores (sesiones 27-39, sin release nueva aún) están listados en `## HISTORIAL DE COMMITS` y detallados en `docs/CHANGELOG.md`. Fuente de verdad real: `git log --oneline`. |
| **Base** | Prism Launcher (GPL-3.0), fork extensamente modificado |
| **Autor** | El_PibeCapo — `elpibecapoofficial@gmail.com` |
| **Repo launcher** | https://github.com/ElPibeCapo/BetelineyLauncher |
| **Repo meta** | https://github.com/ElPibeCapo/meta |
| **Rama principal** | `main` |
| **META server** | https://ElPibeCapo.github.io/meta/v1/ |
| **Código fuente local** | `/home/pibe/Descargas/Beteliney Launcher [Minecraft]/BetelineyLauncher/source/` |

---

## ESTADO ACTUAL — LEER ESTO PRIMERO (actualizado 2026-07-14, sesión 39)
> El detalle completo de cada sesión (auditorías, hallazgos, código, decisiones) está en `## HISTORIAL DE SESIONES` más abajo. Esta sección de arriba es lo único que hace falta leer para continuar el trabajo.

**Sesión 38 (retroactiva, documentada recién en sesión 39 — ver nota abajo) cerró 2 pendientes:** sandboxing opcional con Bubblewrap para el proceso de Minecraft en Linux (`da70d0e6b`, pendiente 8/ítem 11 del backlog) y un fix de path traversal en `uid`/`version` del feed de meta remoto (`254f05760`, pendiente 1 parcialmente — ver hallazgo de sesión 39).

**Sesión 39 encontró y cerró 2 vectores de path traversal adicionales que el fix de sesión 38 NO cubría**, porque ese fix solo validó el feed remoto (`meta/JsonFormat.cpp`), no las rutas de entrada de datos LOCALES no confiables:
1. `Component::m_uid` en `mmc-pack.json` de la instancia (leído en `PackProfile.cpp::componentFromJsonV1`) — un uid tipo `"../../../.ssh"` en un modpack importado/compartido se propagaba sin validar a `Component::getFilename()`, permitiendo lectura/escritura/**borrado arbitrario de archivos** vía `customize()`/`revert()`, y también a `metadataIndex()->get(uid)`.
2. `Meta::Require::uid` (campo `"uid"` dentro de arrays `"requires"`/`"conflicts"`) — parseado por `Meta::parseRequires()`, función compartida por el feed remoto Y por `mmc-pack.json` local (`cachedRequires`/`cachedConflicts`) Y por `patches/*.json` (`OneSixVersionFormat.cpp`). Este era el vector más grave: `ComponentUpdateTask::resolveDependencies()` inyecta automáticamente un `Component` nuevo por cada `Require` no resuelto — **sin ninguna interacción del usuario**, en cada resolve/launch de instancia.

**Fix aplicado:** `Meta::isSafePathComponent()` (antes `static`/interna a `JsonFormat.cpp`) se expuso públicamente en `JsonFormat.h`. Se usa ahora en `componentFromJsonV1` (lanza `JSONValidationError` si el uid es inseguro) y en `Meta::parseRequires` (lanza `Meta::ParseException` vía `requireSafePathComponent`, ya existente). El `catch` de `loadPackProfile` en `PackProfile.cpp` se amplió de `catch (const JSONValidationError&)` a `catch (const Exception&)` (clase base común de ambos tipos de excepción) para no dejar `Meta::ParseException` sin capturar — sin este cambio, un mmc-pack.json malicioso habría tumbado la app en vez de fallar de forma controlada. Verificado que `OneSixVersionFormat.cpp` (tercer consumidor de `parseRequires`, vía `patches/*.json`) ya capturaba `const Exception&` en `ProfileUtils::guardedParseJson`, así que quedó protegido gratis, sin cambios ahí.

**Test añadido:** 7 casos nuevos en `tests/MetaPathTraversal_test.cpp` (`test_requiresRejectsTraversalUid_data/test`, 5 sub-casos maliciosos + `test_requiresAcceptsLegitimateUid`). Build completo (180/180) y `ctest` (**30/30**, incluyendo los 16/16 de `MetaPathTraversal`) verificados sin errores tras el fix.

**Commit:** pendiente de crear al cerrar esta sesión (ver `git diff --stat`: 4 archivos, ~101 inserciones — `JsonFormat.h`, `JsonFormat.cpp`, `PackProfile.cpp`, `tests/MetaPathTraversal_test.cpp`).

**Nota operativa importante — por qué esto se documenta recién ahora:** los commits `254f05760` y `da70d0e6b` (sesión 38) se hicieron DESPUÉS del cierre de sesión 37 (`8cbdfffed`) pero nunca se escribió su entrada correspondiente en este archivo — la sesión terminó (probablemente por corte de contexto) sin documentar. Esto es exactamente el escenario que este archivo existe para evitar: si una sesión nueva hubiera empezado sin revisar `git log` contra el contenido de este documento, habría re-auditado el meta server desde cero sin saber que ya había un fix parcial aplicado. **Lección reforzada: antes de dar cualquier tarea por "pendiente", comparar `git log --oneline -10` contra la última entrada de este archivo — si hay commits más nuevos que la última sesión documentada, documentarlos primero, ANTES de empezar trabajo nuevo.**

**Todo el estado técnico de sesiones 24-37 sigue vigente sin cambios de fondo**, salvo lo de arriba.

**Pendientes reales restantes (5 puntos):**
1. Meta server (`ElPibeCapo/meta`) como fuente de verdad — el path traversal de `uid`/`version` (feed remoto, local, y `Require`) ya está cerrado (sesiones 38+39). Sigue faltando una auditoría línea por línea del resto (validación de tamaño/estructura de respuestas, manejo de errores HTTP, TLS/certificate pinning si aplica, etc.) — esto NO se hizo todavía, solo se cerró el path traversal.
2. `known-hashes.json` (en `~/Descargas/meta_beteliney`, repo `meta`) — bloqueado por API key de abuse.ch/MalwareBazaar, requiere que el usuario la consiga.
3. Purga del historial de git de las 4 API keys viejas de CurseForge — sigue esperando confirmación explícita del usuario, irreversible.
4. Pruebas manuales GUI (backup de mundos, badge de mods) — sin cambios, no automatizables desde este entorno.
5. Paso de firma real en CI nunca probado end-to-end — secret presente, falta que se dispare un release real.

**De los 5 restantes, solo el 2 y el 3 dependen 100% del usuario. El resto (1, 4, 5) se puede seguir trabajando sin intervención.**

---

## STACK TÉCNICO

| Capa | Tecnología |
|---|---|
| Lenguaje | C++20 |
| UI framework | Qt 6 — Widgets + QSS (no QML, no Tauri) |
| Build | CMake 3.25+ + Ninja |
| CI Linux | Ubuntu 24.04, `apt` Qt6, `ninja -j$(nproc)` |
| CI Windows | MSYS2 MinGW64, Qt6 via MSYS2 repos (sin aqtinstall) |
| Optimización | `-O3 -march=znver1 -mtune=znver1` Release |
| Auth Minecraft | MSA Device Code Flow → Xbox Live → XSTS → token Minecraft |
| Hashing | MD4/MD5/SHA1/SHA256/SHA512/Murmur2 async (Murmur2=CurseForge, SHA512=Modrinth) |
| SQLite | Qt QSQLITE driver — usado en GDLauncherMigrator |
| Crash Linux | `sigaction` + `backtrace_symbols_fd()` |
| Crash Windows | `SetUnhandledExceptionFilter` + `MiniDumpWriteDump` (dbghelp) |

---

## HISTORIAL DE COMMITS

> **Corregido en sesión 39:** este bloque estaba congelado desde ~sesión 17 (último commit real listado abajo es `a7795abe7`), mientras la tabla de IDENTIDAD DEL PROYECTO prometía que acá estaba "el último hash real en main" — falso desde hace ~20 sesiones. No se reescribe como espejo completo de `git log` (mantener eso manual a mano es justo lo que causó este desfase); en vez de eso, **la fuente de verdad es `git log --oneline` directamente**, y este bloque queda como snapshot histórico de los primeros commits del proyecto (útil para ver de dónde viene v8.2.0→v8.3.0) más un puntero corto a los commits recientes reales.

**Commits recientes reales (sesiones 27-39, más nuevos primero — ver `docs/CHANGELOG.md` para el detalle de cada uno):**
```
89a7f8c38  docs: ESTADO.md — Sesión 38 (retroactiva) + Sesión 39, path traversal local cerrado
de3717394  fix(security): path traversal en uid/version local (mmc-pack.json y Require compartido)
254f05760  fix(security): path traversal en uid/version del feed de meta remoto
da70d0e6b  feat: sandboxing opcional del proceso de Minecraft con Bubblewrap (Linux)
af88e5b88  fix(build): job pool para links con LTO, resuelve cuelgue histórico
17880fbb0  feat(achievements): sistema de logros de marca por tiempo jugado (Fase 3)
fe3c4a1af  Sesión 35: ESTRATEGIA_IA v5.0 + auditoría docs externos, tools/dev
c149bb9a0  fix(migration): path traversal en importador GDLauncher
77e0f40cc  feat(updater): firma Ed25519 fail-closed para releases + libsodium
e46e1f13d  feat(servers): command palette (Ctrl+K) + servidores favoritos con quick-join
efe33a69e  fix(mods): usar QPointer en BackgroundModUpdateCheckTask para evitar use-after-free
2ef426dcd  feat(mods): chequeo silencioso de actualizaciones en background al seleccionar instancia
b37308428  feat(worlds): botón de backup manual de mundos
ef861cdeb  chore: bump version 8.3.0 → 8.4.0  ← tag v8.4.0
```

**Snapshot histórico (commits originales del proyecto, v8.2.0 → v8.3.0, sin actualizar desde entonces):**
```
a7795abe7  fix: backport 3 fixes reales de Prism 11.0.0 -> 11.0.2 upstream
2d475330e  docs: capturas de BetelineyPacks y perfiles JVM en README, Discord corregido, Roadmap sincronizado
8550d2550  docs: confirmar rotacion CurseForge key + CI verde en corrida 28712624812
359498a21  ci: verificar secret CURSEFORGE_API_KEY recien rotado y cargado
377718865  docs: ESTADO.md Sesión 17 — CI confirmado 100% verde, 5 bugs mas documentados
d33322c  fix(ci): agrega mingw-w64-x86_64-7zip — faltaba, 'Empaquetar' fallaba con 7z: command not found
10fc804  fix(ci): regex de version roto — Launcher_VERSION_NAME contiene variables CMake, no digitos literales
e63f8d7  fix(windows): elimina POST_BUILD duplicado en javacheck, race condition con CopyJars central
6cdbad8  fix(windows): 2 bugs mas del build de Windows (path jars + PCH)
6aaa17b  fix(windows): resuelve los 2 fallos reales del build de Windows (namespace + JDK)
8cf6afc  docs: captura real de la ventana principal verificada (Día 2 cerrado)
19f8f22  docs: Día 2 del plan — capturas de pantalla + sección Roadmap en README
047a2bb  docs: ESTADO.md — Sesión 10, crash crítico en ejecución real documentado
8705aab  fix: crash crítico free(): invalid pointer en NetJob::makeByteArray (6 sitios)
e1d1f48  docs: ESTADO.md — Sesión 9, Día 1 del plan cerrado
6346499  docs: añadir enlace Discord al README (Hito 1.4)
78adefe  docs: ESTADO.md v8 — Sesión 8 completa, tabla de bugs, commits actualizados
42bc5ed  docs: ESTADO.md Sesión 8 — build 100% limpia, commit cerrado
8a79e90  fix: 10 errores de compilación reales (build limpia Fases 3-5)
e389cd2  docs+build+refactor: v8.3.0 — README, CHANGELOG, Flatpak→packaging/, limpieza final
adbd887  docs: ESTADO.md v6 — revisión completa, todo verificado
012d4b1  feat+docs: Fase 5 — Flatpak + AppImage + SmartScreen + ESTADO v5
c9d13d8  feat: Fase 4.4 — GDLauncher Carbon importer
11bfe87  feat: Fase 4.3+4.5 — CrashReporter + Optimizar botón VersionPage
b5f0c3b  feat: Fase 4.1+4.2 — CheckModConflicts + MalwareScanner
c01b787  feat: Fase 3 completa — BetelineyPacks + presets + RSS
4174c4d  feat: BetelineyLogAnalyzer — motor de diagnóstico de logs v1.0
ae1ddd6  fix: Q_INIT_RESOURCE dup, BUILD_TESTING OFF, CurseForge env, BUILD_ARTIFACT CI
2915f18  BetelineyLauncher v8.2.0 — commit inicial
```

---

## CÓMO HACER UNA RELEASE

⚠️ Los tags `v8.3.0` y `v8.4.0` **ya existen** — no se pueden reusar. Antes de tagear, correr `git tag --list` para confirmar cuál es el último y bumpear a uno mayor en `CMakeLists.txt` líneas 179-181.

```bash
cd "/home/pibe/Descargas/Beteliney Launcher [Minecraft]/BetelineyLauncher/source"
git tag --list   # confirmar el último tag antes de elegir el numero nuevo
# 1. Editar CMakeLists.txt líneas 179-181 con la versión nueva (ej. 8.5.0)
git add -A
git commit -m "descripción del cambio"
git tag v8.5.0   # usar la versión nueva bumpeada, NUNCA repetir un tag existente
git push && git push --tags
# El CI compila Linux + Windows y publica la Release en ~15 min automáticamente
```

Artefactos que genera el CI:
- `BetelineyLauncher-{VER}-Linux-x86_64.tar.gz` — Linux
- `BetelineyLauncher-{VER}-Windows-x64.zip` — Windows

Para AppImage manual (después de compilar localmente):
```bash
bash EMPAQUETAR_APPIMAGE.sh
# Genera dist/BetelineyLauncher-{VER}-Linux-x86_64.AppImage
```

---

## API KEYS Y SERVICIOS

| Servicio | Estado | Detalle |
|---|---|---|
| **CurseForge** | ✅ | Key vieja estuvo expuesta públicamente (commits viejos, repo público, ver Sesión 15) — **ya rotada por el usuario y confirmada funcionando en CI** (Sesión 17-18, corrida `28712624812` en verde con el secret `CURSEFORGE_API_KEY` nuevo cargado). Pendiente aparte, no bloqueante: purgar la key vieja del historial de git (`git filter-repo`), decisión del usuario, ver Sesión 15. |
| **Microsoft Azure** | ✅ | App ID: `4b945c78-d30b-489e-915f-b361bf9c933b` |
| **Imgur** | ⚠️ | Key vacía. El código de upload existe. Registrar en `api.imgur.com/oauth2/addclient` si se activa. |
| **META server** | ✅ | Rama `gh-pages` del repo meta tiene todos los JSONs generados. CI corre cada 6h. |

---

## ACCIONES MANUALES — ESTADO (histórico, mantenido por trazabilidad; ver ESTADO ACTUAL al principio del documento para lo realmente pendiente hoy)

**#1 — Secret CurseForge en CI** — ✅ **Resuelto** (Sesión 17-18). Key rotada por el usuario, cargada como `CURSEFORGE_API_KEY` en GitHub Actions, CI confirmado en verde con la key nueva.

**#2 — GitHub Pages del META server** — ✅ **Resuelto** (Sesión 15). La causa real no era falta de activación sino `build_type` mal configurado (`workflow` en vez de `legacy`) — corregido vía `gh api`, build forzado, las 4 URLs de contenido confirmadas en HTTP 200.

**#3 — Feed de noticias** — ✅ **Creado** (Sesión 14): `gh-pages/v1/news/feed.atom`, con la release v8.3.0 anunciada. Persistencia asegurada en Sesión 15 (ver bug de `keep_files:false` más abajo en Sesión 15).

**#4 — Packs de BetelineyPacks** — ✅ **Creados y corregidos** (Sesión 14, con fix de URLs rotas en Sesión 26): 3 presets publicados en `gh-pages/v1/beteliney-packs/`, verificados mod por mod contra la API real de Modrinth.

**#5 — Lista negra de malware** — ⚠️ **Parcial, por diseño honesto** (Sesión 14): `gh-pages/v1/malware/known-hashes.json` existe pero con arrays vacíos — no se encontró en su momento una fuente pública real de hashes de Fractureiser. **Esto cambió** (Sesión 25, addendum): existe MalwareBazaar (`abuse.ch`, API pública gratuita) con corpus real etiquetado para Minecraft. Sembrar la lista con datos reales queda como parte de la Fase 1 del plan de sesión 25 — ver ESTADO CONSOLIDADO al final del documento.

---

## TODOS LOS ARCHIVOS BETELINEY-ESPECÍFICOS

### Código nuevo creado desde cero (no heredado de Prism)

| Archivo | Líneas | Qué hace |
|---|---|---|
| `launcher/BetelineyCode.h` | 23 | Easter egg: escribir B-E-T-E en MainWindow dispara señal `triggered()` |
| `launcher/BetelineyProfiles.h` | ~120 | 7 perfiles JVM con flags Aikar's calibrados, struct `BetelineyJVMProfile` |
| `launcher/BetelineyTime.h/cpp` | ~60 | Utilidades de fecha/hora con formato Beteliney |
| `launcher/BetelineyZip.h/cpp` | ~80 | Wrapper ZIP con soporte de progreso y cancelación |
| `launcher/crash/BetelineyPanicHandler.h/cpp` | 28+219 | Crash reporter: Linux sigaction + Windows MiniDump, muestra el backtrace al siguiente inicio con botón "Reportar en GitHub" |
| `launcher/logs/BetelineyLogAnalyzer.h/cpp` | 92+639 | Motor de diagnóstico: 18 checks, panel visual integrado en LogPage |
| `launcher/minecraft/mod/MalwareScanner.h/cpp` | 64+86 | Singleton que descarga lista negra de hashes de malware, hook en ResourceDownloadTask |
| `launcher/launch/steps/CheckModConflicts.h/cpp` | 31+91 | LaunchStep pre-lanzamiento: detecta mod IDs duplicados con ModUtils, loguea warnings |
| `launcher/migration/GDLauncherMigrator.h/cpp` | 54+309 | Importador GDLauncher Carbon: abre data.sqlite, convierte instancias a formato Prism |
| `launcher/ui/dialogs/GDLauncherMigrateDialog.h/cpp` | 45+170 | UI del importador: lista con selección múltiple, QProgressDialog, acceso en File → menú |
| `launcher/modplatform/beteliney/BetelineyPack.h` | 49 | Structs: Pack, PackMod, PackIndex, enum PackProvider |
| `launcher/modplatform/beteliney/BetelineyPackListModel.h/cpp` | 52+177 | Descarga index.json + packs individuales, ordena featured primero, emite señales async |
| `launcher/modplatform/beteliney/BetelineyPackInstallTask.h/cpp` | 42+164 | InstanceCreationTask: crea instancia con loader correcto, descarga mods, verifica SHA-512 |
| `launcher/modplatform/beteliney/BetelineyPresets.h` | 144 | 3 presets built-in sin red: Vanilla Optimizado, PvP Competitivo, Modpack Pesado NeoForge |
| `launcher/ui/pages/modplatform/beteliney/BetelineyPackPage.h/cpp/.ui` | 59+217+98 | UI completa de BetelineyPacks: lista izquierda, panel derecho, búsqueda, iconos async |
| `launcher/icons/BetelineyIcon.h/cpp` | ~60 | Gestión del ícono personalizado del launcher |
| `launcher/ui/themes/BetelineyTheme.h/cpp` | ~20+810 | Tema visual completo: deep-space `#080912`, neón `#39FF14`, cyan `#00D4FF`, JetBrains Mono |
| `launcher/ui/widgets/JavaSettingsWidget.h/cpp/.ui` | ~50+541+~150 | Widget completo de settings Java: perfiles JVM, auto-detección iGPU, badge GraalVM, warnings RAM |
| `launcher/updater/BetelineyExternalUpdater.h/cpp` | ~40+~100 | Integración del updater en la UI: timer auto-check, canal beta |
| `launcher/updater/betelineyupdater/BetelineyUpdater.h/cpp` | ~80+~200 | Motor del updater: GitHub Releases API, semver, pre-releases, backup, AppImage |
| `packaging/com.beteliney.BetelineyLauncher.json` | 88 | Manifest Flatpak: runtime KDE 6.6, permisos Wayland+X11+audio+filesystem, módulos |
| `EMPAQUETAR_APPIMAGE.sh` | 103 | Script AppImage: auto-descarga herramientas, prepara AppDir, genera AppImage |

### Archivos heredados de Prism con modificaciones significativas

| Archivo | Modificación |
|---|---|
| `launcher/ui/themes/BetelineyTheme.cpp` | 810 líneas de QSS custom (reemplaza completamente el tema Prism) |
| `launcher/ui/pages/instance/LogPage.h/cpp/.ui` | Panel `diagnosisPanel` integrado, `onLaunchTaskFinished()`, `showDiagnosis()` |
| `launcher/ui/pages/instance/VersionPage.h/cpp` | Botón "Optimizar (rendimiento)" para Fabric/Quilt con presets built-in |
| `launcher/minecraft/MinecraftInstance.cpp` | Hook `CheckModConflicts` después de `ScanModFolders` |
| `launcher/Application.cpp` | Hook `MalwareScanner::loadIfNeeded()` + `checkAndShowCrashReport()` en showMainWindow |
| `launcher/main.cpp` | Hook `installPanicHandler()` antes de `Application` |
| `launcher/ui/dialogs/NewInstanceDialog.cpp` | `BetelineyPackPage` como primera pestaña |
| `launcher/ui/MainWindow.cpp` | Acción "Importar desde GDLauncher Carbon..." en File menú |
| `launcher/ResourceDownloadTask.cpp` | Hook `MalwareScanner::isMaliciousSha256/512()` después de cada descarga |
| `program_info/win_install.nsi.in` | `MUI_WELCOMEPAGE_TEXT` con instrucciones bypass SmartScreen |
| `.github/workflows/build.yml` | CurseForge key desde secret, `BUILD_ARTIFACT`, body Release con aviso SmartScreen |
| `CMakeLists.txt` | URLs propias, `BETELINEY_PACKS_URL`, CurseForge key desde env, `BUILD_TESTING=OFF` |
| `buildconfig/BuildConfig.h/cpp.in` | Campo `BETELINEY_PACKS_URL` |

---

## FASES DE DESARROLLO — ESTADO COMPLETO

### ✅ FASE 0 — Estabilización (commits ae1ddd6, b844c53)
- `Q_INIT_RESOURCE(beteliney_icons)` duplicado eliminado de `main.cpp`
- `BUILD_TESTING=OFF` por defecto en `CMakeLists.txt`
- CurseForge API key eliminada del código → lee `$ENV{CURSEFORGE_API_KEY}`
- CI inyecta key desde `secrets.CURSEFORGE_API_KEY`
- `Launcher_BUILD_ARTIFACT` configurado en CI → auto-updater activo en builds de CI
- META server verificado: rama `gh-pages` tiene net.minecraft, Forge, NeoForge, Fabric, Quilt, Java (Adoptium/Azul/IBM)

### ✅ FASE 1 — Motor de diagnóstico de logs (commit 4174c4d)

**Archivos:** `launcher/logs/BetelineyLogAnalyzer.h` (92 líneas) + `.cpp` (639 líneas)

**18 checks implementados:**

| Check | Detecta | Sev. | ActionTarget |
|---|---|---|---|
| `checkOutOfMemory` | `java.lang.OutOfMemoryError` — heap / GC overhead / Metaspace | Critical | `"java"` |
| `checkHeapReservation` | `Could not reserve enough space` / `Unable to create native thread` | Critical | `"java"` |
| `checkDuplicateMod` | `Duplicate mod id` (Fabric) / `Found duplicate mod` (Forge) | Critical | `"mods-folder"` |
| `checkMissingDependency` | `requires mod X to be loaded` / `Missing Mods:` — extrae nombre | Critical | `"search-modrinth:X"` |
| `checkIncompatibleMods` | `Incompatible mods found` / `conflicts with mod` | Error | — |
| `checkMixinConflict` | `Mixin transformation failed` / `MixinApplyError` | Error | — |
| `checkFabricIncompatible` | Versiones incompatibles de Fabric loader | Error | — |
| `checkJavaNotFound` | `Failed to start the minecraft runtime` | Critical | `"java"` |
| `checkUnsupportedJavaVersion` | `UnsupportedClassVersionError` | Critical | `"java"` |
| `checkForgeJavaRequirement` | `Forge requires Java` | Critical | `"java"` |
| `checkOpenGLNotAccelerated` | `Pixel format not accelerated` — GPU sin aceleración | Error | — |
| `checkOpenGLError` | Errores genéricos de OpenGL | Warning | — |
| `checkNativesCrash` | Exit code -1073741819 (Windows AV) / SIGSEGV | Error | — |
| `checkWindowsLoadLibrary` | `LoadLibrary failed` — DLL faltante | Error | — |
| `checkNetworkError` | Timeout / SSL / connection refused durante auth | Warning | — |
| `checkFractureiser` | Hashes y firmas conocidos del malware Fractureiser | Critical | — |
| `checkForgeEarlyWindow` | `Failed to create early progress window` | Error | — |
| `checkForgeCoremods` | Error de Coremod durante init de Forge | Error | — |

**Panel de diagnóstico en LogPage:**
- Se activa cuando `Task::finished` y `gameExitCode != 0`
- Severidad visual: Critical=rojo `#FF4444`, Error=naranja `#FF8C00`, Warning=amarillo `#FFD700`, Info=cyan `#00D4FF`
- Múltiples diagnósticos navegables con contador "1/3", botón "Siguiente ▶"
- ActionTargets: `"java"` → abre Settings Java, `"mods-folder"` → abre carpeta mods en explorador, `"search-modrinth:X"` → abre Modrinth en navegador
- Botón ✕ para descartar, se reinicia al lanzar nueva sesión

### ✅ FASE 2 — Selector de perfiles JVM en UI (commit inicial 2915f18)

**Archivo:** `launcher/ui/widgets/JavaSettingsWidget.h/cpp/.ui` (541 líneas)

**7 perfiles en `BetelineyProfiles.h`:**

| Idx | Nombre | Xmx sugerido | Notas clave |
|---|---|---|---|
| 0 | Personalizado (sin flags) | manual | Limpia todo para config manual |
| 1 | iGPU / RAM compartida | 384–1536 MB | G1HeapRegionSize=1M, sin UseTransparentHugePages, MaxGCPauseMillis=100 |
| 2 | Ligero Vanilla | 512–2048 MB | G1HeapRegionSize=1M, sin UseStringDeduplication |
| 3 | Balanceado | 2048–4096 MB | G1HeapRegionSize=2M, UseStringDeduplication=ON |
| 4 | Pesado 100–300 mods | 2048–6144 MB | G1HeapRegionSize=8M, AlwaysPreTouch=ON |
| 5 | Extremo ≥300 mods | 6144–12288 MB | G1HeapRegionSize=16M, MaxGCPauseMillis=200 |
| 6 | iGPU ZGC Java21+ | 384–1536 MB | UseZGC + ZGenerational, pausas <1ms, SoftMaxHeapSize=1280m |

**JavaSettingsWidget funcionalidades:**
- Auto-detección iGPU: Linux via `lspci -mm` (keywords: Vega, Picasso, Renoir, UHD Graphics, Iris...), Windows via `wmic Win32_VideoController`
- Auto-sugerencia de perfil según RAM total + iGPU detectada
- Badge GraalVM: ejecuta `java -version` en background al cambiar ruta Java, muestra borde neón si es GraalVM
- Warnings: rojo si Xmx ≥ RAM total, amarillo si > 90%, naranja si iGPU y > 50%
- Botón "Aplicar": confirma si hay -Xmx/-Xms conflictivos en args, auto-rellena spinboxes + jvmArgsTextBox

**INI keys:**
`MinMemAlloc`, `MaxMemAlloc`, `JvmArgs`, `OverrideMemory`, `OverrideJavaArgs`, `OverrideJavaLocation`, `JavaPath`, `IgnoreJavaCompatibility`, `AutomaticJavaSwitch`, `AutomaticJavaDownload`

**Arquitectura:**
```
InstanceSettingsPage → MinecraftSettingsWidget → JavaSettingsWidget
JavaPage (global) → JavaSettingsWidget
```

### ✅ FASE 3 — Ecosistema Beteliney (commit c01b787)

**BetelineyPacks** — plataforma propia de modpacks servida desde GitHub Pages:
- `modplatform/beteliney/BetelineyPack.h` — structs Pack, PackMod, PackIndex
- `modplatform/beteliney/BetelineyPackListModel.h/cpp` — descarga `index.json` + cada pack, async, featured primero
- `modplatform/beteliney/BetelineyPackInstallTask.h/cpp` — crea instancia (loader+versión), descarga mods, verifica SHA-512
- `modplatform/beteliney/BetelineyPresets.h` — 3 presets built-in disponibles sin internet:
  - `builtin-vanilla-optimized`: Fabric 1.21.1 + Sodium + Lithium + Iris + ModernFix
  - `builtin-pvp-competitive`: Fabric 1.21.1 + Sodium + Lithium + FerriteCore
  - `builtin-heavy-modpack`: NeoForge 1.21.1 base (sin mods)
- `ui/pages/modplatform/beteliney/BetelineyPackPage.h/cpp/.ui` — primera pestaña en NewInstanceDialog

**Formato del índice** (`gh-pages/v1/beteliney-packs/index.json`):
```json
{ "formatVersion": 1, "ids": ["beteliney-survival-1"] }
```

**Formato de cada pack** (`gh-pages/v1/beteliney-packs/{id}.json`):
```json
{
  "formatVersion": 1, "id": "beteliney-survival-1", "name": "Beteliney Survival",
  "description": "...", "version": "1.0.0", "minecraft": "1.21.1",
  "loader": "fabric", "loaderVersion": "0.16.9", "featured": true,
  "icon": "https://elpibecapo.github.io/meta/v1/beteliney-packs/icons/survival.png",
  "screenshots": [], "tags": ["survival"],
  "mods": [
    { "provider": "modrinth", "projectId": "AANobbMI",
      "url": "https://cdn.modrinth.com/...",
      "sha512": "abc123...", "filename": "sodium-fabric-0.6.4.jar" }
  ]
}
```

**RSS propio:** `NEWS_RSS_URL` → `https://ElPibeCapo.github.io/meta/v1/news/feed.atom`
`BETELINEY_PACKS_URL` → `https://ElPibeCapo.github.io/meta/v1/beteliney-packs/`

### ✅ FASE 4 — Features avanzados (commits b5f0c3b, 11bfe87, c9d13d8)

**4.1 CheckModConflicts** (`launch/steps/CheckModConflicts.h/cpp` — 31+91 líneas):
- `LaunchStep` que corre después de `ScanModFolders`, antes de lanzar Minecraft
- Lee todos los `.jar` con `ModUtils::process(mod, BasicInfoOnly)`, extrae `mod_id`
- Detecta IDs duplicados → los registra como warnings en el log de lanzamiento con cuadro ASCII
- No bloquea el lanzamiento — advierte y continúa

**4.2 MalwareScanner** (`minecraft/mod/MalwareScanner.h/cpp` — 64+86 líneas):
- Singleton global, descarga `gh-pages/v1/malware/known-hashes.json` al iniciar el launcher
- Cachea en `QSet<QString>` SHA-256 y SHA-512 → búsqueda O(1)
- Hook en `ResourceDownloadTask::downloadSucceeded()`: hash → lista negra → borra archivo + emite error
- Fallo silencioso si no hay red (no bloquea funcionalidad normal)
- `MalwareScanner::instance()->loadIfNeeded()` en `Application::showMainWindow()`

**4.3 BetelineyPanicHandler** (`crash/BetelineyPanicHandler.h/cpp` — 28+219 líneas):
- Linux: `sigaction` para SIGSEGV/SIGABRT/SIGFPE/SIGILL → escribe `/tmp/beteliney_crash_<pid>.txt` con `backtrace_symbols_fd()`
- Windows: `SetUnhandledExceptionFilter` + `MiniDumpWriteDump` (dbghelp ya en CMakeLists)
- Al siguiente inicio: `checkAndShowCrashReport()` busca crashfiles → QDialog con backtrace + botón "Reportar en GitHub" (URL con template pre-llenado) → borra el archivo
- `installPanicHandler()` en `main()` antes de `Application()`
- `checkAndShowCrashReport()` en `Application::showMainWindow()`

**4.4 GDLauncherMigrator** (`migration/GDLauncherMigrator.h/cpp` — 54+309 líneas):
- Detecta `~/.local/share/gdlauncher_next/` (Linux) o `%APPDATA%/gdlauncher_next/` (Windows), prueba 4 rutas candidatas
- Abre `data.sqlite` con Qt QSQLITE, usa `PRAGMA table_info` para detectar schema automáticamente
- Lee: id, name, mc_version, modloader, modloader_version, shortpath
- Exporta: crea `instance.cfg` + `mmc-pack.json` en formato Prism, copia `.minecraft/` buscando 3 subrutas posibles, genera `AVISO_MIGRACIÓN.txt` si los archivos no se encontraron
- `GDLauncherMigrateDialog.h/cpp` (45+170 líneas): lista selección múltiple, QProgressDialog con cancelación, acceso en **File → "Importar desde GDLauncher Carbon..."**

**4.5 Botón "Optimizar" en VersionPage** (97 líneas añadidas):
- Visible solo cuando loader es Fabric, Quilt o LegacyFabric (método `updateVersionControls`)
- Al pulsar: lee preset `builtin-vanilla-optimized` de `BetelineyPresets.h`, detecta mods faltantes en `modsRoot()`, pide confirmación, descarga via NetJob + QProgressDialog
- No reinstala mods ya presentes (check por nombre de archivo)
- Ícono: `QIcon::fromTheme("run-build-configure")`

### ✅ FASE 5 — Distribución profesional (commit 012d4b1)

**5.1 Flatpak** (`packaging/com.beteliney.BetelineyLauncher.json` — 88 líneas):
- App ID: `com.beteliney.BetelineyLauncher`
- Runtime: `org.kde.Platform//6.6`
- Finish-args: `--share=network --share=ipc --socket=wayland --socket=x11 --socket=pulseaudio --device=all --filesystem=home`
- Módulos: libqrencode + cmark + BetelineyLauncher
- Post-install: instala binario, iconos SVG/PNG, metainfo, desktop entry, JARs

**5.2 SmartScreen Windows:**
- `win_install.nsi.in`: `MUI_WELCOMEPAGE_TEXT` con instrucciones exactas (Más información → Ejecutar de todas formas)
- `build.yml` Release body: sección `⚠️ Windows Defender SmartScreen` con pasos numerados
- Solución ideal futura: certificado EV code signing (~$200-400/año)

**5.3 AppImage** (`EMPAQUETAR_APPIMAGE.sh` — 103 líneas):
- Auto-descarga linuxdeploy + plugin-qt + appimagetool si no están en `dist/tools/`
- Prepara AppDir completo: binario + JARs + iconos + desktop entry + metainfo (con sed para vars)
- Usa linuxdeploy-plugin-qt para deployar Qt automáticamente
- Output: `dist/BetelineyLauncher-{VER}-Linux-{ARCH}.AppImage`

---

## CÓDIGO HEREDADO DE PRISM (funcional, sin cambios significativos)

- **Auth MSA completa**: MSAStep → MSADeviceCodeStep → XboxUserStep → XboxAuthorizationStep → XboxProfileStep → EntitlementsStep → MinecraftProfileStep → GetSkinStep
- **AccountType {MSA, Offline}** — ambas funcionales
- **Symlinks entre instancias** — 50 instancias comparten assets, solo sus mods ocupan espacio extra
- **Forge processors** — `install_profile.json` con processors ejecutados en cadena (1.20+)
- **Modrinth** — API + CheckUpdate + InstanceCreation + PackExport
- **CurseForge/Flame** — API + CheckUpdate + InstanceCreation + FileResolving
- **ATLauncher, FTB, LegacyFTB, Technic, PackWiz** — todos funcionales
- **Java auto-descarga** — ON en Windows, OFF en Linux por defecto (compatibilidad distros)
- **McClient/McResolver** — ping TCP de servidores Minecraft (MOTD, versión, jugadores, SRV)
- **HashUtils** — SHA1/SHA256/SHA512/MD4/MD5/Murmur2 async
- **Setup wizard** — AutoJava, Java, Language, Login, Theme, Paste
- **NSIS installer** — 60+ idiomas, URL handlers, asociaciones de archivo, desinstalador
- **BetelineyUpdater** — GitHub Releases API, semver, pre-releases, AppImage update
- **AnonymizeLog** — elimina tokens, UUIDs, IPs antes de compartir logs
- **Imgur upload** — código existe, key vacía

---

## DECISIONES TÉCNICAS FIJAS

| Decisión | Alternativa rechazada | Razón |
|---|---|---|
| Fork Prism, no reescritura | Rust + Tauri desde cero | Auth MSA + Forge processors + todas las plataformas de mods = meses reimplementando. 90% ya existe y funciona. |
| Qt Widgets + QSS, no QML | Migrar a Qt Quick | Migrar 100+ vistas = reescribir la UI entera. QSS produce resultados excelentes. |
| GitHub Pages para META | VPS propio | Gratis, cero mantenimiento. Migrar a Cloudflare Pages si supera 100 GB/mes. |
| MSYS2/MinGW64 Windows CI | aqtinstall | aqtinstall falló en todos los mirrors durante el setup del CI. |
| INI + SQLite donde aplica | Solo SQLite | INI es suficiente para settings. SQLite solo donde hay datos relacionales (GDLauncher). |
| `BUILD_TESTING=OFF` default | Tests en todo build | ECMAddTests.cmake falla en CMake 4.x en este entorno. |

---

## VERSIONES

| Tipo | Criterio |
|---|---|
| **Patch** x.x.+1 | Bugfixes, cambios menores |
| **Minor** x.+1.0 | Feature completo, fase completa |
| **Major** +1.0.0 | Cambio arquitectural, reescritura de subsistema |

**Actual en código:** v8.4.0 (bump y release ejecutados en Sesión 27, tag `v8.4.0` publicado en GitHub Releases)
**Para publicar:** bumpear a la versión nueva en `CMakeLists.txt` primero, después `git tag vX.Y.Z && git push --tags` — **nunca** reusar `v8.3.0` ni `v8.4.0`, ambos tags ya existen.

---

## QUÉ SIGUE (IDEAS FUTURAS, NO PLANIFICADAS — categoría distinta al plan priorizado de Sesión 25)

Estas son ideas de infraestructura/distribución de largo plazo, sin investigación de comparación con otros launchers detrás (a diferencia del plan de Sesión 24-26, que sí la tiene y está priorizado — ver **ESTADO CONSOLIDADO** al final del documento para ese plan). No se solapan salvo un ítem, marcado abajo.

- **Flathub** — submitear el manifest Flatpak para revisión oficial
- **macOS** — el código heredado existe (Sparkle updater, entitlements), sin CI activo
- **Sincronización en nube de instancias** — GDLauncher Carbon lo tiene, requiere backend propio
- **Verificación de mods en instancias existentes** — escanear mods ya instalados con MalwareScanner
- **Soporte ARM64** — cambiar `-march=znver1` por detección automática en CI
- ~~i18n propio~~ — **movido al plan priorizado** (Fase 4 de Sesión 25), no duplicar acá.

---

## HISTORIAL DE SESIONES

### Sesión 1 — commit inicial v8.2.0
Branding Beteliney sobre Prism, BetelineyTheme v5, idioma español, perfiles JVM, fork funcional.

### Sesión 2 — Fases 0 y 1
Fase 0: estabilización CI (Q_INIT_RESOURCE dup, BUILD_TESTING, CurseForge env, BUILD_ARTIFACT).
Fase 1: BetelineyLogAnalyzer — motor de diagnóstico de logs, 18 checks, panel en LogPage.

### Sesión 3 — Fases 3 y 4
Fase 3: BetelineyPacks (PackListModel + InstallTask + PackPage + Presets built-in + RSS propio).
Fase 4.1-4.2: CheckModConflicts + MalwareScanner con lista negra remota.

### Sesión 4 — Fase 4 completa + Fase 5
Fase 4.3: CrashReporter (sigaction Linux + MiniDump Windows) + botón Optimizar VersionPage.
Fase 4.4: GDLauncher Carbon importer (SQLite → formato Prism).
Fase 5: Flatpak manifest + AppImage script + SmartScreen bypass en NSIS + CI Release body.

### Sesión 5 — Auditoría completa
Verificación sistemática: 29/29 archivos, 19/19 CMakeLists, 18/18 hooks, CI 19/19, Flatpak 12/12.
Commit: `adbd887 docs: ESTADO.md v6 — revisión completa, todo verificado`

### Sesión 6 — Documentación y versión (2026-06-18)
CMakeLists.txt: 8.2.0 → 8.3.0.
README.md (source/): reescrito completo con todas las features v8.3.0.
docs/CHANGELOG.md: header v7→actual + entrada v8.3.0 con 32 entradas detalladas (Fases 0-5).
DEVLOG.md + MASTER_PLAN.md: convertidos a stubs deprecated.
ESTADO.md: versión 8.2.0 → 8.3.0.

### Sesión 7 — Reorganización estructural (2026-06-18)
**Bug corregido:** `dist/com.beteliney.BetelineyLauncher.json` estaba en carpeta gitignored → nunca se commitió en todas las sesiones anteriores. Movido a `packaging/` (rastreado por git).
**dist/ limpiado:** eliminados tarballs obsoletos v7 y v8 (builds stale locales).
**docs/ raíz eliminado:** INDICE.md, PENDIENTES.md, ESTRUCTURA.md, AUDITORIA.md, AUDITORIA\_v8.md, SESIONES.md — todos v8.2.0, todos absorbidos en ESTADO.md. Directorio eliminado.
**README.md raíz:** v8.2.0→v8.3.0, tabla docs apunta a ESTADO.md + source/docs/, árbol refleja estructura real, changelog simplificado.
**ESTADO.md:** referencias `dist/` → `packaging/` en tabla de archivos y sección Fase 5. Versión VERSIONES corregida (v8.2.0→v8.3.0).
**CHANGELOG.md:** entradas 33-34 añadidas (REFACTOR packaging + CLEANUP dist).


### Sesión 8 — Build limpia completa v8.3.0 (2026-06-19)
**Objetivo:** primera compilación real de punta a punta con `ninja -j$(nproc)` (GCC 15, `-Werror`, `-flto=auto`, `-O3 -march=znver1`) para validar el código de las Fases 3-5 antes de tagear. **10 bugs reales encontrados y corregidos.** Build terminó limpia: 403/403 objetos + link final → ejecutable `build/beteliney` (15 MB).

**Por qué no se detectaron antes:** ningún bug es error de diseño — son APIs internas del fork (`Net::Download`, `Task::setName`, `Net::ChecksumValidator`, `InstanceList`, `SettingsObject`, `QNetworkAccessManager`) que cambiaron de firma en algún punto del desarrollo. El código de Fases 3-5 fue escrito sin compilar incrementalmente, quedando con firmas viejas. Esta sesión fue la primera compilación limpia completa del proyecto.

**Bugs corregidos — 10 en total, 12 archivos:**

| # | Archivo | Error | Fix |
|---|---|---|---|
| 1 | `minecraft/mod/MalwareScanner.cpp` | `makeByteArray(url, &buf)` — firma vieja (2 args) | `auto [dl, buf] = Net::Download::makeByteArray(url)` |
| 2-3 | `modplatform/beteliney/BetelineyPackListModel.cpp` | mismo en `fetchIndex()` y `fetchPack()` | ídem (2 ocurrencias) |
| 4 | `modplatform/beteliney/BetelineyPackInstallTask.cpp` | `setName(name, version)` — `setName()` acepta 1 arg | `setName(pack.name)` |
| 5 | `modplatform/beteliney/BetelineyPackInstallTask.cpp` | `addValidator(make_shared<ChecksumValidator>(...))` — espera puntero crudo | `new Net::ChecksumValidator(...)` |
| 6 | `ui/MainWindow.cpp:269` | `APPLICATION->instances()->instDir()` — método inexistente | `APPLICATION->settings()->get("InstanceDir").toString()` |
| 7 | `modplatform/beteliney/BetelineyPresets.h` | `tr("...")` en función libre (no clase QObject) | `QObject::tr(...)` + `#include <QCoreApplication>` (8 ocurrencias) |
| 8 | `ui/pages/instance/LogPage.ui` | `<property name="contentsMargins">` con 4 `<number>` repetidos — `uic` lo convierte a `setContentsMargins(8)` que no existe en Qt6 | 4 propiedades separadas: `leftMargin`/`topMargin`/`rightMargin`/`bottomMargin` |
| 9 | `ui/pages/modplatform/beteliney/BetelineyPackPage.h` | `QListWidgetItem*` sin forward-declare — compilador lo lee como `int*` en cascada | `class QListWidgetItem;` antes del namespace Ui |
| 10a | `ui/pages/modplatform/beteliney/BetelineyPackPage.cpp` | `APPLICATION->settings().get()` — `.get()` sobrante en puntero crudo | `APPLICATION->settings()` directo |
| 10b | ídem | `APPLICATION->network().get()` — ídem, errores de lambda en cascada | `APPLICATION->network()` directo |
| 11 | `migration/GDLauncherMigrator.cpp:107` | `selectSql = QString(...)` sin declarar — variable usada antes de existir | `QString selectSql = QString(...)` |

**Commits de la sesión:**
- `8a79e90` — fix: 10 errores de compilación reales (build limpia Fases 3-5) — 12 archivos, 489 ins / 23 del
- `42bc5ed` — docs: ESTADO.md Sesión 8

**Estado:** build 100% limpia verificada localmente. **Pendiente: `git push` + `git tag v8.3.0`.**

### Sesión 9 — Push, tag v8.3.0 y Discord (2026-06-19)
`git push origin main` (78adefe) + `git tag v8.3.0 && git push origin v8.3.0` → CI dispara build Linux+Windows y publica la Release automáticamente (`.github/workflows/build.yml`, job `release`, trigger `tags: [v*.*.*]`).

Servidor Discord: https://discord.gg/fMbSkEd85r — badge añadido al README (commit `6346499`, link actualizado sesión 19).

**Plan de lanzamiento (Día 1 — cerrado):**
| Hito | Estado |
|---|---|
| 1.1 — push código v8.3.0 | ✅ |
| 1.2 — Release v8.3.0 vía CI + tag | ✅ |
| 1.3 — Discord creado | ✅ |
| 1.4 — enlace Discord en README | ✅ |

**Pendiente (Días 2-7 del plan):**
- Día 2: capturas de pantalla de las 5 features clave → `screenshots/` + enlazar en README, sección Roadmap.
- Día 3: `known-hashes.json` real (Fractureiser y similares) en repo meta; 3 packs base en BetelineyPacks.
- Día 4: revisar tests de traducción (posible falso-vacío si `BETELINEY_SRCDIR` no está definido).
- Día 6: publicar en r/feedthebeast, r/Minecraft, Discord de Prism Launcher.
- Día 7: formulario claude.com/contact-sales/claude-for-oss.

### Sesión 10 — Crash crítico en ejecución real: free(): invalid pointer (2026-06-19)
**Contexto:** la Sesión 8 verificó que el proyecto *compilara* limpio (403/403) pero nunca se ejecutó el binario resultante. Esta sesión testeó el binario real (`lanzar.sh`) por primera vez desde el fix de compilación — **el launcher crasheaba el 100% de las veces al arrancar**, en menos de 3 segundos, siempre que `MalwareScanner` falla la descarga de `known-hashes.json` (lo cual ocurre siempre ahora mismo, porque GitHub Pages del repo `meta` sigue sin activarse → 404).

**Causa raíz:** el fix de compilación de la Sesión 8 (bug #1, nueva firma de `Net::Download::makeByteArray`) introdujo `delete response;` sobre el puntero devuelto por esa función. Ese puntero **no es memoria propia** — apunta a `ByteArraySink::m_output` (`&m_output`, un `QByteArray` miembro normal de clase), cuyo ciclo de vida lo gestiona `Download::m_sink` (`unique_ptr`) dentro del propio `Download`/`NetJob` (`shared_ptr`). `delete` sobre la dirección de un miembro de objeto es UB garantizado: corrompe el heap, y glibc aborta con `free(): invalid pointer` en la siguiente `malloc`/`free`.

Diagnóstico confirmado con `addr2line` sobre el binario (no stripped) — backtrace exacto: `ConcurrentTask::executeNextSubTask()` → `NetJob::emitFailed()` → `Task::emitFailed()` → lambda dentro de `MalwareScanner::loadIfNeeded()`.

**Corregido en 6 sitios** (mismo patrón, mismo origen — la Sesión 8 ya había advertido "mismo bug en 2 ocurrencias" en `BetelineyPackListModel.cpp` sin notar que el fix aplicado introducía uno nuevo):
- `minecraft/mod/MalwareScanner.cpp` — 2 (`succeeded` + `failed`)
- `modplatform/beteliney/BetelineyPackListModel.cpp` — 4 (`fetchIndex` succeeded+failed, `fetchPack` succeeded+failed)

Fix: eliminar todos los `delete response;`. El `QByteArray` sigue siendo válido durante el callback porque el `NetJob` (`m_job`, `shared_ptr`) sigue vivo en ese punto.

**Verificación:** recompilado limpio (403/403) → lanzado dos veces reproduciendo el 404 real → antes crasheaba consistentemente en <3s, ahora corre estable 35+s sin generar crash dumps. Commit `8705aab`.

**Lección operativa:** "compila limpio" ≠ "funciona". A partir de ahora, todo fix de compilación que toque gestión de memoria/punteros se valida ejecutando el binario, no solo `ninja`.

### Sesión 11 — Día 2 del plan cerrado: capturas + Roadmap (2026-06-20)
README: añadida sección **Roadmap** (pendientes reales del plan de 7 días + ideas futuras ya listadas en este documento, ahora visibles públicamente) y captura real de la ventana principal del launcher corriendo (`screenshots/ventana-principal.png`), tomada y verificada por el usuario directamente — confirma visualmente que el launcher arranca limpio con el fix de la Sesión 10: tema neón cargado, instancias reales con mods, sin crash. Corregida también una referencia obsoleta `dist/` → `packaging/` en la estructura de archivos del README (arrastrada desde la Sesión 7).

Commits: `19f8f22` (Roadmap + placeholder) → `8cf6afc` (captura real, placeholder retirado).

**Plan de lanzamiento — estado consolidado:**
| Día | Hito | Estado |
|---|---|---|
| 1 | push + tag v8.3.0 + Release CI | ✅ |
| 1 | Discord creado + enlazado en README | ✅ |
| 2 | Captura de la ventana principal + Roadmap en README | ✅ |
| 2 | Resto de capturas (BetelineyPacks, perfiles JVM, diagnóstico de logs) | ⏳ pendiente |
| 3 | `known-hashes.json` real en repo `meta` (depende de activar GitHub Pages, pendiente #2 más arriba) | ⏳ |
| 3 | 3 BetelineyPacks publicados | ⏳ |
| 4 | Verificar test de traducción con `BUILD_TESTING=ON` — fix BUG-2 + traducción `ModFolderPage.cpp` | ✅ |
| 6 | Publicar en r/feedthebeast, r/Minecraft, Discord de Prism Launcher | ⏳ |
| 7 | Formulario claude.com/contact-sales/claude-for-oss (deadline 30/06/2026) | ⏳ |

**Estado real del launcher a cierre de esta sesión:** compila limpio (403/403), arranca sin crashear, GUI funcional confirmada visualmente. Repo público al día (`8cf6afc`), release v8.3.0 publicada vía CI. El bloqueo principal para los Días 3-4 sigue siendo activar GitHub Pages en el repo `meta` (acción manual de navegador, pendiente #2 de la sección "Acciones manuales pendientes").

### Sesión 12 — Auditoría completa del proyecto + fix real BUG-2 (2026-06-20)
**Contexto:** revisión de cada archivo del proyecto (README raíz, ESTADO.md, plan Claude for Open Source, scripts, tests, CMakeLists) a pedido del usuario. Dos hallazgos:

**1. BUG-1, BUG-3 y BUG-4 (del plan de postulación del 18/06) ya estaban arreglados en `lanzar.sh` / `beteliney-updater.sh`, sin documentar en ninguna sesión anterior:**
- BUG-1 (lanzar.sh reenviaba `--debug`/`--no-jvm`/etc. al exe Qt, generando warnings) — ya filtrado vía array `LAUNCHER_ARGS`, marcado `# BUG-1 FIX` en el código.
- BUG-3 (race condition iGPU: en el primer arranque, sin `.cfg`, `Application.cpp` aplicaba `suitableMaxMem()` ~8192 MB antes de que el perfil iGPU se aplicara) — ya arreglado: `lanzar.sh` crea el `.cfg` mínimo con perfil iGPU *antes* de lanzar el ejecutable si no existe, marcado `# BUG-3 FIX`.
- BUG-4 (`--silent` del updater no llamaba `verify_build`) — verificado en el código: sí lo llama (`apply_update; recompile; verify_build`). Ya estaba bien.

No se pudo determinar en qué sesión se hicieron estos fixes — no aparecen en el `git log` con mensaje propio ni en el historial de sesiones de este documento. Quedan documentados acá retroactivamente.

**2. BUG-2 (tests de traducción) — confirmado bug real, no "posible falso-vacío" como decía la Sesión 11.** `BETELINEY_SRCDIR` sí estaba definido en `tests/CMakeLists.txt`, pero apuntaba a `${CMAKE_SOURCE_DIR}/launcher` en vez de `${CMAKE_SOURCE_DIR}`. El propio comentario del test (mismo commit inicial `2915f18`, nunca tocado después) decía explícitamente *"BETELINEY_SRCDIR que CMake define como CMAKE_SOURCE_DIR"* — la implementación nunca coincidió con su propia documentación. `BetelineyTranslation_test.cpp::readFile()` concatena `base + "/" + relPath`, y los 11 `relPath` ya incluyen el prefijo `launcher/...` → con el bug, la ruta resultante era `.../launcher/launcher/ui/...`, que no existe → `QFile::open()` falla → `QSKIP` → **los 11 tests de traducción al español pasaban en falso, sin verificar ningún string, desde el commit inicial del proyecto.** El fallback `QFINDTESTDATA("../launcher")` tenía el mismo error de raíz duplicada.

**Fix** (2 líneas, no se tocaron los 11 `relPath` porque el comentario original define el contrato correcto):
- `tests/CMakeLists.txt`: `BETELINEY_SRCDIR="${CMAKE_SOURCE_DIR}/launcher"` → `BETELINEY_SRCDIR="${CMAKE_SOURCE_DIR}"`
- `tests/BetelineyTranslation_test.cpp`: fallback `QFINDTESTDATA("../launcher")` → `QFINDTESTDATA("..")`

**Verificación:** reconfigurado con `cmake -DBUILD_TESTING=ON`, compilado el target `BetelineyTranslation` y corrido con `ctest`. Primera ejecución tras el fix de ruta: **15 passed, 1 failed** — el test encontró `tr("Download Mods")` sin traducir en `ModFolderPage.cpp::downloadDialogFinished()` (la función hermana `updateMods()` ya usaba `"Descargar Mods"` como string literal pero sin `tr()`, confirma que la traducción correcta es `tr("Descargar Mods")`). Corregida esa línea. Segunda ejecución: **16 passed, 0 failed, 0 skipped** — 100%.

Cierra el Día 4 del plan de lanzamiento de verdad (antes solo estaba "revisado", el bug seguía vivo).

### Sesión 13 — Auditoría de traducción completa + ESTRATEGIA_IA.md (2026-06-20)

**Dos bloques de trabajo en esta sesión:**

**1. ESTRATEGIA_IA.md (documento maestro de estrategia IA v4.0)**
Creado y guardado en `source/ESTRATEGIA_IA.md`. Consolida toda la información verificada sobre programas de patrocinio, configuración de modelos de IA y plan de acción. Puntos clave:
- Anthropic Claude for Open Source: deadline real 30/06/2026, 10.000 cupos por orden de llegada. Acción: enviar HOY vía `claude.com/contact-sales/claude-for-oss`.
- OpenAI Codex for Open Source: sin deadline fijo, URL `openai.com/form/codex-for-oss/`.
- DeepSeek V4 Pro: $0.435/$0.87 por 1M tokens (precio permanente desde 22/05/2026). Con $11.72 disponibles → ~21.5M tokens (mix 3:1).
- Claude Fable 5 / Mythos 5: no disponible (suspendido por control de exportación EE. UU.).
- Datos sin verificar marcados explícitamente como `⚠️` (chances en %, tiempos de respuesta Anthropic).

**2. Auditoría de traducción — archivos Beteliney-específicos: CERRADA AL 100%**

La sesión anterior (12) tenía pendiente un batch de ediciones que falló completamente por un error en el 6º elemento. Esta sesión reaplicó todo de forma individual, más lo que se detectó en pasadas de verificación sucesivas.

Archivos modificados y resultado:

| Archivo | Strings corregidas | Notas |
|---|---|---|
| `launcher/updater/betelineyupdater/UpdaterDialogs.cpp` | 6 | Encabezados tabla (Versión/Fecha de publicación), label, botones Cancelar/Aceptar — ambos diálogos |
| `launcher/updater/betelineyupdater/SelectReleaseDialog.ui` | 2 | `windowTitle` y `eplainLabel` placeholder |
| `launcher/BetelineyTime.cpp` | 1 | `"days"` → `"d"` (consistencia con h/m/s/ms) |
| `launcher/updater/betelineyupdater/BetelineyUpdater.cpp` | ~35 | Ver detalle abajo |

Detalle de `BetelineyUpdater.cpp` (todos los strings de usuario/log en el archivo):
- Líneas 496–507: `"No release for version!"`, `"Can not find..."`, `"No version selected."` (sin `tr()`, añadido `tr()` y traducidos)
- Líneas 760–782: `"No Valid Release Assets"`, `"Github release %1 has no valid assets..."`, `"yes"/"no"`, `"No version selected."`, `"Failed to Download"`, `"Failed to download the selected asset."`
- Líneas 884–916: bloque completo del lock file (`"Update already in progress"`, infoMsg de 10 líneas, `"Update Aborted"`)
- Línea 924: `"Updating from %1 to %2"`
- Líneas 927–930: `"Updating portable install at %1"`, `"Running installer file at %1"`
- Línea 939: `"Process start result: %1"` + `"yes"/"no"`
- Línea 946: `"Backing up install"`
- Líneas 967–969: `"Starting new updater at '%1'"`, `"Failed to launch '%1' %2"`
- Líneas 520–522: `"Finishing update process"`, `"Waiting 2 seconds for resources to free"` (sin `tr()`, añadido)
- Líneas 533/986: `"Reading manifest from %1"` (2 ocurrencias en funciones gemelas, diferenciadas por contexto)
- Línea 1017: `"manifest.txt empty or missing..."` (sin `tr()`, añadido)
- Líneas 1019–1048: bloque backup completo (`"Backing up:\n  %1"`, progress dialog, logUpdate, lambda `copy` con 3 strings)
- Líneas 1061: `"File doesn't exist, ignoring: %1"` (en `backupAppDir()`, diferente a la ya traducida en `moveAndFinishUpdate()`)
- Líneas 1082–1086: `"Extracted the following to..."`, `"Failed to extract %1 to %2"` (×2), primer argumento de `showFatalErrorMessage` era string literal sin `tr()`
- Líneas 1100–1104: `"Failed to Check Version"` (×2) + `"Failed to launch child process..."`, `"Child launcher process failed."`

**Verificación final:** grep amplio sobre todos los archivos Beteliney-específicos → 0 strings en inglés visibles al usuario o en logs de usuario. Los falsos positivos del regex final (5 en BetelineyExternalUpdater, 3 en BetelineyUpdater, etc.) se revisaron manualmente y todos estaban en español.

**Plan de lanzamiento — estado actualizado:**
| Día | Hito | Estado |
|---|---|---|
| 1 | push + tag v8.3.0 + Release CI | ✅ |
| 1 | Discord creado + enlazado en README | ✅ |
| 2 | Captura ventana principal + Roadmap en README | ✅ |
| 2 | Resto de capturas (BetelineyPacks, perfiles JVM, diagnóstico) | ⏳ |
| 3 | `known-hashes.json` real en repo `meta` (depende GitHub Pages, pendiente #2) | ⏳ |
| 3 | 3 BetelineyPacks publicados | ⏳ |
| 4 | Tests de traducción 16/16 pasando | ✅ |
| 4 | Auditoría traducción archivos Beteliney 100% | ✅ |
| 5 | ESTRATEGIA_IA.md creado, planes de patrocinio claros | ✅ |
| 5 | Enviar solicitud Anthropic Claude for OSS | ⏳ pendiente (HOY o madrugada, deadline 30/06) |
| 6 | Publicar en r/feedthebeast, r/Minecraft, Discord Prism | ⏳ |
| 7 | Formulario OpenAI Codex for OSS | ⏳ |

### Sesión 14 — Meta server poblado + corrección crítica de hashes falsos (2026-06-21)

**Contexto:** continuación directa de la Sesión 13. El objetivo era cerrar los pendientes #3, #4 y #5 (feed de noticias, BetelineyPacks, lista de malware) creando los archivos en el repo `meta` (rama `gh-pages`), clonado localmente en `~/Descargas/meta_beteliney`.

**Incidente y corrección — hashes de malware falsos:**

En el primer intento, generé un `known-hashes.json` con tres cadenas hexadecimales **inventadas como placeholder**, presentadas con una fuente falsa ("fractureiser-investigation, boletines CurseForge/Modrinth jun-2023"). El usuario las detectó al revisar el documento maestro (no porque yo lo notara primero). Verificación posterior confirmó el problema: dos de las tres cadenas tenían 65 caracteres en vez de 64, ni siquiera eran SHA-256 válidos en formato.

Investigación real hecha después:
- Repo oficial `fractureiser-investigation/fractureiser` (ahora `trigram-mrp/fractureiser`, 1.1k stars): revisado completo (`README.md`, `docs/users.md`, `docs/tech.md`, `docs/timeline.md`). **No publican una lista de hashes.** La guía oficial usa indicadores de ruta de archivo (carpeta falsa `Microsoft Edge` en Windows, `~/.config/.data/lib.jar` en Linux) y remite a escáneres de terceros por firma/comportamiento (Overwolf jar-infection-scanner, douira web-detector, nekodetector) — no por hash estático. El equipo cerró el inbox de muestras explícitamente ("no pidan samples, compartir malware es peligroso").
- Únicos hashes reales encontrados: 2 MD5 de una investigación de Bitdefender Labs (`0e583c572ad823330b9e34d871fcc2df` Stage 0, `e69b50d1d58056fc770c88c514af9a82` Stage 2 temprano). Inútiles para este scanner: `MalwareScanner.cpp::parseJson()` solo lee `hashes.sha256` y `hashes.sha512` — no existe soporte MD5 en el código (sin `isMaliciousMd5`). Meterlos en los arrays sha256/sha512 los habría dejado poblados pero sin comparar nunca contra nada real.

**Decisión final:** `known-hashes.json` se publicó con `hashes.sha256` y `hashes.sha512` como **arrays vacíos**, `"status": "no-public-hash-source-found"`, un campo `comment` explicando la investigación completa, los 2 MD5 documentados aparte (`knownSamplesMd5_notUsedByScanner`, claramente marcados como no usados) y `sourcesChecked` con las 5 URLs revisadas. El MalwareScanner queda funcional (carga el JSON sin error) pero sin pretender cobertura que no existe.

**BetelineyPacks — verificación de Modrinth antes de escribir, no después:**

Tras el incidente de los hashes, antes de escribir los 3 packs verifiqué cada `projectId` de Modrinth contra la API real (`api.modrinth.com/v2/project/{slug}` y `.../version?game_versions=...&loaders=...`) en vez de confiar en memoria. Resultado:
- **Krypton tenía un `projectId` fabricado** (`Ha28R6CL`) que no existe — el real es `fQEb0iXm`. Corregido antes de commitear.
- **LazyDFU descartado del pack PvP**: confirmado sin ningún build publicado para 1.21.1/fabric (mod efectivamente discontinuado, superado por ModernFix).
- Los 8 mods restantes (Sodium, Lithium, Iris, ModernFix, FerriteCore, EntityCulling, ImmediatelyFast, Fabric API) y los 6 del pack NeoForge (JEI, Curios API, Create, AppleSkin, Waystones, Iron Jetpacks) — todos verificados con build real para 1.21.1 y su loader correspondiente antes de incluirlos.
- 2 candidatos para el pack NeoForge (`ironchest`, `ftb-ultimine`) tenían slug incorrecto en la API — descartados en vez de adivinar el slug correcto.

**Archivos creados y pusheados a `meta` (rama `gh-pages`, commit `57c7764`):**

| Archivo | Contenido |
|---|---|
| `v1/malware/known-hashes.json` | Arrays vacíos + investigación documentada (ver arriba) |
| `v1/beteliney-packs/index.json` | IDs de los 3 packs |
| `v1/beteliney-packs/vanilla-optimizado.json` | 8 mods de rendimiento, verificados |
| `v1/beteliney-packs/pvp-competitivo.json` | 4 mods (Sodium, Lithium, Krypton corregido, Fabric API) |
| `v1/beteliney-packs/pesado-neoforge.json` | 6 mods base para NeoForge, verificados |
| `v1/news/feed.atom` | Anuncio de la release v8.3.0 |

**Bloqueo verificado, no resuelto — GitHub Pages sigue inactivo:** probé las 4 URLs (`v1/beteliney-packs/index.json`, `v1/malware/known-hashes.json`, `v1/news/feed.atom`, y un archivo que ya existía antes de esta sesión `v1/net.fabricmc.fabric-loader/`) con `curl -o /dev/null -w "%{http_code}"` — **las 4 devuelven 404**, incluida la que ya existía. Confirma que GitHub Pages nunca se activó en el repo `meta` (pendiente #2 original, Settings → Pages → branch `gh-pages`, acción manual de navegador). El contenido está listo y correcto en el repo, pero no será accesible hasta que se active. MalwareScanner y BetelineyPacks van a fallar la descarga silenciosamente (`loadFailed`) hasta entonces — comportamiento esperado del código, no un bug nuevo.

**Plan de lanzamiento — estado actualizado:**
| Día | Hito | Estado |
|---|---|---|
| 3 | `known-hashes.json` en repo `meta` | ✅ creado (honesto, cobertura real = 0 hashes) — bloqueado por Pages |
| 3 | 3 BetelineyPacks publicados | ✅ creados y verificados — bloqueado por Pages |
| 3 | feed de noticias | ✅ creado — bloqueado por Pages |
| 2 | **Activar GitHub Pages en repo `meta`** | 🔴 sigue pendiente, bloquea Día 3 completo, acción manual única |

### Sesión 15 — Fuga de credenciales corregida + causa raíz real de GitHub Pages + bug crítico de borrado automático (2026-06-21)

**Contexto:** el usuario dio acceso vía `gh` CLI (ya autenticado localmente con scopes `repo`+`workflow`) para resolver pendientes que antes requerían acción manual en navegador. No se pidió ni se recibió ningún token/contraseña en el chat — se verificó que `gh auth status` ya estaba logueado en la máquina local antes de hacer nada.

**1. Fuga de credenciales — CurseForge API key expuesta públicamente**

Hallazgo (sesión 14, cerrado parcialmente en esa sesión por corte de mensajes): la key (prefijo `$2a$10$wIJ...`, valor completo REDACTADO de esta documentación) estaba en texto plano en `ESTADO.md`, commiteada y pusheada a `origin/main` del repo público `BetelineyLauncher`. Confirmado expuesta en vivo: `raw.githubusercontent.com` la servía con HTTP 200 sin autenticación.

Hecho en esta sesión (commit `ae645db`):
- Redactadas las 2 ocurrencias en `ESTADO.md` (tabla de API keys + instrucciones de secret de CI).
- Verificado que `build/` (que también tenía la key en `CMakeCache.txt` y `BuildConfig.cpp` generados) **nunca estuvo trackeado** — está en `.gitignore`, cero exposición ahí.
- Verificado que `CMakeLists.txt` actual (HEAD) ya usa `$ENV{CURSEFORGE_API_KEY}`, sin el valor hardcodeado.

**Sin resolver, no resoluble por mí — acción exclusiva del usuario:** la key sigue viva en el historial de git (commits viejos de `ESTADO.md` y de `CMakeLists.txt` de antes de migrar a env var). Quien haya clonado/forkeado el repo, o cualquier bot que escanea GitHub público (lo hacen en minutos, no días), ya pudo haberla capturado. Redactar el HEAD no neutraliza nada retroactivamente. **La única acción que de verdad invalida el riesgo: rotar la key en el panel de CurseForge (revocar la actual, generar una nueva) y cargarla como secret nuevo en GitHub Actions.** Esto requiere login/MFA del usuario en `console.curseforge.com` — ningún acceso de GitHub lo sustituye. **No se ha hecho a la fecha de este commit.**

Adicional: se verificó `gh secret list --repo ElPibeCapo/BetelineyLauncher` → **lista vacía**. El secret `CURSEFORGE_API_KEY` **no existe** en GitHub Actions pese a que la tabla de API keys (antes de esta sesión) decía "✅ CI usa secret". Esa documentación estaba desactualizada/incorrecta. CI actualmente no tiene la key — los builds que la requieran fallarán o se saltarán ese paso. Pendiente: cargar el secret **con la key nueva, después de rotar**, nunca con la vieja.

**2. GitHub Pages del repo `meta` — causa raíz real (no era "falta activar")**

La documentación de sesiones anteriores asumía que Pages simplemente no estaba activado (pendiente #2 original: "Settings → Pages → Source: branch gh-pages"). Investigación real vía `gh api repos/ElPibeCapo/meta/pages` mostró que Pages **ya estaba configurado y "activo"**, pero con `build_type: "workflow"` apuntando a `main` — es decir, GitHub esperaba un deploy vía Actions con artifact upload (`actions/deploy-pages`). El workflow real (`generate.yml`) despliega con `peaceiris/actions-gh-pages@v3`, que hace un simple `git push origin gh-pages` (mecanismo legacy de branch). Como el tipo de build configurado no coincidía con el mecanismo de deploy real, GitHub **nunca registraba esos pushes como un build de Pages** (`pages/builds` devolvía `[]`, sin deployments) — de ahí el 404 persistente incluso en contenido del launcher que el workflow llevaba meses desplegando exitosamente (runs en verde cada ~6h).

Fix aplicado vía API (sin tocar el navegador):
```
gh api -X PUT repos/ElPibeCapo/meta/pages -f build_type=legacy -f 'source[branch]=gh-pages' -f 'source[path]=/'
gh api -X POST repos/ElPibeCapo/meta/pages/builds   # forzar build manual
```
Resultado confirmado: build `status: built`, sin error, desde el commit `57c7764` (el que contenía los 3 BetelineyPacks + malware list + feed). Verificado con curl: `net.fabricmc.fabric-loader/`, `beteliney-packs/index.json`, `malware/known-hashes.json`, `news/feed.atom` → **los 4 en HTTP 200**.

**3. Bug crítico descubierto (no estaba documentado, no lo reportó nadie antes) — el contenido Beteliney se borraba solo cada 6 horas**

Al revisar `generate.yml` para diagnosticar lo anterior, se encontró que el step de deploy usa `peaceiris/actions-gh-pages@v3` con `keep_files: false`. Ese flag borra **todo** el contenido existente de la rama `gh-pages` antes de cada push y la reemplaza únicamente con lo que el script genera (`launcher/` → metadata de Mojang/Fabric/Forge/NeoForge/Quilt/Java). El workflow corre por cron cada 6 horas además de en cada push a `main`.

Consecuencia real: `v1/beteliney-packs/`, `v1/malware/`, `v1/news/` — todo lo creado en la sesión 14 — sobrevivía únicamente porque ninguna corrida automática se había ejecutado todavía desde el push. La siguiente corrida programada los habría borrado sin que nadie lo notara, hasta que algún usuario reportara que el launcher no carga packs/noticias/malware list.

Fix aplicado (commit `04bda93` en `main` del repo `meta`):
- Creado `static/v1/` en `main`, versionado, con copia de `beteliney-packs/`, `malware/`, `news/` (la fuente de verdad para este contenido deja de ser solo la rama `gh-pages`).
- Modificado el step "Preparar pages" de `generate.yml` para copiar `static/v1/.` → `pages/v1/` **después** de copiar `launcher/.`, de forma aditiva. Así el contenido Beteliney queda incluido en el artifact que se publica en cada corrida, automática o manual, indefinidamente.

**Estado de verificación de este fix, exacto, sin redondear:** el push a `main` disparó la corrida `27908943553` (trigger `push`). A la hora de escribir esto, esa corrida seguía `in_progress` (>7 min, el historial muestra corridas de 8 a 54 min). La rama `gh-pages` en ese momento seguía en el commit `57c7764` (el deploy manual forzado en el paso 2, anterior al fix del paso 3) — **el fix todavía no se había probado en una corrida real del workflow**. Las 4 URLs seguían en HTTP 200 porque el deploy manual previo ya las tenía, no porque el fix nuevo se haya confirmado funcionando. Verificar en la próxima sesión: `gh run list --repo ElPibeCapo/meta --limit 1` (debe decir `completed success` para el run `27908943553` o posterior) y reconfirmar las 4 URLs con curl.

**Tabla de pendientes — estado real actualizado:**

| # | Ítem | Estado |
|---|---|---|
| 1 | Secret `CURSEFORGE_API_KEY` en CI | 🔴 No existe en GitHub Actions (confirmado con `gh secret list`, vacío). Bloqueado por la rotación de la key (ítem de seguridad arriba) — no cargar la key vieja. |
| 2 | GitHub Pages del repo `meta` | ✅ Resuelto — causa raíz era `build_type` mal configurado, no falta de activación. Corregido vía API, build forzado, 4 URLs confirmadas en 200. |
| 3 | Contenido Beteliney persistente en cada deploy | ✅ Corregido (commit `04bda93`) — pendiente de confirmar en una corrida automática real, ver arriba. |
| — | **Rotar key de CurseForge expuesta** | 🔴 Crítico, exclusivo del usuario, sin sustituto posible. `console.curseforge.com` → revocar la actual → generar nueva → cargarla como secret nuevo en GitHub (no en archivos). |
| — | Purgar key vieja del historial de git (`git filter-repo`) | ⏸️ Decisión pendiente del usuario — destructivo (fuerza push, rompe clones/forks existentes), y secundario: no sustituye la rotación. |
| 4 | Capturas de pantalla restantes (BetelineyPacks, perfiles JVM, diagnóstico) | ⏳ Requiere la app corriendo, acción manual. |
| 5 | Solicitud Anthropic Claude for OSS | ✅ Enviada (20/06, según documento maestro previo). |
| 6 | Publicar en r/feedthebeast, r/Minecraft, Discord Prism | ⏳ Manual. |
| 7 | Formulario OpenAI Codex for OSS | ⏳ Manual. |

### Sesión 16 — Auditoría completa: CI llevaba roto desde el primer commit registrado (2026-06-21)

**Contexto:** se pidió revisar todo en busca de errores, fugas o cosas mal hechas, sin asumir que lo reportado en sesiones anteriores era correcto. Resultado: **el CI del repo principal (`Build BetelineyLauncher`) lleva fallando en el 100% de sus corridas desde el primer run registrado (13/06)** — 30 de 30 corridas en rojo, incluida toda la ventana de las sesiones 8 a 15. Nadie lo había verificado hasta ahora; las sesiones previas documentaban features y fixes sin confirmar que el proyecto compilara en CI.

**Bug #1 — condición de carrera real en CMake (commit `c18f7bf`)**

`add_custom_target(CopyJars ...)` copia tres JARs (`JavaCheck.jar`, `NewLaunch.jar`, `NewLaunchLegacy.jar`) pero su `DEPENDS` solo listaba `JavaCheck NewLaunch` — **faltaba `NewLaunchLegacy`**. Con build paralelo (`-j`, el caso real tanto en CI como en desarrollo), ninja agendaba la copia antes de que `NewLaunchLegacy.jar` terminara de compilarse. Confirmado en el log real de CI: `[47/557] Copiando JARs` corría antes que `[49/557] Compilando NewLaunchLegacy.jar`. Fallaba en Linux con `Error copying file (if different)`; en Windows con `ninja: build stopped: cannot make progress due to previous errors` (mismo origen — el log de Windows no capturó el mensaje exacto del subproceso, pero es el mismo grafo de dependencias roto).

Fix: agregar `NewLaunchLegacy` al `DEPENDS`. Verificado localmente antes de pushear — no solo en teoría: se forzó el escenario exacto (`rm -rf build/jars build/libraries/*/share`, rebuild con `-j8`) y se confirmó que ahora `NewLaunchLegacy.jar` (paso 4/5) termina antes de `CopyJars` (paso 5/5). Build completo local: 403/403 sin errores, 29/29 tests (`ctest`) pasando.

**Bug #2 — include roto de un rename viejo, sin relación con el bug #1 (commit `718b158`)**

El push del fix #1 disparó CI real — y **volvió a fallar**, por una causa completamente distinta: `BetelineyUpdater.cpp:59` incluye `"updater/prismupdater/UpdaterDialogs.h"`, un path que ya no existe. El directorio se renombró a `updater/betelineyupdater/` en algún momento de la migración del fork de PrismLauncher, pero ese include específico no se actualizó. `fatal error: ... No such file or directory` en el runner limpio de Linux.

Esto no se detectó en ningún build local anterior porque el `.o` de ese archivo ya estaba compilado y cacheado de antes del rename — ninja, al no detectar cambios en el `.cpp`, nunca lo recompilaba, así que el include roto quedaba invisible en cualquier build incremental local. Solo un build verdaderamente limpio (o el runner de CI, que siempre parte de cero) lo expone. Lección concreta: **un build local "exitoso" sin partir de cero no es evidencia confiable de que el código compile**, esto incluye el primer build local de esta misma sesión, cuyo "403/403 sin errores" fue, en retrospectiva, una verificación incompleta (solo se habían borrado `jars/` y `share/`, no `build/` entero).

Fix: corregido el path del include. Verificado con grep recursivo que no queda ninguna otra referencia a `prismupdater`/`prism_updater` en `.cpp`/`.h`/`.ui` del proyecto.

**Intento de verificación 100% limpia local — bloqueado por el entorno, no por el código:** se intentó un `rm -rf build/` completo seguido de reconfigurar y recompilar desde cero para validar ambos fixes juntos antes de pushear el segundo. La reconfiguración de CMake falló con `Could NOT find Java (missing: Java_JAVAC_EXECUTABLE Java_JAR_EXECUTABLE Java_JAVADOC_EXECUTABLE Development)` pese a tener JDK 21 instalado — un problema de detección de entorno específico del proceso en segundo plano (probablemente diferencia de `PATH`/`JAVA_HOME` entre la sesión interactiva y el proceso `nohup`), no del repositorio. No se persiguió más — la verificación autoritativa la da CI (entorno reproducible de GitHub Actions), no esta máquina.

**Estado de CI al cierre de esta sesión, sin redondear:** el push del fix #2 (`718b158`) disparó la corrida `27911380966`, que seguía `in_progress` (~1m38s) al momento de escribir esto. **No confirmado todavía que ambos fixes juntos hagan pasar CI en verde.** Verificar en la próxima sesión con `gh run list --repo ElPibeCapo/BetelineyLauncher --limit 1` — si sigue en rojo, revisar el log del paso que falle; dado que se descartaron ya 2 causas reales y distintas, es razonable pero no seguro que sea la última.

**Otros hallazgos de la auditoría, sin acción requerida:**
- El workflow `generate.yml` del repo `meta` tiene 2 fallas de 30 corridas (6.7%) en el step "Generar Forge" — el log corta sin error visible a mitad de iterar versiones de Forge, probablemente rate-limit del servidor de Forge o problema transitorio del runner. Es del workflow original de PrismLauncher (no nuestro código), se autorecupera en la siguiente corrida programada (cron cada 6h), y no bloquea nada porque tiene `continue-on-error: true`. No se investigó más a fondo — frecuencia baja, sin impacto.
- Escaneo amplio de secretos en **todo** el historial de ambos repos (`BetelineyLauncher` y `meta`), no solo grep de la key conocida: sin hallazgos adicionales. El único hit fue un string de prueba (`supersecretvalue123abc`) en un test unitario que verifica el anonimizador de logs — no es una credencial real.
- `.gitignore` cubre correctamente `build/`, sin archivos de credenciales sueltos fuera de control de versiones.
- Schema de `BetelineyPackListModel.cpp` (parser de los packs) verificado campo por campo contra los JSON publicados — coincide exactamente, sin bug.
- URL base `BETELINEY_PACKS_URL` (con mayúscula `ElPibeCapo` en el host) probada explícitamente — DNS no distingue mayúsculas, sin problema.
- No se repite en ningún otro lado del CMake el patrón de `DEPENDS` incompleto del bug #1 (revisado cada `add_custom_target` con `DEPENDS` del proyecto).

### Sesión 17 — Confirmación de CI en verde: 5 bugs más, ninguno de código C++ (2026-07-04)

**Contexto:** continuación directa de la sesión 16. Al cierre de esa sesión, CI seguía sin confirmar verde tras los 2 fixes de la race condition de `CopyJars` y el include roto de `prismupdater`. Se retomó el diagnóstico corrida por corrida, un fallo a la vez, sin asumir que arreglar uno implicara que los demás fueran del mismo tipo.

**Bug #3 — namespace faltante en llamada a función (commit `6aaa17b`)**

`BetelineyUpdater.cpp` llamaba `AttachWindowsConsole()` sin calificar, pero la función vive en el namespace `console` (`WindowsConsole.h`). Fix: `console::AttachWindowsConsole()`.

**Bug #4 — JDK equivocado en el runner de Windows (mismo commit `6aaa17b`)**

`NewLaunch.jar` y `JavaCheck.jar` fallaban con `javac: invalid flag: --release`. Causa: el job de Windows no fijaba una versión de JDK, y `find_package(Java 1.8)` tomaba el primero que cumpliera `>=1.8` en el `PATH` — el runner `windows-2022` expone por defecto un Temurin 8.0.492-9, cuyo `javac` no reconoce `--release` (el flag existe desde JDK 9). Fix: agregado `actions/setup-java` con Temurin 21 antes del `configure`, para que CMake detecte el JDK correcto.

**Bug #5 — path de jars hardcodeado a Linux (commit `6cdbad8`)**

`CopyJars` (target central en el `CMakeLists.txt` raíz) asumía el path fijo `share/${Launcher_Name}` como origen de los 3 JARs — correcto solo en Linux, donde `JARS_DEST_DIR` vale eso. En Windows `JARS_DEST_DIR="jars"`, así que los jars reales quedaban en `libraries/{javacheck,launcher}/jars/*.jar` y `CopyJars` fallaba con "No such file or directory". Fix: usar `${JARS_DEST_DIR}` en vez del literal, igual que los `CMakeLists.txt` que generan los jars.

**Bug #6 — PCH inválido por definición de macro distinta (mismo commit `6cdbad8`)**

`target_precompile_headers("${Launcher_Name}_updater" REUSE_FROM prism_updater_logic)` reusaba el precompiled header de una librería estática que no define `QT_NEEDS_QMAIN`, en un ejecutable `WIN32` que sí la define — con `-Werror`, eso es `invalid-pch` tratado como error fatal. Mismo patrón ya corregido antes para `Launcher_logic` y `filelink_logic` (comentarios "FIX" ya existentes ahí); solo faltaba aplicarlo a este target. Fix: generar su propio PCH en vez de reusar el de `prism_updater_logic`.

**Bug #7 — race condition duplicada en javacheck (commit `e63f8d7`)**

`javacheck/CMakeLists.txt` tenía su propio `POST_BUILD` que copiaba `JavaCheck.jar` al mismo destino que el `CopyJars` central de la raíz (mismo origen, mismo destino: `build/jars/JavaCheck.jar`). En Windows/Ninja esa dependencia cruzada entre directorios para un `POST_BUILD` de un target utilitario no tiene garantía de orden entre sí — el propio comentario del repo ya advertía sobre este patrón ("requiere mismo directorio en CMake 4.x"). `CopyJars` espera solo el archivo de salida del target `JavaCheck`, no el paso `POST_BUILD` duplicado, así que ambos corrían sin orden garantizado. `launcher/CMakeLists.txt` ya no tenía este hack (su propio comentario decía que fue reemplazado) — solo quedaba en `javacheck`. Fix: eliminado el duplicado, `CopyJars` queda como único mecanismo.

Con los bugs #3-#7 aplicados, **"Compilar" pasó en verde por primera vez** tanto en Linux como en Windows (confirmado en la corrida `28692985548`). El resto de bugs de esta sesión ya no son de C++/CMake — son del propio workflow de CI.

**Bug #8 — regex de versión roto desde el commit inicial del proyecto (commit `10fc804`)**

El step "Obtener version" (Linux y Windows) usaba `grep -oP 'Launcher_VERSION_NAME "\K[\d.]+' CMakeLists.txt`, pero esa línea es `set(Launcher_VERSION_NAME "${Launcher_VERSION_MAJOR}.${Launcher_VERSION_MINOR}.${Launcher_VERSION_PATCH}")` — no hay dígitos literales tras la comilla, son variables CMake sin resolver en el archivo de texto. El regex nunca matcheó nada, ni en este ni en ningún commit anterior.

Por qué solo se manifestaba en Windows: el shell del job Linux es `bash -e {0}` (sin `pipefail`) — `VER=$(grep ... | head -1)` fallaba en silencio porque el exit status de la pipeline es el de `head` (éxito, aunque su entrada esté vacía), así que `VER` quedaba vacío pero el job seguía sin error visible (el nombre del artifact quedaba con un hueco: `BetelineyLauncher--Linux-x86_64`, nadie lo notó). El shell del job Windows es el wrapper `msys2.CMD`, que sí propaga el fallo de `grep` (exit 1, sin match) a través de la pipeline — abortando el script completo sin ningún output visible en el log, solo `Process completed with exit code 1`.

Fix: leer `Launcher_VERSION_MAJOR`/`MINOR`/`PATCH` por separado con tres greps simples y componer `VER="${MAJOR}.${MINOR}.${PATCH}"`. Probado localmente antes de pushear: `VER=8.3.0` correcto.

**Bug #9 — falta el paquete de 7-Zip en el setup de MSYS2 (commit `d33322c`)**

Con los bugs #3-#8 corregidos, "Empaquetar" en Windows falló con `7z: command not found` (exit 127). La lista de paquetes `install:` de `msys2/setup-msys2@v2` no incluía `mingw-w64-x86_64-7zip`, y el script de empaquetado invoca `7z a` directamente para generar el `.zip`. Fix: agregado `mingw-w64-x86_64-7zip` a la lista de instalación.

**Confirmación final:** corrida `28694701624` — **Linux y Windows ambos en verde** (`✓ Windows (msys2 MinGW64) in 13m47s`, `✓ Linux (Ubuntu 24.04) in 8m25s`), incluyendo "Compilar", "Obtener version" y "Empaquetar" en los tres. "Crear Release" quedó como skipped (`-`), esperado — ese job solo dispara con push de tags, no con push normal a `main`. Artifacts subidos correctamente: `windows-build`, `windows-ninja-build-log`, `linux-build`.

**Total de bugs reales de CI encontrados y corregidos entre las sesiones 16 y 17: 9.** Ninguno era el mismo tipo de error que el anterior — cada fix exponía el siguiente fallo real, nunca un síntoma repetido. Confirma la lección de la sesión 16 ("compila limpio local ≠ pasa en CI") llevada un paso más: ni siquiera "compila en CI" garantiza que el pipeline entero (empaquetado incluido) funcione — cada etapa del workflow necesitó su propia verificación independiente.

**Commits de la sesión (en orden):** `6aaa17b`, `e63f8d7`, `10fc804`, `d33322c`.

**Tabla de pendientes — estado real actualizado:**

| # | Ítem | Estado |
|---|---|---|
| — | CI del repo principal pasando en verde | ✅ Confirmado — Linux y Windows compilan, empaquetan y suben artifacts sin fallos (corrida `28694701624`) |
| 1 | Secret `CURSEFORGE_API_KEY` en CI | ✅ Rotado por el usuario y confirmado funcionando en CI — corrida `28712624812` en verde (sesión 18) |
| — | **Rotar key de CurseForge expuesta** | ✅ Hecho por el usuario (sesión 18) |
| — | Purgar key vieja del historial de git | ⏸️ Decisión del usuario, pendiente. |
| 4 | Capturas de pantalla restantes | ✅ BetelineyPacks y perfiles JVM integradas al README (sesión 19). ⏳ Falta solo el panel de diagnóstico de logs (requiere forzar un crash de lanzamiento). |
| 6 | Publicar en r/feedthebeast, r/Minecraft, Discord Prism | ⏳ Manual. |
| 7 | Formulario OpenAI Codex for OSS | ⏳ Manual. |

### Sesión 23 — Acceso directo en Escritorio + auditoría de confusión de nombres "Beteliney" en el sistema (2026-07-05)

**Contexto:** se pidió poner el launcher en el Escritorio y revisar si algo más se llama "Beteliney" en el sistema, por posible confusión.

**Hallazgo real de confusión de nombres — dos proyectos distintos, mismo nombre:**
1. **BetelineyLauncher (Minecraft)** — este repo. AppID `com.beteliney.BetelineyLauncher`.
2. **Beteliney (Roblox)** — proyecto completamente distinto en `~/Descargas/Beteliney Launcher [Roblox]/`, un fork de Sober (launcher de Roblox para Linux). AppID `org.beteliney.Beteliney`.

**Problema real encontrado y corregido:** en `~/.local/share/applications/` había 3 archivos `.desktop`:
- `beteliney.desktop` (genérico, sin AppID) — apuntaba correctamente al `lanzar.sh` de Minecraft actual. Parche manual creado el 12/06.
- `com.beteliney.BetelineyLauncher.desktop` — el `.desktop` "oficial" de este proyecto (con `MimeType` completo para modrinth/curseforge/`beteliney://`), pero con el `Exec` **roto**: apuntaba a `.../Beteliney Launcher [Minecraft] 7.0v/lanzar.sh`, una ruta con sufijo de versión vieja que ya no existe (confirmado, la carpeta actual no tiene sufijo). Este archivo quedó desactualizado desde el 23/05 tras un rename de carpeta, y el genérico de arriba fue el parche manual del usuario para no quedarse sin lanzador funcional.
- `org.beteliney.Beteliney.desktop` — el de Roblox, correcto, sin tocar.

**Confusión adicional, no corregida por ser de otro proyecto:** `~/.local/bin/beteliney` (en el `$PATH`) es el binario del launcher de **Roblox**, confirmado por hash MD5 idéntico al binario en `Beteliney Launcher [Roblox]/beteliney/beteliney`. Si se ejecuta `beteliney` desde una terminal esperando abrir Minecraft, en realidad abre Roblox/Sober. No se tocó porque pertenece a la gestión de otro proyecto instalado por su propio instalador — cambiarlo sin permiso explícito podría romper esa instalación. Queda anotado para que el usuario decida si quiere renombrar ese binario o el símbolo del PATH.

**Corregido:**
- `com.beteliney.BetelineyLauncher.desktop`: `Exec` corregido a la ruta real actual (`bash "/home/pibe/Descargas/Beteliney Launcher [Minecraft]/lanzar.sh" %U`). Validado con `desktop-file-validate` — sin errores.
- Ícono verificado: `~/.local/share/icons/hicolor/scalable/apps/com.beteliney.BetelineyLauncher.svg` ya era idéntico (diff) al logo real del repo — no había que tocarlo, ya estaba instalado correctamente desde antes.
- Eliminado `beteliney.desktop` (genérico, redundante una vez arreglado el oficial — tenía menos metadata, sin `MimeType`).
- `update-desktop-database` corrido para refrescar la caché.

**Acceso directo en Escritorio:** copiado `com.beteliney.BetelineyLauncher.desktop` a `~/Escritorio/BetelineyLauncher.desktop`, permisos `755` (idénticos al `Paralives.desktop` que ya funciona ahí). `gio set metadata::trusted` no aplica en este entorno (KDE Plasma, no GNOME/Nautilus) — en KDE el bit ejecutable es suficiente, no hace falta el atributo extra.

**Nota:** estos cambios son a nivel de sistema operativo del usuario (`~/.local/share/applications`, `~/Escritorio`), no del repo — se documentan acá por ser la fuente de verdad de sesiones, pero no generan commit.

### Sesión 22 — Limpieza y organización de estructura del repo (2026-07-05)

**Contexto:** se pidió limpiar y organizar la estructura general. Auditoría real antes de mover nada, no reorganización cosmética a ciegas.

**Intento revertido conscientemente:** se evaluó mover los scripts de compilación/empaquetado de la raíz (`COMPILAR_LINUX.sh`, `COMPILAR_BETELINEY.bat`, `COMPILAR.ps1`, `EMPAQUETAR_*`, `MONTAR_WINDOWS_NOBARA.sh`) a `scripts/` para despejar la raíz. Se revirtió al confirmar que `COMPILAR_LINUX.sh` usa `SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"` y luego rutas relativas (`build/`, `libraries/libnbtplusplus/`, `.git/HEAD`) asumiendo que el script vive en la raíz del repo. Moverlo sin reescribir esa lógica interna habría roto el build. Además, 13 archivos (README, LEEME.txt, docs/*) referencian estos scripts asumiendo ejecución desde la raíz — el riesgo/beneficio no lo justificaba sin poder recompilar y confirmar en el momento. **Se dejaron donde estaban.**

**Falso positivo descartado:** `COPYING.md` (raíz) y `docs/COPYING.md` parecían duplicados (contenido idéntico, confirmado con `diff`). No lo son: `docs/COPYING.md` está referenciado por `launcher/resources/documents/documents.qrc` y se embebe como recurso Qt dentro del binario compilado (se muestra en la app); `COPYING.md` de la raíz es el que GitHub detecta automáticamente como licencia del repo. Cumplen roles distintos — no se tocó ninguno.

**Limpieza real aplicada:**
- Eliminados `DEVLOG.md` y `MASTER_PLAN.md` — eran stubs de 3 líneas ("DEPRECATED, reemplazado por ESTADO.md") desde hace varias sesiones, sin contenido útil, y solo referenciados históricamente en este mismo documento.
- Renombrado `tools/pc/` → `tools/windows-fixes/` (scripts `ACTUALIZAR_DRIVERS.bat`, `DESHABILITAR_FAST_STARTUP.bat`, `DESHABILITAR_HYPERV.bat`). "pc" no describía nada; confirmado por búsqueda de código que ningún otro archivo referencia esa ruta por nombre, así que el rename es seguro.
- Confirmado que `dist/` (ignorado en `.gitignore`) no tiene ningún archivo trackeado por error.
- Búsqueda de archivos temporales/basura sueltos (`*.tmp`, `*.log`, `*~`, `*.orig`, `*.rej`) en el árbol — ninguno real encontrado.

**Conclusión:** la estructura del repo ya estaba, en general, bien pensada — lo que parecía desorden (scripts en la raíz, "duplicados" de licencia) tenía razones funcionales reales. La limpieza real posible era acotada: 2 archivos basura eliminados, 1 carpeta renombrada.

### Sesión 21 — Ícono macOS (Icon Composer) corregido + bug en genicons.sh (2026-07-05)

**Contexto:** se pidió mejorar la estética general (íconos, logo, UI, fondos). Revisión real del branding existente, no cambios cosméticos a ciegas.

**Estado encontrado:**
- Logo principal (`com.beteliney.BetelineyLauncher.logo.svg`) y tema de widgets (`BetelineyTheme.cpp`) — ya correctos: hexágono deep-space + "B" en gradiente verde neón `#39FF14`, paleta consistente, tipografía JetBrains Mono. No se tocaron.
- **Bug real encontrado:** el ícono para macOS moderno (formato Icon Composer, `program_info/BetelineyLauncher.icon/Assets/block.svg` + `rainbow.svg`) seguía siendo literalmente el logo de PrismLauncher sin reemplazar (`<title>Prism Launcher Logo</title>` en el XML), mientras el resto del proyecto ya tenía el logo propio.

**Fix aplicado:** reemplazo de `block.svg` y `rainbow.svg` por el hexágono-B real, escalado matemáticamente desde el logo oficial 512×512 a las unidades del formato Icon Composer (12.7×12.7), separado en las dos capas que pide ese formato (capa base opaca + capa "glass" con contorno y puntos de vértice en glow). Verificado por histograma de color, no a ojo: verde en `#37F513` (`#39FF14` con antialiasing esperado) y gradiente deep-space correcto. Backups `.bak` generados y luego excluidos vía `.gitignore` (no hacía falta versionarlos, git ya guarda el original en el historial).

**Segundo bug encontrado en esta sesión, no en la anterior:** `genicons.sh` tenía el `LAUNCHER_APPID` ya corregido a `com.beteliney.BetelineyLauncher` de una pasada previa, pero los nombres de archivo de salida seguían hardcodeados como `prismlauncher.ico`/`prismlauncher.icns` — mientras los archivos reales del proyecto son `beteliney.ico`/`beteliney.icns` (confirmado listando `program_info/`). Esto habría generado íconos con el nombre equivocado en cada regeneración, y `rm prismlauncher.ico` habría fallado porque ese archivo nunca existe en este repo (corregido a `rm -f beteliney.ico` de paso). Corregidos todos los nombres intermedios (`beteliney_16.png`...`beteliney_256.png`, `beteliney.iconset`) y los dos `cp`/`icotool` finales.

**Pendiente, decisión del usuario:** los fondos (`resources/backgrounds/`) siguen siendo las mascotas heredadas de PrismLauncher (`rory`, `kitteh`, `teawie` — esta última con licencia CC BY-SA de terceros, atribución obligatoria en el `.qrc`). Reemplazarlos requiere diseñar ilustración propia de marca y, si se quita `teawie`, retirar correctamente su atribución legal del `.qrc`. No se tocó sin definición explícita.

### Sesión 20 — Backport de Prism 11.0.0→11.0.2 testeado en la práctica (2026-07-05)

**Contexto:** entre la sesión 19 y esta, se hizo un backport de 3 fixes reales de PrismLauncher upstream (commit `a7795abe7`, ya pusheado y con CI verde confirmado antes de retomar esta sesión). Se pidió documentar todo y testear en la práctica, no solo confiar en que CI pasó.

**Los 3 fixes backporteados:**
1. `LaunchProfile::getLibraryFiles` — nuevo parámetro `addJarMods` (default `true`) para poder pedir la lista de jars sin el jar-mods merge.
2. `EnsureOfflineLibraries` — ya no cuenta los jar mods como si fueran librerías normales (se fusionan después, en un paso posterior), y cuando faltan librerías reales ahora loguea la lista completa en vez de fallar con un mensaje genérico sin detalle.
3. `BetelineyUpdater` — el signal `finished` de la descarga ya no resetea `m_current_task`, lo que causaba un softlock del updater cuando necesitaba traer más de 1 página de releases de GitHub.

**Verificación de la base real antes del backport (ya hecha, documentada en el mensaje del commit):** diff limpio del fork contra el tag `11.0.0` de PrismLauncher en los archivos no tocados por branding — confirma que el fork parte exactamente de esa versión. El gap real a 11.0.2 son 24 commits / 16 archivos; de esos, solo 3 eran fixes reales aplicables sin conflicto con la customización propia. El resto queda pendiente de revisión manual uno por uno (no se tocan a ciegas): `MinecraftInstance.cpp` (toggle LowMemWarning), `JavaSettingsWidget.cpp/.ui` (checkbox — el `.ui` requeriría rehacer el layout entero en Qt Designer), `PrintInstanceInfo.cpp` (ya diverge ~50 líneas por logging propio), `McClient.cpp/h` + `ManagedPackPage.cpp` (fix de changelog de Modrinth + fix de pack upgrade — es un refactor grande de `McClient`, no un fix aislado).

Remote `upstream` (`PrismLauncher/PrismLauncher`) agregado al repo local para poder diffear contra versiones futuras.

**Problema de tooling encontrado y corregido en esta sesión — `gh` apuntaba al repo equivocado:** al agregar el remote `upstream` en la sesión del backport, `gh repo view` empezó a resolver por defecto a `PrismLauncher/PrismLauncher` en vez de `ElPibeCapo/BetelineyLauncher`, pese a que `origin` seguía apuntando correctamente y el directorio de trabajo era el correcto. `gh run list` sin `--repo` explícito devolvía corridas de CI de PrismLauncher (irrelevantes), lo cual habría hecho perder tiempo revisando el pipeline equivocado si no se hubiera verificado con `gh repo view --json nameWithOwner` antes de confiar en el resultado. Fix: `gh repo set-default ElPibeCapo/BetelineyLauncher` — confirmado que a partir de ahí `gh` resuelve al repo correcto en este directorio. **Lección operativa para sesiones futuras: verificar siempre `gh repo view --json nameWithOwner` antes de confiar en `gh run list`/`gh pr list` sin `--repo` explícito, especialmente en repos con más de un remote configurado.**

**Testeo real hecho en esta sesión (no solo confiar en el verde de CI):**
- CI: corrida `28716550226` — `completed success`, confirmado con el repo default ya corregido.
- Build local incremental limpio: `ninja -C build -j$(nproc)` desde el binario existente (previo al backport) — 91 pasos, terminó con exit code 0, cero errores y cero warnings pese a tener `-Werror` activo en `Launcher_logic` y `BetelineyLauncher` (cualquier warning real habría abortado el build). Tardó varios minutos por el LTO con recursos limitados de esta máquina (8 tests linkeando en paralelo con LTO en una APU con 13.5 GB de RAM compartida) — normal en este hardware, no indica ningún problema.
- Ejecución real del binario recompilado (`lanzar.sh --offline`): arrancó y quedó estable, sin crash dumps en `/tmp/beteliney_crash_*`. Confirmó además, como efecto colateral útil, que **la descarga real desde el meta server funciona en producción** (no solo probado con `curl` como en la sesión 15): el log de la sesión mostró `Net::Download(...) Request succeeded` tanto para `known-hashes.json` como para `feed.atom`, y la noticia "BetelineyLauncher v8.3.0 ya disponible" se cargó correctamente en la UI.
- `ctest` sobre el build recompilado: **29/29 tests pasando, 100%**, incluyendo los 16 de `BetelineyTranslation` (sesión 12) y los tests base (`Library`, `Task`, `Version`, `INIFile`, etc.) — el backport no rompió ningún test existente.

**Conclusión:** el backport está confirmado en 4 niveles independientes (CI real, build local limpio, ejecución real estable, suite de tests completa) — no solo "CI dice que pasó".

**Pendientes reales identificados para sesiones futuras, sin tocar todavía:**
- 21 commits / 13 archivos restantes del gap Prism 11.0.0→11.0.2, listados arriba, cada uno requiere revisión manual uno por uno por el conflicto con customización propia (especialmente el refactor de `McClient`).
- El resto de la tabla de pendientes no cambió respecto a la sesión 19 (ver abajo).

### Sesión 19 — Capturas de pantalla integradas al README + corrección de Roadmap desactualizado (2026-07-04)

**Contexto:** el usuario ya había tomado 46 capturas de pantalla navegando el launcher manualmente (`/home/pibe/Imágenes/Capturas de pantalla/`), tras el bloqueo de automatización en Wayland de sesiones anteriores (sin `ydotool`/`kdotool` funcionando de forma confiable, foco de ventana robado por la app de Claude Desktop). Se pidió revisar todo y elegir las mejores para documentar.

**Método:** en vez de adivinar el contenido visual de 46 archivos, se corrió `tesseract` (OCR) sobre las 46 capturas en un solo batch, extrayendo el texto visible de cada una. Esto permitió identificar con certeza (no por inspección visual subjetiva) qué pantalla del launcher representa cada archivo:
- **BetelineyPacks:** identificada por el texto "Modpacks de Beteliney", "Vanilla Optimizado", "PVP Competitivo", "Destacado" — elegida `Captura...121049.png` (primera vista de la pestaña, sin scroll).
- **Perfiles JVM:** identificada por "Perfiles Beteliney (Ryzen 7 3700U + Vega 10)" + dropdown de perfiles — elegida `Captura...121642.png` porque muestra el desplegable abierto con varios perfiles listados (más informativa que las capturas con el dropdown cerrado).
- **Diagnóstico de logs:** no se encontró ninguna captura del panel real (`BetelineyLogAnalyzer`/`diagnosisPanel`) porque ese panel solo se activa cuando un lanzamiento de Minecraft termina con `gameExitCode != 0` — el usuario nunca forzó ese escenario. Sí hay una captura de "View Launcher Logs" (ventana de logs cruda, sin el panel de diagnóstico), ya identificada pero **no usada** para no hacer pasar una cosa por otra. Queda pendiente en el Roadmap, con la causa exacta documentada.

Archivos copiados a `source/screenshots/`: `betelineypacks.png`, `perfiles-jvm.png`.

**README actualizado:**
- Galería de capturas ampliada (ventana principal + BetelineyPacks + perfiles JVM lado a lado).
- Link de Discord corregido: `discord.gg/2JdB7pvBq3` (el original de la sesión 9, ya no válido/reemplazado por el usuario) → `discord.gg/fMbSkEd85r`. Corregido también en `ESTADO.md` (sección Sesión 9) y `ESTRATEGIA_IA.md` (tabla de enlaces).
- **Roadmap corregido — hallazgo no pedido pero relevante:** tenía 3 ítems marcados como pendientes (`[ ]`) que en realidad ya estaban resueltos desde las sesiones 14 a 17 según este mismo documento: activar GitHub Pages del repo `meta`, publicar los 3 BetelineyPacks, y `known-hashes.json`. El README nunca se había sincronizado con el avance real documentado acá. Marcados `[x]` con la aclaración exacta de cada uno (incluida la honestidad de que `known-hashes.json` quedó vacío por diseño, no relleno con datos inventados — ver sesión 14). También se agregó al Roadmap la rotación de la CurseForge API key (sesión 18) y el pendiente de publicar en Reddit/Discord de Prism, que no estaban listados ahí.




### Sesión 24 — Investigación y comparación con otros launchers para priorizar mejoras futuras (2026-07-05)

**Contexto:** el usuario pidió una investigación con lógica, verificación y comparación real (no opiniones) sobre qué mejorar en el launcher, qué tecnología usar, y si valía la pena portar código/ideas de otros launchers. Sesión sin cambios de código — es research y priorización para sesiones futuras.

**Decisión de fondo (verificada con research externo):** no cambiar de stack. Comparando contra Modrinth App (Rust/Tauri), CurseForge App (JS sobre Overwolf) y ATLauncher, los rankings 2026 siguen poniendo a los launchers basados en Prism (Qt/C++) primero por control y estabilidad. Reescribir a otro framework sería meses de trabajo sin ganancia real, solo estética. Portar código literal de otros launchers no es viable salvo con Prism mismo (mismo lenguaje/framework) — con el resto, como mucho se pueden tomar ideas de UX, no código.

**Verificado en el código propio (no supuesto) — cosas que ya existen, no son gaps:**
- Soporte FTB (`launcher/modplatform/ftb/`, `import_ftb/`) — viene de la base Prism 11.0.0 forkeada, que restauró soporte FTB (sacado en la 7.0) y agregó trackeo de dependencias de mods.
- Galería de screenshots (`launcher/ui/pages/instance/ScreenshotsPage.cpp`) — ya existe.
- Exportar instancia a `.mrpack` (`ExportPackDialog.cpp`) — ya existe.
- `BetelineyPackListModel.cpp` ya tiene el enum `PackProvider::CurseForge` implementado en la arquitectura, pero los 3 presets reales en `BetelineyPresets.h` apuntan 100% a `cdn.modrinth.com` — ninguno usa CurseForge todavía pese a que la infraestructura ya lo soporta.

**Verificado como ausente (gaps reales, no inventados):**
- Discord Rich Presence a nivel de proceso launcher/juego — solo existen íconos SVG estáticos de Discord (`resources/*/discord.svg`), cero integración RPC.
- Backup automático de mundos (`saves/`) antes de actualizar mods/pack — no existe ningún módulo de backup.
- Badge de "actualización disponible" en la card de instancia — no existe, la info ya la trae la API de Modrinth/CurseForge pero no se muestra agregada.
- Servidores favoritos / quick-join desde el dashboard — no existe ningún `FavoriteServer` o similar.
- Command palette tipo Ctrl+K (búsqueda universal de instancias/settings) — no existe, no hay ningún `CommandPalette`/`QuickSearch`.
- Sistema de logros/gamificación ligado a la marca (neón gamer) — no existe en Beteliney ni se encontró en ningún launcher competidor revisado (Prism, Modrinth App, ATLauncher, CurseForge, GDLauncher Carbon) — sería diferencial único, no copiado.

**Descartado explícitamente, con razón:**
- Overlay de FPS/RAM in-game — capa incorrecta: requiere inyectar en el proceso de Minecraft (hook LWJGL o mod), no es código de launcher. Terreno de mods tipo BetterFPS, fuera de alcance.
- Diseño Fluent nativo de Windows (como FluentLauncher) — chocaría con el tema neón de marca propio, no suma.
- Búsqueda combinada Modrinth+CurseForge en una sola pestaña — sí es una mejora real (hoy `atlauncher/`, `flame/`, `ftb/`, `modrinth/`, `technic/`, `beteliney/` son pestañas separadas por proveedor, ningún launcher tipo Prism lo resuelve bien en 2026 según reviews), pero de alto esfuerzo (dedup por hash entre dos APIs) — no es para ahora.

**Hallazgo con validación cruzada (research externo + pendiente interno ya anotado):** el refactor pendiente de `McClient.cpp`/`ManagedPackPage.cpp` (gap Prism 11.0.0→11.0.2, ver sesión 20) es exactamente la funcionalidad de "ver el changelog del modpack en la página de instancia" — Prism la rompió en 11.0.0 y la restauró en 11.0.1 (PR #5354). Reviews externas de 2026 la destacan como diferencial de calidad. Se recomienda subir su prioridad dentro de los 21 commits pendientes del gap, en vez de dejarla para el final por ser "el refactor grande".

**Lista priorizada de mejoras propuestas (research + código, sin implementar todavía — pendiente de que el usuario decida cuáles hacer):**
1. Backup automático de mundos con timestamp antes de update de instancia — bajo esfuerzo, alto valor (red de seguridad real).
2. Servidores favoritos + quick-join desde el dashboard — bajo esfuerzo, usa el flag `--server` que Minecraft ya soporta por CLI.
3. Command palette Ctrl+K (`QDialog` flotante + `QCompleter`, patrón estándar en Qt) — bajo esfuerzo, sensación de herramienta pulida.
4. Badge de mods con actualización disponible en la card de instancia — bajo esfuerzo, reusa llamadas API que el launcher ya hace.
5. Discord Rich Presence a nivel de proceso (librería oficial `discord-rpc`, C++, MIT) — bajo esfuerzo, cierra el círculo con el servidor de Discord ya promocionado.
6. Subir prioridad al backport de `McClient`/`ManagedPackPage` (changelog de modpack) dentro del gap Prism pendiente.
7. Preset de BetelineyPacks con fuente CurseForge, aprovechando el enum `PackProvider::CurseForge` ya existente en el código.
8. Sistema de logros de marca (gamificación ligada al tema neón) — diferencial único frente a toda la competencia revisada, sin backend, solo estado local.
9. i18n propio conectado a Weblate — ya estaba en el Roadmap del README, sigue sin resolver.
10. Búsqueda combinada Modrinth+CurseForge en una sola pestaña — alto esfuerzo, mediano/largo plazo, no urgente.

**Pendientes reales identificados para sesiones futuras, sin tocar todavía:** las 10 ideas de arriba, en el orden de prioridad indicado. Ninguna implementada aún — requiere que el usuario elija por cuál empezar.


### Sesión 25 — Plan de ejecución completo para las 10 mejoras + hallazgo de release sin tagear (2026-07-06)

**Contexto:** el usuario pidió un plan completo de qué sigue después de la investigación de sesión 24, con orden lógico, y qué reforzar cuando eso termine. Sesión de planificación pura — sin cambios de código.

**Hallazgo previo al plan, verificado con git (no supuesto):** el tag `v8.3.0` ya existe (creado sesión 8, 2026-06-19, apunta al commit `78adefe8bc5032ca16e54d353e81110beb4561b6`). Desde entonces hay **32 commits reales** sin ningún tag nuevo: los 3 backports de Prism (sesión 20), el fix del ícono macOS (sesión 21), la limpieza de estructura (sesión 22), el acceso directo de escritorio (sesión 23) y la investigación de sesión 24. `CMakeLists.txt` (líneas 179-181) sigue con `Launcher_VERSION_MAJOR/MINOR/PATCH` en `8.3.0` literal, sin bump. Conclusión: hay trabajo real, probado y funcional (91 pasos de build limpio, 29/29 tests, ver sesión 20) que nunca llegó a un release público.

**Paso 0 — antes de cualquier feature nueva, cero riesgo, cero código:**
1. Bump de versión en `CMakeLists.txt` líneas 179-181: `8.3.0` → `8.4.0`.
2. `git tag v8.4.0 && git push --tags`.
3. Generar release notes en GitHub Releases a partir de las sesiones 9-24 de este mismo documento (no inventar contenido, reusar lo ya documentado).

**Plan de las 10 mejoras de sesión 24, organizado en 4 fases por dependencia técnica real (verificada en código, no orden arbitrario):**

**Fase 1 — bajo riesgo, reutiliza infraestructura ya existente:**
- Backup automático de mundos: reusa `BetelineyZip.h` (mismo wrapper de compresión que ya usa `ExportPackDialog.cpp` para exportar `.mrpack`) — no requiere librería nueva. Hook en el punto donde arranca la actualización de instancia/pack, antes de tocar `saves/`.
- Badge de mods con actualización disponible: las llamadas a las APIs de versión de Modrinth/CurseForge ya existen en el flujo de actualización de mods — falta cachear el resultado y pintarlo en la card de instancia (`InstanceView`/delegate correspondiente).

**Fase 2 — UX rápida, mismo patrón de bajo riesgo:**
- Command palette Ctrl+K: sigue el patrón ya usado en `MainWindow.cpp` (confirmado uso extensivo de `QShortcut`/`QKeySequence` para atajos existentes, líneas ~261-287 y 715) — `QDialog` flotante + `QCompleter`, nada nuevo conceptualmente.
- Servidores favoritos + quick-join: se apoya en `SettingsObject.h`, el sistema de persistencia de configuración ya existente — sin mecanismo de guardado nuevo. Lanza con el flag `--server` que Minecraft ya soporta por CLI.

**Fase 3 — marca y comunidad:**
- Discord Rich Presence: única dependencia externa nueva de todo el plan — librería `discord-rpc` (C++, MIT).
- Preset de BetelineyPacks con fuente CurseForge: el enum `PackProvider::CurseForge` ya existe en `BetelineyPackListModel.cpp`, la API key de CurseForge ya está rotada y cargada en CI (sesión 18) — solo falta escribir el dato del preset nuevo en `BetelineyPresets.h`.
- Sistema de logros de marca: reutiliza `totalTimePlayed()`/`lastTimePlayed()`, accesores ya existentes y confirmados en `MinecraftInstance.cpp` (línea ~1086-1097) — sin trackeo nuevo. Persistencia de logros desbloqueados vía `SettingsObject.h`. No hay sistema de notificación/toast previo en el código (confirmado, búsqueda vacía) — la UI de notificación de logro desbloqueado es la única pieza nueva de UI de esta fase.

**Fase 4 — trabajo pesado, al final a propósito por ser lo más delicado:**
- Backport de `McClient.cpp`/`ManagedPackPage.cpp` (changelog de modpacks, ya identificado como prioridad en sesión 24 dentro del gap Prism 11.0.0→11.0.2) — mismo nivel de cuidado que los 3 backports ya hechos en sesión 20, no apurar.
- i18n conectado a Weblate — trabajo de proceso/configuración, no de código, puede llevarse en paralelo sin bloquear nada del resto.
- Búsqueda combinada Modrinth+CurseForge en una sola pestaña — fuera del sprint principal, proyecto aparte de mediano/largo plazo (arquitectura nueva, dedup entre dos APIs).

**Fase 5 — refuerzo, DESPUÉS de completar las fases 1-4, tal como pidió el usuario explícitamente ("cuando eso termine, reforzar"):**
- Repetir con cada feature nueva el mismo nivel de validación usado en el backport de sesión 20: no solo CI en verde, sino build local limpio (`ninja -C build`), ejecución real del binario, y `ctest` completo — cada feature nueva debería sumar sus propios tests a la suite (hoy 29/29).
- Capturas de pantalla nuevas para el README siguiendo el método de sesión 19 (OCR con `tesseract` sobre capturas reales tomadas a mano, nunca inventar contenido visual).
- Actualizar el Roadmap del README quitando de la lista de "ideas de mejora futuras" lo ya resuelto.
- Tag de versión nueva (`8.5.0` o el número que corresponda) una vez cerrado el bloque completo.
- Recién ahí, publicar en r/feedthebeast, r/Minecraft, Discord de Prism Launcher (ítem ya en el Roadmap desde antes) — tiene más sentido promocionar con el pulido de las 4 fases encima que con la versión actual sin tagear.

**Orden de sesiones sugerido (uno por sesión, mismo patrón incremental ya usado en todo este proyecto, para no mezclar cambios grandes):**
- Sesión 26: Paso 0 (bump de versión + tag + release).
- Sesión 27: backup de mundos.
- Sesión 28: badge de updates de mods.
- Sesión 29: command palette Ctrl+K.
- Sesión 30: servidores favoritos + quick-join.
- Sesión 31: Discord Rich Presence.
- Sesión 32: preset BetelineyPacks CurseForge.
- Sesión 33: sistema de logros de marca.
- Sesión 34: backport `McClient`/`ManagedPackPage` (con tiempo dedicado, el más delicado).
- Sesión 35: refuerzo completo (tests, capturas, tag final de la tanda).
- Sesión 36+: i18n/Weblate y búsqueda combinada Modrinth+CurseForge, sin fecha fija, no bloquean nada del resto.

**Pendientes reales identificados para sesiones futuras, sin tocar todavía:** todo el plan de arriba, empezando por el Paso 0 (bump de versión y release), que es lo más urgente por ser trabajo ya terminado sin publicar.


### Sesión 26 — Auditoría con acceso real al código y a APIs en vivo: correcciones críticas al plan de sesión 24/25

**Contexto:** las sesiones 24 y 25 se hicieron sin acceso al repositorio (solo memoria/razonamiento). Apareció un archivo suelto sin trackear, `PLAN_MEJORAS.md`, generado por una sesión con acceso real al código, que corrige varios supuestos. Esta sesión 26 verifica cada claim de forma independiente (grep sobre el código real + llamadas en vivo a la API de Modrinth) y agrega hallazgos nuevos no detectados antes.

### Correcciones confirmadas al plan de sesión 24/25

1. **El changelog de modpacks NO era un gap.** Sesión 24 identificó `McClient.cpp`/`ManagedPackPage.cpp` como backport prioritario pendiente del gap Prism 11.0.0→11.0.2. Falso: `ManagedPackPage.cpp` ya renderiza el changelog completo tanto de Modrinth (`markdownToHTML(version.changelog)`) como de CurseForge (`m_api.getModFileChangelog(...)`). No hay backport pendiente acá.

2. **Preset CurseForge en BetelineyPacks no es "solo escribir el dato".** El enum `PackProvider::CurseForge` existe, pero `BetelineyPackInstallTask.cpp` salta cualquier mod sin URL con un comentario explícito citando los ToS de CurseForge. Verificado externamente: no es una prohibición general, es el **"Project Distribution Toggle"** — cada autor de mod decide si su proyecto es descargable vía API de terceros. Cuando está desactivado, ningún launcher externo (Prism incluido) tiene la URL directa. El patrón real que usa Prism para esto es un flujo de "abrir navegador → usuario descarga manual → launcher detecta el archivo" — Beteliney no lo implementa, simplemente descarta el mod. Esfuerzo real: medio, no bajo.

3. **Badge de actualización de mods: gap real y verificado, más importante de lo que parecía.** `grep` sobre `launcher/` completo confirma: `setUpdateAvailable(bool)` (`BaseInstance.h`) tiene **un solo match en todo el árbol — su propia definición.** Nadie lo llama. `InstanceDelegate.cpp` sí lee `hasUpdateAvailable()` y pinta el ícono `checkupdate` en la card — la UI y el modelo están 100% listos, es una feature fantasma completa. Los dos puntos de enganche verificados: `ModrinthCheckUpdate`/`FlameCheckUpdate` (ambos vía `CheckUpdateTask`) solo se instancian dentro de `ResourceUpdateDialog.cpp` (el usuario tiene que abrir el diálogo manualmente); y `ManagedPackPage::suggestVersion()` solo corre cuando el usuario abre esa página o cambia el combo de versión. No hay ningún trigger pasivo al cargar `InstanceList`. Para cerrar esto de verdad hace falta un chequeo en background al iniciar el launcher o al refrescar la lista de instancias, no solo cachear un resultado ya existente.

### Hallazgo nuevo — bug crítico activo, no detectado ni por sesión 24/25 ni por PLAN_MEJORAS.md

**Las 5 URLs de descarga hardcodeadas en `BetelineyPresets.h` (los mods de los presets built-in "Vanilla Optimizado" y "PvP Competitivo") están rotas ahora mismo.** Verificado en vivo contra el CDN de Modrinth: las 5 devuelven HTTP 404 (versiones movidas/eliminadas de Modrinth). Esto significa que **hoy, cualquier usuario que instale uno de estos dos presets falla en el 100% de los mods.** No es un problema de seguridad apagada (falta de hash) — es una feature core completamente rota. Prioridad más alta que cualquier otra cosa del plan de sesión 24/25.

También se corrige el conteo: PLAN_MEJORAS.md decía "9 mods sin hash". Contando el archivo real: 7 (Sodium/Lithium/Iris/ModernFix en Vanilla Optimizado, Sodium/Lithium/FerriteCore en PvP Competitivo; el preset NeoForge es base limpia sin mods).

**Datos reales obtenidos en vivo de la API de Modrinth (`api.modrinth.com/v2`), filtrados por `loaders=fabric` + `game_versions=1.21.1`, listos para reemplazar en `BetelineyPresets.h`:**

| Mod | versión | version id | sha512 |
|---|---|---|---|
| Sodium | mc1.21.1-0.8.12-fabric | KIRFiWG4 | `8afe411eec65a9f677611ed6390ce656e5a3572f9be473e5dca51ae882a9426a547cd2e8c793278577bb14c17e48158030b11753108926ef33698614bd94ed7f` |
| Lithium | mc1.21.1-0.15.4-fabric | N08Z8wog | `182064b00e6315e2255b857eaab8eb759e6b042ebd4cc8b855ff0d93f875a5a7188fac49f878d7b29d4ef7e6b6341190ad7f6f6f39f4a6d2c62003468b08e4c6` |
| Iris | 1.8.14-beta.1+1.21.1-fabric | bAo1Qhte | `a7fbb629793c52f0be8b049f787cb598879239b1ad8e1de62e103c8b9efff140e3232b93ef1f14e505d262897d8cf9505b1126396429ad4056bff969c8674e52` |
| ModernFix | 5.25.1+mc1.21.1 | NnNX8LBn | `dc67d6e023e1fcdeaf7837917c477cba212c611dfc2463c6ea021319c644087c79b477e0ea8194e113ddd7332fd5c6d82baa47c291eaac7f4a86252507b4e19f` |
| FerriteCore | 7.0.3-fabric | sOzRw3CG | `3ad31620fac4ff44327dc7dedbe162b2d978f3f246dc16255a6e400ce9592a0d326fe36a626f3c1bf30a11f813093cbb4dcc107af039cff724d0cdf648541fdf` |

### Corrección sobre Discord Rich Presence

La librería `discord-rpc` (C++, MIT) que sesión 24/25 propuso como "la única dependencia externa nueva del plan" está **deprecada oficialmente por Discord**, reemplazada por su GameSDK/Social SDK. El protocolo IPC local sigue funcionando y hay forks comunitarios activos (ej. el que mantienen proyectos como Borked3DS), así que sigue siendo viable técnicamente, pero no es "la oficial mantenida" como se dijo. Confirmado además que no hay nada heredado de Prism para esto — el ícono de Discord en el código actual es solo un link estático.

### Plan de acción inmediato (sesión 26)

1. Aplicar el fix de `BetelineyPresets.h` con las 5 URLs + 7 hashes reales de la tabla de arriba — cierra de raíz el bug de presets rotos y la Prioridad 1 de seguridad (hashes vacíos) en un solo cambio.
2. Build limpio + `ctest` para confirmar que no rompe nada.
3. Commit.
4. El resto del plan de sesión 25 (Paso 0 bump de versión, Fases 1-5) sigue vigente sin cambios salvo las correcciones de arriba.

### Estado de ejecución (previo a testear)

Paso 1 confirmado aplicado: `git diff` sobre `launcher/modplatform/beteliney/BetelineyPresets.h` muestra las 7 entradas de mods reemplazadas exactamente con los datos de la tabla de arriba (mismos version id, mismos sha512) — verificado de nuevo de forma independiente contra `api.modrinth.com` en este mismo chat antes de leer el diff, coincidencia exacta. Cambio sin commitear todavía (`M` en `git status`). `PLAN_MEJORAS.md` (el archivo suelto original que disparó la auditoría) sigue sin trackear, contenido ya absorbido y corregido en esta sección — se puede borrar tras el commit para no dejar dos fuentes de verdad.

Paso 2 (build + ctest) arranca ahora mismo, resultado se documenta abajo al terminar.

### Resultado de testing (paso 2, completo)

**Build incremental (`ninja -j$(nproc)`, Release con LTO):** exitoso, sin errores ni warnings nuevos. El header modificado compila limpio — `build/beteliney` se regeneró (16 MB, timestamp fresco).

**`ctest --output-on-failure`:** 29/29 tests pasan, 0 fallos, 3.30s total. Ningún test unitario existente cubre específicamente los datos de `BetelineyPresets.h` (no hay test dedicado a presets), así que este resultado confirma que el cambio no rompió nada existente, pero no valida por sí solo que los datos nuevos sean correctos.

**Test funcional end-to-end (el que realmente prueba el fix):** se descargaron los 5 jars reales desde las URLs nuevas del CDN de Modrinth y se calculó su SHA-512 real, comparándolo contra el hash hardcodeado en `BetelineyPresets.h`. Resultado: **5/5 coinciden exactamente.**

| Mod | Tamaño descargado | SHA-512 real vs. código |
|---|---|---|
| Sodium | 1,573,180 bytes | ✅ coincide |
| Lithium | 797,454 bytes | ✅ coincide |
| Iris | 2,791,343 bytes | ✅ coincide |
| ModernFix | 471,258 bytes | ✅ coincide |
| FerriteCore | 123,450 bytes | ✅ coincide |

Esto cierra el ciclo completo: las URLs funcionan (ya no 404), el archivo que se descarga es exactamente el que el hash dice que debe ser, y el binario compilado con estos datos pasa toda la suite de tests existente. El fix de `BetelineyPresets.h` queda validado de punta a punta, no solo "compila".

**Siguiente paso:** commit de `BetelineyPresets.h` + esta sección de ESTADO.md, y borrar `PLAN_MEJORAS.md` (contenido ya absorbido en sesión 26, no dejar dos fuentes de verdad).


**Actualización sesión 25 (mismo día) — hallazgo adicional tras seguir investigando (MalwareBazaar, privacidad, accesibilidad, atribución GPL):**

- **Reabre la conclusión de sesión 14** ("no hay lista pública de hashes de Fractureiser"): ya no es cierto. **MalwareBazaar** (`abuse.ch`) es una base de datos pública y gratuita de hashes de malware con API abierta, y tiene un corpus real etiquetado para Minecraft (`fractureiser`, `mavenrat`, `maksstealer`, `maksrat`). Existe además un proyecto open source de referencia, `jarspect` (GitHub, Microck), que hace exactamente lo que `MalwareScanner.cpp` intenta: hash contra MalwareBazaar + YARA + análisis de bytecode como fallback, probado sin falsos positivos/negativos contra 70 muestras de malware y 50 mods legítimos de Modrinth. Hash público confirmado del stage 1 de Fractureiser (`dl.jar`): SHA-1 `dc43c4685c3f47808ac207d1667cc1eb915b2d82`.
  - **Opción A (mínimo cambio, recomendada primero):** sembrar `known-hashes.json` (hoy vacío) con los hashes ya publicados/confirmados + una consulta filtrada por esos 4 tags a MalwareBazaar. No cambia la arquitectura actual (lista estática descargada por `MalwareScanner::loadIfNeeded()`).
  - **Opción B (más robusta, más trabajo, evolución futura):** que `MalwareScanner` consulte la API de MalwareBazaar en vivo por cada hash de mod al momento del escaneo, en vez de mantener una lista estática que envejece. Gratis para uso no comercial, requiere API key de `abuse.ch`.
  - Esto entra en **Fase 1** del plan (bajo esfuerzo, reusa `MalwareScanner.cpp` ya existente, solo cambia el contenido de los datos).

- **Telemetría/analytics: confirmado que NO existe ninguna** (grep limpio en todo `launcher/`, los matches iniciales eran falsos positivos de palabras sueltas). No es un gap — es una decisión de privacidad correcta y coherente con no tener servidor propio de datos de usuario. Vale la pena mencionarlo explícitamente en el README como argumento de venta ("cero telemetría") si no está ya.

- **Gap nuevo — accesibilidad:** no existe ningún tema de alto contraste ni consideración de accesibilidad (confirmado, no hay recursos `contrast`/`access` en `resources/`). Ningún launcher competidor revisado (Prism, Modrinth App, ATLauncher, CurseForge) lo tiene tampoco como feature destacada — sería diferencial original, no copiado, con impacto social real (usuarios con baja visión/daltonismo). Barato de implementar: el sistema de temas (`BetelineyTheme`) ya existe, solo falta una variante de paleta. Entra en **Fase 3** junto a los otros diferenciales de marca.

- **Nota menor de higiene legal:** no hay archivo `NOTICE`/`ATTRIBUTION` explícito (más allá de `COPYING.md`/`LICENSE` y la aclaración "No afiliado con Prism Launcher" en el README). GPLv3 §5(a) técnicamente pide aviso prominente de modificación en archivos cambiados — en la práctica casi ningún fork chico lo cumple al pie de la letra y no es un riesgo legal real hoy, pero queda anotado por si el proyecto crece y se quiere estar prolijo del todo.

### Sesión 27 — Paso 0 ejecutado (release v8.4.0) + arranque de Fase 1 (2026-07-07)

**Contexto:** continuación directa de la sesión 26. Se ejecutó el Paso 0 documentado en el bloque consolidado (bump + tag + release) y se arrancó la Fase 1 del plan (backup de mundos, sembrado de `known-hashes.json`, badge de updates).

**Paso 0 — completo, verificado en GitHub:**
1. Bump `CMakeLists.txt` líneas 179-181: `8.3.0` → `8.4.0`. Commit `7a3dbf14b` ("chore: bump version 8.3.0 → 8.4.0"), pusheado a `main`.
2. Tag `v8.4.0` creado con mensaje anotado (resumen de todo lo acumulado desde `v8.3.0`) y pusheado.
3. Release publicado en GitHub Releases con notas reales (no genéricas) armadas a partir de las sesiones 9-26 de este documento: https://github.com/ElPibeCapo/BetelineyLauncher/releases/tag/v8.4.0

**Intento de sembrar `known-hashes.json` — bloqueado, no completado, hallazgo real nuevo:** al ir a aplicar el hash SHA-1 de Fractureiser Stage 1 (`dc43c4685c3f47808ac207d1667cc1eb915b2d82`) que quedó anotado como "confirmado" en la sesión 26, se intentó verificarlo de forma independiente contra la API de MalwareBazaar (`mb-api.abuse.ch`) antes de escribir nada. La API devolvió `{"error": "Unauthorized"}` — **MalwareBazaar ahora exige `Auth-Key` para absolutamente todas las queries de la API**, incluida la consulta simple por hash (antes solo se sabía que hacía falta key para descargar la muestra completa, ver Opción B de la sesión 26; ahora aplica también a la Opción A, la de solo consultar). Sin esa key no hay forma de verificar el hash ni de obtener el SHA-256/SHA-512 real asociado (el scanner de Beteliney solo soporta esos dos algoritmos, no SHA-1). **No se escribió ningún hash en `known-hashes.json` — se dejó exactamente como estaba (arrays vacíos), para no fabricar un dato no verificado.** Repo: `/home/pibe/Descargas/meta_beteliney` (clon local separado del repo principal). Pendiente real: conseguir una API key de `abuse.ch` (gratis para uso no comercial, requiere registro) antes de poder avanzar esto.

**Feature implementada — backup manual de mundos, reusando infraestructura existente:**
- Nuevo botón "Backup" en la toolbar de `WorldListPage` (pestaña Worlds de cada instancia), junto a Copy/Rename/Delete — sigue exactamente el mismo patrón `on_action*_triggered()` que ya usan las demás acciones del archivo.
- Al presionarlo con un mundo seleccionado: comprime ese mundo completo a un `.zip` con timestamp (`{nombre}_{yyyyMMdd_HHmmss}.zip`) dentro de `backups/worlds/` en la raíz de la instancia. Reusa `BetelineyZip::collectFileListRecursively` + `BetelineyZip::ExportToZipTask` — el mismo mecanismo que ya usa `ExportInstanceDialog.cpp` para exportar la instancia entera a zip. Cero librerías nuevas, cero mecanismo de compresión nuevo.
- Progreso visible vía `ProgressDialog` (mismo patrón que el export de instancia), con manejo de error vía `CustomMessageBox` si falla.
- Archivos tocados: `launcher/ui/pages/instance/WorldListPage.ui` (acción `actionBackup` + separador en la toolbar), `WorldListPage.h` (slot nuevo), `WorldListPage.cpp` (implementación + habilitado/deshabilitado según selección en `worldChanged`).
- **Alcance real de esto — corrección honesta sobre lo que dice el plan de sesión 25:** esto es un backup **manual**, a pedido del usuario desde la UI de Worlds — no es todavía el "backup automático antes de actualizar el pack" que proponía la sesión 24/25. Ese automatismo requeriría enganchar el mismo `ExportToZipTask` en el punto donde arranca cada actualización de pack (distinto por proveedor: Modrinth/Flame/FTB/ATLauncher/Technic/Beteliney), lo cual es varias veces más superficie de cambio y de testeo que este botón manual. Se implementó primero la pieza manual porque es autocontenida, de bajo riesgo, y da valor inmediato por sí sola; el hook automático queda como siguiente paso explícito, no asumido como ya resuelto.
- **Build:** compilación incremental (`ninja -C build -j$(nproc)`) — exit code 0, sin errores ni warnings pese a `-Werror` activo. Primer intento había fallado por un include faltante (`BetelineyZip.h` no lo trae `ExportToZipTask.h` automáticamente) — corregido agregando el include, no fue necesario tocar lógica.
- **`ctest --output-on-failure`: 29/29 tests pasando**, 2.77s total. Ningún test existente cubre específicamente esta feature (no hay test dedicado a la acción de Backup), así que confirma que no rompió nada existente, no que la feature en sí sea correcta — la verificación de la feature en sí queda pendiente de una prueba manual real (ver abajo).
- **Commit y push confirmados:** `523027d18` ("feat(worlds): botón de backup manual de mundos"), pusheado a `main` sobre el commit del bump (`7a3dbf14b`).

**Pendiente real, sin completar todavía:**
1. Prueba funcional manual: usar el botón Backup sobre un mundo real dentro de la app corriendo, y confirmar que el zip se genera en `backups/worlds/`, se puede reabrir, y contiene los archivos correctos. No se hizo en esta sesión (requeriría la app corriendo con una instancia real con mundos).
2. Seguir con el resto de Fase 1: badge de updates de mods (gap real confirmado en sesión 26, `setUpdateAvailable()` sin ningún caller en todo el árbol) y, si el usuario consigue una API key de `abuse.ch`, retomar el sembrado de `known-hashes.json`.


### Sesión 28 — Badge de actualización de mods: cerrado de punta a punta (2026-07-07)

**Contexto:** continuación directa de la sesión 27. En la sesión anterior se había escrito e integrado el código del chequeo silencioso de actualizaciones de mods (`BackgroundModUpdateCheckTask` + hook en `MainWindow::instanceChanged()`), pero el servidor de Desktop Commander se colgó justo antes de poder compilar y testear — quedó sin confirmar. Esta sesión retomó desde ese punto exacto.

**Verificación del estado real antes de tocar nada:** `git status` confirmó que el diff de la sesión 27 seguía intacto en el árbol de trabajo, sin commitear (`MainWindow.cpp/.h`, `CMakeLists.txt` modificados; `BackgroundModUpdateCheckTask.h/.cpp` sin trackear). Se releyó el diff completo y el contenido de los dos archivos nuevos línea por línea antes de compilar, para confirmar que lo aplicado coincidía con lo documentado (sí coincidía).

**Build:** `cmake -S . -B build` reconfiguró sin errores (confirmó que `BackgroundModUpdateCheckTask.cpp/.h` quedaron agregados a `MINECRAFT_SOURCES` en `CMakeLists.txt`). Al correr `ninja -C build -j$(nproc)` el resultado fue sospechosamente rápido (1 solo paso, "Copiando JARs") — se verificó con `stat` que no era un falso positivo: el binario `build/beteliney` (18:58) es más nuevo que ambos archivos fuente modificados (18:41 y 18:28), y existen los `.o` compilados de `BackgroundModUpdateCheckTask.cpp` y `MainWindow.cpp` con moc generado. Conclusión real: el build había quedado corriendo en background en la sesión anterior pese a que la herramienta de terminal se colgó y no pudo confirmar el resultado en su momento — terminó solo, exit 0, sin errores ni warnings pese a `-Werror` activo en ambos targets. Este `ninja` de esta sesión fue un no-op porque ya estaba todo compilado.

**`ctest --output-on-failure`: 29/29 tests pasando**, 2.88s total. Ningún test nuevo dedicado a esta feature (igual que con el backup de mundos de sesión 27) — confirma que no rompió nada existente, la validación funcional de la feature en sí queda para prueba manual con la app corriendo (ver pendiente abajo).

**Commit y push confirmados:** `5c7eaa702` ("feat(mods): chequeo silencioso de actualizaciones en background al seleccionar instancia"), sobre `f7aaad0c5`, pusheado a `main`. Incluye los 5 archivos: `CMakeLists.txt`, `MainWindow.cpp`, `MainWindow.h`, `BackgroundModUpdateCheckTask.cpp`, `BackgroundModUpdateCheckTask.h` — 220 líneas insertadas.

**Qué hace la feature, en concreto:** al seleccionar una instancia (`instanceChanged()`), si es una `MinecraftInstance` y no se chequeó ya en esta sesión del launcher, se lanza `BackgroundModUpdateCheckTask` en background: escanea los mods que ya tienen metadata (nunca genera metadata nueva — eso requiere elegir provider a mano, terreno de `ResourceUpdateDialog`), corre `ModrinthCheckUpdate`/`FlameCheckUpdate` sin ningún diálogo visible, y al terminar llama `MinecraftInstance::setUpdateAvailable(bool)`. Esa función ya existía desde antes (confirmado en sesión 26 que no tenía ningún caller en todo el árbol — feature fantasma: la UI en `InstanceDelegate.cpp` ya pintaba el ícono `checkupdate` en la card, y el modelo ya emitía `propertiesChanged` al llamarla, solo faltaba quién la disparara). Timeout de seguridad de 30s por si `updateFinished()` del mod list nunca llega. `m_modUpdateCheckedInstances` evita re-pegarle a las APIs de Modrinth/CurseForge cada vez que el usuario reselecciona la misma instancia dentro de la misma sesión del launcher.

**Pendiente real, sin completar todavía (igual que el backup de mundos de sesión 27):** prueba funcional manual con la app corriendo — seleccionar una instancia con mods desactualizados y confirmar visualmente que el badge `checkupdate` aparece en la card sin haber abierto el diálogo de update manual. No se hizo en esta sesión (requiere GUI interactiva, fuera del alcance de este entorno de herramientas).

**Con esto, Fase 1 del plan (sesión 25) queda:**
- Backup manual de mundos — hecho (sesión 27), falta prueba manual GUI.
- Badge de actualización de mods — hecho (esta sesión), falta prueba manual GUI.
- `known-hashes.json` — sigue bloqueado por falta de API key de `abuse.ch` (sesión 27), sin cambios.

**Siguiente paso real:** Fase 2 del plan (command palette Ctrl+K + servidores favoritos con quick-join), o conseguir la API key de MalwareBazaar para cerrar el punto de `known-hashes.json` que sigue pendiente. Ambas fases 1-completadas-en-código quedan a la espera de que el usuario haga la prueba manual con la app corriendo cuando tenga oportunidad — no es bloqueante para seguir avanzando en código.

### Sesión 29 — Auditoría del código real (no solo "compila"): use-after-free encontrado y corregido en el badge de mods (2026-07-07)

**Contexto:** el usuario pidió una revisión a fondo de todo lo hecho hasta acá, no solo confirmar que compilaba y pasaban los tests. Se releyó el diff completo de la sesión 28 línea por línea contra los archivos reales (no de memoria), y se verificaron los signatures reales de las clases usadas (`ModrinthCheckUpdate`, `FlameCheckUpdate`, `PackProfile::getModLoadersList()`, `BaseInstance::setUpdateAvailable/hasUpdateAvailable`, `InstanceDelegate.cpp`) para confirmar que el tipado coincidía por diseño y no por conversión implícita casual.

**Hallazgo real — use-after-free potencial en `BackgroundModUpdateCheckTask`:** `m_instance` era `MinecraftInstance*` (puntero crudo). Verificado en `InstanceList.cpp`: las instancias viven en `std::vector<std::unique_ptr<BaseInstance>>`, sin refconteo compartido — `MainWindow::m_selectedInstance` es solo una vista raw sobre ese vector. Si el usuario borra la instancia seleccionada mientras el chequeo de mods en background sigue corriendo (la parte de red — `ModrinthCheckUpdate`/`FlameCheckUpdate` vía `ConcurrentTask` — puede tardar varios segundos reales), el callback final:

```cpp
connect(check_task.get(), &Task::finished, this, [this, check_task, found_update, instance]() {
    if (instance)
        instance->setUpdateAvailable(*found_update);
```

quedaba escribiendo sobre un puntero colgante. El `if (instance)` no protegía nada porque es una copia de un puntero crudo, no se vuelve `nullptr` solo porque el objeto original murió. Es un crash real y alcanzable en uso normal (seleccionar instancia → dispara el chequeo automático → borrar esa misma instancia dentro de la ventana de tiempo que tarda el chequeo de red), no un edge case teórico.

**Por qué la otra conexión (`mod_list->updateFinished`) sí estaba a salvo:** su `connect()` usa `this` (la propia Task) como contexto — Qt desconecta automáticamente esa conexión si el emisor (`mod_list`, que vive dentro de la instancia) es destruido, así que esa mitad del flujo nunca llegaba a ejecutarse sobre memoria inválida. El problema estaba específicamente en el segundo tramo (después de que arranca el chequeo de red), cuyo contexto de conexión es la propia Task, no la instancia — ahí Qt no tenía forma de saber que debía invalidar el puntero capturado.

**Fix aplicado:** `MinecraftInstance* m_instance` → `QPointer<MinecraftInstance> m_instance`. `QPointer` se pone en `nullptr` automáticamente en el momento exacto en que Qt destruye el objeto trackeado, sin importar cuántas copias del `QPointer` existan (incluida la copia capturada por valor en la lambda del callback final) — resuelve la carrera de forma correcta, no solo cosmética. Se agregó además un guard defensivo al inicio de `onModListReady()` por si el objeto muriera justo antes de esa llamada puntual.

**Verificación del fix:** build incremental (`ninja -C build -j$(nproc)`) — terminó solo en background (mismo patrón que sesión 27/28: la herramienta de terminal se colgó esperando el resultado, pero el proceso siguió corriendo del lado del servidor; confirmado con `stat` que `build/beteliney` (22:21) es más nuevo que ambos archivos editados (22:05), sin necesidad de re-lanzar el build). `-Werror` activo, sin warnings. `ctest --output-on-failure`: **29/29 tests pasando**, 2.85s.

**Commit y push confirmados:** `6b2395ee6` ("fix(mods): usar QPointer en BackgroundModUpdateCheckTask para evitar use-after-free"), sobre `1eabc46b6`, pusheado a `main`. 2 archivos, 14 inserciones, 1 eliminación.

**Nota operativa para sesiones futuras:** el servidor de Desktop Commander se sigue colgando de forma consistente en builds largos con LTO (ya van 3 sesiones seguidas con el mismo síntoma — 20, 27, 29). El proceso en sí termina bien del lado del servidor; el problema es solo que la herramienta de terminal no devuelve el resultado a tiempo. Patrón de recuperación que funciona: verificar con `stat` el timestamp del binario contra los archivos fuente modificados, en vez de asumir que el build falló o quedó a medias.

**Resto del estado sin cambios respecto a sesión 28** (ver bloque consolidado arriba): Fase 1 completa en código (backup de mundos + badge de mods), pendiente de prueba manual GUI para ambas; `known-hashes.json` bloqueado por API key de `abuse.ch`; Fase 2 en adelante sin empezar.

### Sesión 30 — Threat model: cómo alguien malicioso podría abusar el código actual (2026-07-07)

**Contexto:** el usuario pidió pensar en todos los ángulos posibles de abuso malicioso del código existente, no un pentest formal. Se verificaron los puntos de mayor impacto contra el código real (no especulación) antes de reportarlos; el resto queda marcado explícitamente como inferencia lógica sin verificar línea por línea, para no mezclar hallazgo confirmado con hipótesis.

**CRÍTICO — confirmado leyendo el código:**

1. **El auto-updater (`BetelineyUpdater.cpp`) no verifica firma ni hash de nada.** Descarga el asset seleccionado de GitHub Releases (`downloadAsset()`) y lo ejecuta directo con `proc.startDetached()`, o lo desempaca y se auto-reemplaza sobre la instalación corriendo (`unpackAndInstall()` → `moveAndFinishUpdate()`). Ninguna verificación criptográfica propia por encima de HTTPS. Quien tenga acceso a publicar/editar un GitHub Release en el repo (cuenta comprometida, CI comprometido, o colaborador malicioso) puede entregar un binario que el updater ejecuta con privilegios de usuario en la máquina de cualquiera con auto-update activo — RCE total, sin necesitar tocar ni una línea del código del launcher. **Es el hallazgo de mayor impacto de toda la revisión.** Heredado de Prism (mismo diseño ahí), pero el riesgo es igual de real acá.

2. **El malware scanner hoy no protege nada en la práctica**, aunque el mensaje de UI sugiera lo contrario. Verificado en `ResourceDownloadTask::downloadSucceeded()`: la comparación contra la blacklist usa `m_pack_version.hash` (el hash declarado por la fuente — Modrinth/CurseForge/BetelineyPack), lo cual está bien diseñado porque `Net::ChecksumValidator` ya garantiza que el archivo descargado coincide con ese hash antes de llegar a este punto (no hay spoofing posible ahí). El problema real: `known-hashes.json` está vacío (bloqueado por API key de MalwareBazaar, ya documentado en sesiones previas) — así que `isMaliciousSha512/Sha256` nunca va a encontrar coincidencia real hoy. El mensaje "⚠ ALERTA DE SEGURIDAD" transmite una protección que actualmente no existe.

3. **La blacklist por hash exacto, una vez poblada, se evade con recompilar/repackear el mod malicioso** (un byte distinto = hash completamente distinto = cero coincidencia). No es un bug de implementación, es la limitación estructural de cualquier detección por hash estático en vez de por comportamiento — pero es exactamente lo primero que alguien que conozca el mecanismo explotaría.

4. **Si un mod/pack no trae hash declarado, se saltan a la vez la verificación de integridad de descarga Y la blacklist de malware.** Mismo `if (!m_pack_version.hash.isEmpty())` gatea ambas protecciones en `ResourceDownloadTask.cpp`. Un BetelineyPack malicioso o comprometido que omita el hash pasa sin ningún control.

**ALTO IMPACTO — no verificado línea por línea esta sesión, inferencia lógica razonable, pendiente de confirmar si se quiere profundizar:**

5. El meta server propio (`ElPibeCapo/meta`, GitHub Pages) es la fuente de verdad de presets/packs. Comprometerlo permite publicar un preset con un mod real (hash real, pasa todos los controles de integridad) pero elegido a propósito para ser dañino — el problema no sería la integridad del archivo sino que el índice mismo mentiría sobre qué instalar.
6. JVM args provenientes de un pack/instancia importada (BetelineyPack, importador GDLauncher, compartir instancia) — si se aplican sin mostrarle al usuario qué argumentos trae antes de instalar, es ejecución de código arbitraria disfrazada de "solo tunear memoria".
7. El importador de GDLauncher lee una base SQLite ajena — si construye rutas de archivo a partir de campos de esa base sin sanitizar, hay riesgo de path traversal (`../../../`) durante la migración. No verificado en código esta sesión.

**RIESGOS DE SUPERFICIE (ya documentados antes, reforzados acá con la lógica de "cómo se explotarían"):**

8. API key vieja de CurseForge sigue en el historial de git (sesión anterior ya lo marcó pendiente) — bots escanean GitHub 24/7 buscando exactamente esto; si la clave se reusó en algún otro lado, sigue siendo explotable ahí aunque esté rotada acá.
9. Sin firma de código en Windows (SmartScreen bypass ya documentado) — el riesgo real no es solo el bypass, es que enseña al usuario a ignorar la advertencia de Windows, lo que también protege a un clon malicioso del launcher distribuido con nombre parecido.
10. Tokens de auth de Microsoft — si se guardan en texto plano en INI/settings en vez del keychain del SO, cualquier otro malware ya presente en la máquina puede robar la sesión de Minecraft directamente. Heredado de Prism/MultiMC, no específico de este fork.
11. `AnonymizeLog` depende de regex — cualquier formato de token futuro que el regex no contemple se filtra tal cual si el usuario comparte un log para pedir ayuda.

**Mitigación de mayor apalancamiento identificada:** firmar los releases (firma detached, verificada por el updater antes de ejecutar/desempacar) es la única acción de esta lista que cambia la *categoría* del riesgo #1 en vez de solo mitigarlo parcialmente — pasa de "quien comprometa mi cuenta de GitHub tiene RCE en todos los usuarios" a "necesita además mi clave de firma privada, que no vive en GitHub". Recomendado como prioridad #1 de seguridad si se decide actuar sobre esta lista.

**Nada de esto se corrigió esta sesión** — es threat model puro, a pedido explícito del usuario, no una sesión de fixes. Ningún commit de código en esta sesión, solo esta documentación.

### Sesión 31 — Fase 2 completa: command palette (Ctrl+K) + servidores favoritos con quick-join (2026-07-08)

**Contexto:** sesión anterior había dejado el código de Fase 2 escrito en el filesystem (6 archivos, 4 modificados) pero sin terminar de auditar ni documentar — el estado real en disco iba más adelantado que ESTADO.md. Esta sesión fue de revisión línea por línea contra el código real (no releer la transcripción de la sesión pasada como si fuera la fuente de verdad), completar lo que faltaba, y dejar todo compilado, testeado y commiteado.

**Qué había en disco al empezar (verificado con `git status`/`git diff`, no asumido):**
- `launcher/FavoriteServers.h/.cpp` (nuevo) — `struct FavoriteServer{name,address}` + `load()/save()` sobre `SettingsObject`, JSON compacto, tolerante a entradas corruptas (una entrada rota no tira toda la lista).
- `launcher/ui/dialogs/CommandPaletteDialog.h/.cpp` (nuevo) — diálogo Ctrl+K, recorre `QMenuBar` recursivamente + acciones extra que le pase el caller, filtra en vivo, navega con flechas, Enter confirma.
- `launcher/ui/dialogs/FavoriteServersDialog.h/.cpp` (nuevo) — gestión completa: agregar/editar/eliminar, cada cambio se persiste al toque (sin estado "sin guardar" que perder).
- `launcher/Application.cpp` — registro del setting `FavoriteServers` (default `"[]"`).
- `launcher/CMakeLists.txt` — los 6 archivos nuevos agregados a `LAUNCHER_SOURCES`.
- `launcher/ui/MainWindow.h/.cpp` — shortcut `Ctrl+K` → `openCommandPalette()`, menú "Servidores favoritos" insertado antes de "Ayuda" (repoblado on-demand via `aboutToShow`), `quickJoinFavoriteServer()`, `openManageFavoriteServers()`.

**Auditoria de esta sesion sobre lo que habia en disco (verificado linea por linea contra el codigo real, no releido de memoria ni de la transcripcion pasada):**

- FavoriteServers.h/.cpp: correcto. load() tolera JSON corrupto/vacio (devuelve lista vacia, nunca crashea), descarta entradas sin address sin tirar el resto de la lista, usa name = address como fallback si falta el nombre. save() serializa a JSON compacto sobre SettingsObject. Sin problemas.
- CommandPaletteDialog.h/.cpp: correcto. Recorre QMenuBar recursivamente incluyendo submenus, filtra separadores y acciones deshabilitadas/invisibles, filtra en vivo por texto (sin mnemonics), navega con flechas y confirma con Enter/doble-click. Diseno explicitamente pensado contra use-after-free: el dialogo nunca dispara la accion elegida internamente, solo guarda el puntero en selectedAction() para que el caller la dispare despues de que exec() retorne y el dialogo ya este cerrado (documentado en el propio header). Es la leccion directa de sesion 29 aplicada de entrada al escribir codigo nuevo, no aplicada despues de encontrar un bug.
- FavoriteServersDialog.h/.cpp: correcto. Agregar/editar/eliminar con promptServer() (mini-dialogo con validacion: no deja guardar direccion vacia). Cada cambio persiste al toque via persist() - no hay estado sin guardar que se pueda perder al cerrar con la X.
- Application.cpp: correcto. Registra FavoriteServers con default "[]", mismo patron que el resto de registerSetting() en ese bloque.
- CMakeLists.txt: correcto. Los 6 archivos nuevos agregados a LAUNCHER_SOURCES en la seccion de migration/, antes de MSALoginDialog.
- MainWindow.h/.cpp: correcto, verificado contra los signatures reales (no asumidos):
  - quickJoinFavoriteServer() usa std::make_shared<MinecraftTarget>(MinecraftTarget::parse(address, false)) + APPLICATION->launch(m_selectedInstance, LaunchMode::Normal, target) - es el mismo patron exacto, caracter por caracter, que ya esta en produccion en ui/pages/instance/ServersPage.cpp:761 (quick-join desde la lista de servidores de una instancia). No es una construccion nueva sin precedente, es reutilizar el flujo ya probado.
  - Application::launch() verificado en Application.h:220 - firma (BaseInstance*, LaunchMode, std::shared_ptr<MinecraftTarget>, ...) coincide exactamente.
  - MinecraftTarget::parse(fullAddress, useWorld) verificado en MinecraftTarget.h:28 - segundo parametro false es correcto (es una direccion de servidor, no una ruta de mundo).
  - m_selectedInstance verificado en MainWindow.h:269 - es BaseInstance*, coincide con el primer parametro de launch(). quickJoinFavoriteServer() chequea if (!m_selectedInstance) antes de usarlo (muestra CustomMessageBox de advertencia en vez de crashear).
  - openCommandPalette(): las acciones sinteticas de quick-join (ownedActions) se crean con this como padre Qt, se le pasan al dialogo solo para listar/filtrar (el dialogo no toma ownership, ver comentario en CommandPaletteDialog.h), se disparan despues de exec() si fueron la elegida, y se destruyen con qDeleteAll() al final - sin fugas ni doble-free, sin importar si la accion elegida fue una sintetica o una real del menu.
  - populateFavoriteServersMenu(): se repuebla en cada aboutToShow via m_favoriteServersMenu->clear() - verificado que QMenu::clear() borra las QAction de las que el menu es dueno (todas aca, creadas con addAction()), asi que no hay fuga de memoria ni acciones fantasma acumulandose sesion tras sesion de abrir/cerrar el menu.
  - Insercion del menu (ui->menuBar->insertMenu(ui->helpMenu->menuAction(), ...)) es incondicional (no depende del toggle MenuBarInsteadOfToolBar), correcto porque la barra de menu existe siempre como objeto aunque esa opcion solo controle si se ve o no.

**Ningun bug encontrado.** A diferencia de sesion 29 (donde la auditoria encontro un use-after-free real), esta revision no encontro defectos - el codigo de Fase 2 esta bien disenado desde el vamos, siguiendo tanto los patrones ya probados del codebase (ServersPage.cpp) como las lecciones de la sesion anterior.

**Build y tests verificados por esta sesion (no asumidos de la transcripcion anterior, que se corto antes de confirmarlo):**
- Timestamps: build/beteliney (14:20) mas nuevo que los 4 archivos fuente modificados mas recientemente (FavoriteServers.cpp, CommandPaletteDialog.cpp, FavoriteServersDialog.cpp, todos a las 13:34) y que MainWindow.cpp (12:31) - el binario esta al dia con el codigo en disco.
- ctest --output-on-failure corrido directamente esta sesion: **29/29 tests pasando**, 2.86s. Sin fallos.
- clang-format --dry-run --Werror sobre los 6 archivos nuevos: **limpio, cero violaciones**. Sobre los archivos modificados (Application.cpp, MainWindow.cpp/.h) hay violaciones de formato, pero estan repartidas por todo el archivo (incluido codigo preexistente sin relacion con este diff) - es un desfasaje preexistente entre la version de clang-format instalada aca y estos archivos legados, no algo introducido por Fase 2. No se toco nada de eso: esta fuera de alcance.

**Estado real de git al cierre de esta seccion de la sesion:** nada de esto esta commiteado todavia. git status muestra 6 archivos nuevos sin trackear + Application.cpp/CMakeLists.txt/MainWindow.cpp/MainWindow.h/ESTADO.md modificados sin stagear. Ultimo commit en el arbol es ffb219f69 (threat model de sesion 30).

### Sesion 32 - Firma criptografica Ed25519 del updater + correccion de metodologia de verificacion (2026-07-08)

**Contexto de arranque:** la sesion anterior se corto (MCP colgado) en medio del build de verificacion, dejando el codigo escrito en disco pero sin confirmar compilacion, sin tests, sin commit. Esta sesion arranco reconciliando el estado real del repo (git status/log/diff contra el arbol, no contra la transcripcion) antes de tocar nada.

**Hallazgo #1 - por que el build anterior se colgo sin verificar nada util:** `ninja -C build -j$(nproc)` sin argumentos de target compila el launcher completo, que tiene LTO activado (`IPO / LTO enabled`, confirmado en el log de cmake) y tarda minutos - el mismo patron de cuelgue ya documentado en sesiones 20/27/29. Pero ademas, y esto es lo que importa: **ese build ni siquiera iba a tocar el codigo del updater**. `Launcher_BUILD_UPDATER` (CMakeLists.txt:404) solo se activa si `Launcher_BUILD_ARTIFACT` (CMakeLists.txt:209) no esta vacio, y por defecto esta vacio en un build local. Confirmado con `ninja -C build -t targets | grep updater` antes de reconfigurar: cero resultados, el target `prism_updater_logic` no existia en el grafo de build. La sesion anterior podria haber esperado horas al build completo sin haber compilado ni una linea de `UpdateVerify.cpp`.

**Fix de metodologia:** reconfigure con `cmake -S . -B build -DLauncher_BUILD_ARTIFACT="linux-x86_64"` (reconfiguracion incremental, no reset del build existente), lo que habilito los targets `prism_updater_logic` y `BetelineyLauncher_updater`. Confirmado en el log de cmake: `Enabling all warnings as errors for target 'prism_updater_logic'` y `'BetelineyLauncher_updater'` - mismo nivel de rigor (`-Werror`) que el resto del proyecto.

**Build real, en background para no repetir el cuelgue de la herramienta:** `ninja -C build -j8 prism_updater_logic BetelineyLauncher_updater` lanzado con `nohup ... & disown`, log a archivo, sondeado con `cat`/`pgrep` sin bloquear la conexion del tool. **40/40, sin un solo warning ni error** (grep sobre el log completo: cero coincidencias de "error"/"warning"). `UpdateVerify.cpp.o` compilo en el paso 8/40. Link final: `build/beteliney_updater` generado y confirmado con `find`.

**Lo que esto confirma de lo que dejo la sesion anterior (auditado, no asumido):**
- La logica de verificacion Ed25519 fail-closed en `UpdateVerify.cpp/h` (32 bytes de clave publica embebida, borra el archivo descargado y aborta si falta `.sig` o no valida) compila limpio contra libsodium.
- El wiring en `launcher/CMakeLists.txt` (PRISMUPDATER_SOURCES + link condicional `PkgConfig::libsodium` o fallback `find_library`) esta correcto - se verifico linea por linea con `git diff`, no solo confiando en que "deberia estar bien".
- El `find_package`/`pkg_check_modules` de libsodium en el CMakeLists.txt raiz resuelve bien (`Checking for module 'libsodium' -- Found libsodium, version 1.0.22`).
- `vcpkg.json` y `.github/workflows/build.yml` (dependencias apt/msys2, paso de firma Ed25519 del release con secret `RELEASE_SIGNING_KEY`) quedan sin verificar en esta sesion - no hay forma de correr el workflow de GitHub Actions localmente; la revision fue solo de sintaxis/logica leyendo el YAML.

**No verificado en esta sesion (pendiente real):**
- No se corrio `ctest` sobre el nuevo codigo - no existen tests unitarios para el updater en el proyecto (los 29/29 tests de sesion 31 son del launcher principal, target distinto).
- `clang-format --dry-run` sobre `UpdateVerify.h/.cpp` no se pudo correr: el archivo `.clang-format` no existe en la raiz del repo en este checkout (buscado con `find`, no aparece), pese a que el custom target de clang-format en `build.ninja` lo referencia por ruta absoluta. Es una condicion preexistente del entorno, no algo introducido por este cambio - queda anotado pero fuera de alcance arreglarlo ahora.
- El paso de firma real en CI (`RELEASE_SIGNING_KEY` -> firmar assets del release) sigue sin probarse end-to-end porque requiere que subas el secret a GitHub y se dispare un release real.

**Clave privada:** sigue en `/tmp/beteliney_signing/release_signing_key.pem` (permisos 600, solo pibe), sin commitear, confirmado que sigue ahi. **Segue pendiente que la subas vos a Settings -> Secrets and variables -> Actions -> New repository secret con el nombre exacto `RELEASE_SIGNING_KEY`, pegando el PEM completo, y borres el archivo de /tmp despues.**

**Pregunta sin resolver de sesiones anteriores, sigue sin tocarse:** reescribir el historial de git para purgar la API key vieja de CurseForge - irreversible, rompe forks/clones existentes. No se hizo, esperando tu confirmacion explicita.

**Estado de git al cierre de esta seccion:** `git add` + `commit` hecho sobre los 8 archivos (6 modificados + 2 nuevos) con el fix de ESTADO.md incluido. Push a origin/main pendiente de confirmar en el mensaje de cierre de sesion (ver abajo si ya se hizo).

### Continuacion sesion 32 - fixes reales aplicados sobre el threat model, antes de cierre por limite de tokens

**Contexto:** el usuario pidio avanzar todo lo posible sobre la lista de pendientes antes de que la sesion se corte. De los items marcados "sin verificar linea por linea" en el threat model de sesion 30, se resolvieron dos de forma concreta:

**1. Path traversal en el importador de GDLauncher - CONFIRMADO y ARREGLADO.**

`GDLauncherMigrator.cpp` lee `shortpath`/`id` desde `data.sqlite` de GDLauncher (un archivo que el usuario puede recibir de un tercero, o de una instalacion de GDLauncher comprometida) y los concatenaba sin sanitizar para construir `sourcePath = dataDir + "/instances/" + shortpath`. Un `shortpath` como `"../../../../home/usuario/.ssh"` hacia que `copyDirRecursive()` copiara esos archivos hacia el `.minecraft/` de la instancia recien creada - lectura arbitraria de archivos del sistema, disfrazada de "importar una instancia".

Ademas, `inst.name` (tambien de la DB) se usaba para el nombre del directorio destino sanitizando solo `\/:*?"<>|` - un nombre literal `".."` no tiene ninguno de esos caracteres, y `destInstancesDir + "/" + ".."` resuelve al directorio padre: escritura de `instance.cfg`/`mmc-pack.json` fuera de la carpeta de instancias (blast radius menor que el de lectura, porque no hay forma de encadenar mas niveles sin `/`, pero real).

**Fix aplicado:** funcion `safeChildPath()` que usa `QDir::cleanPath()` para resolver logicamente los `../` y verificar que el resultado siga dentro del directorio base; si no, devuelve vacio (la instancia cae al camino ya existente de "no se encontraron los archivos", sin crashear). Para el nombre del directorio destino: rechazo explicito de nombres compuestos solo por puntos (`^\.+$`), fallback a `GDL_<id>` igual que el caso de nombre vacio que ya existia.

**Verificado que compila:** extraido el comando exacto de `build/compile_commands.json` para `GDLauncherMigrator.cpp` y corrido standalone (sin disparar el link/LTO del launcher completo) - **compila limpio con `-Werror`**, objeto generado.

**No se corrio el build completo del launcher ni ctest sobre este cambio** - se prioriza dejar esto documentado con precision sobre simular una verificacion mas completa de la que hubo tiempo de hacer. Pendiente para la proxima sesion: build completo + `ctest` antes de dar esto por definitivamente cerrado (aunque el archivo compila aislado, un cambio en `migration/` podria interactuar con algo que solo aparece en el link completo - improbable dado que no toca ninguna interfaz externa, pero no confirmado).

**2. JVM args de packs importados (FTB legacy) - VERIFICADO, sin fix separado (decision razonada).**

Confirmado en `PackInstallTask.cpp:61-63`: el `JvmArgs` del manifiesto del pack importado se aplica tal cual via `OverrideJavaArgs`, sin sanitizar. Pero se concluye que esto no es una escalada de privilegios *adicional* real: un modpack ya ejecuta codigo arbitrario en cuanto se lanza (los mods son codigo Java arbitrario) - inyectar `-javaagent` u otro flag no le da a un pack malicioso mas poder del que ya tiene. Es el mismo modelo de confianza que existe en cualquier launcher de modpacks (este proyecto, el original del que viene, CurseForge, etc.) - no un bug introducido aca. No se aplico ningun fix para no crear una falsa sensacion de seguridad arreglando el sintoma sin tocar el problema real (confiar en packs de fuentes no verificadas).

**3. `RELEASE_SIGNING_KEY` subido a GitHub Actions - HECHO.**

`gh` estaba autenticado con scope `repo` (`gh auth status` confirmo cuenta `ElPibeCapo`, token con scopes `gist, read:org, repo, workflow`). Se subio el secret directamente con `gh secret set RELEASE_SIGNING_KEY --repo ElPibeCapo/BetelineyLauncher < /tmp/beteliney_signing/release_signing_key.pem`, confirmado con `gh secret list` (aparece con fecha de esta sesion). **La clave privada temporal en `/tmp/beteliney_signing/release_signing_key.pem` fue borrada de forma segura (`shred -u`) despues de subirla.** Solo queda la clave publica en `/tmp/beteliney_signing/release_signing_pub.pem` (sin riesgo, es publica por diseno).

**4. `.clang-format` faltante - investigado, no es un bug.**

`git log --all --diff-filter=D -- .clang-format` muestra que se borro a proposito en un commit del proyecto (`ffe84d6ec "remove some dead things"`). No se restauro - fue una decision deliberada previa, no algo perdido por error. Queda como esta salvo que se pida explicitamente restaurarlo.

**Estado de git al cierre de esta seccion:** el fix de `GDLauncherMigrator.cpp` + esta documentacion quedan commiteados y pusheados junto con esta seccion (ver hash de commit en el mensaje de cierre de sesion si ya se hizo el push).

**Pendiente real que queda, actualizado:**
1. Meta server (`ElPibeCapo/meta`) como fuente de verdad - **sigue sin verificar linea por linea**, es mas arquitectural (requeriria firmar el indice del meta server tambien, cambio grande) - no se ataco esta sesion por alcance/tiempo.
2. `known-hashes.json` bloqueado por API key de abuse.ch/MalwareBazaar - sin cambios, requiere que consigas la key vos.
3. Purga del historial de git de la API key vieja de CurseForge - **sigue esperando tu confirmacion explicita**, irreversible.
4. Pruebas manuales GUI (backup de mundos, badge de mods) - sin cambios, no automatizables desde este entorno.
5. Build completo + ctest sobre el fix de GDLauncherMigrator - pendiente para proxima sesion (se verifico compilacion aislada, no el link completo ni tests).
6. Paso de firma real en CI nunca probado end-to-end - ahora que el secret esta subido, falta que se dispare un release real para confirmar que firma bien.

### Sesion 33 - Verificacion externa contra GitHub real (no contra lo que dice este documento) (2026-07-08)

**Contexto:** el usuario pidio revisar todo de nuevo, a fondo. En vez de releer ESTADO.md y confiar en el, esta sesion verifico cada afirmacion pendiente contra el estado real del repo (`git`) y de GitHub (`gh`), incluyendo un intento de build completo local que revelo un problema nuevo.

**1. Intento de build completo local con LTO - CONFIRMADO que se cuelga, causa aislada de si el fix es correcto.**

Se lanzo `cmake --build . -j$(nproc)` (build completo, sin restringir targets) en background sobre el commit `350227d48`. Avanzo limpio hasta compilar y linkear `Launcher_logic` (la libreria estatica que contiene el fix de `GDLauncherMigrator.cpp`) y linkear `beteliney_updater`, `GradleSpecifier`, `GZip` (10/38 targets) - **el fix compila y linkea limpio dentro de la libreria principal**, confirmado mas alla de la compilacion aislada de sesion 32. Despues de eso el build se colgo: proceso `ninja` vivo pero 0% CPU, log sin crecer por mas de 2 minutos. Se mato el proceso (`SIGKILL`) para no dejarlo zombie. Es el mismo patron de cuelgue con LTO ya documentado en sesiones 20/27/29/31/32 - no revela nada nuevo sobre el fix en si, solo reconfirma que el build completo local con LTO no es confiable en este entorno para verificacion. Causa raiz del cuelgue en si: **no investigada** (sigue pendiente si se quiere resolver la herramienta de verificacion local, no bloqueante para el proyecto).

**2. CI de GitHub Actions - verificado con `gh run list`, no asumido: el build completo SI paso, en un entorno limpio.**

El commit `350227d48` (fix de path traversal) corrio en CI y termino `completed success` en 14m11s. El commit `36af71d2c` (firma Ed25519) tambien `completed success`. Esto es evidencia mas fuerte que el intento local fallido: confirma que el fix de GDLauncher compila y linkea el launcher completo de punta a punta en un entorno limpio (Ubuntu 24.04 CI), aunque localmente el build se cuelgue por un problema de entorno no relacionado con el codigo.

**Matiz importante encontrado sobre este punto:** el workflow de CI (`.github/workflows/build.yml`, lineas 80 y 164) tiene `-DBUILD_TESTING=OFF` hardcodeado. **CI nunca corre `ctest`, ni antes ni ahora.** Esto significa que "build completo + ctest" como pendiente (item 5 de la lista de sesion 32) va a seguir sin poder cerrarse via CI para siempre - la unica forma de correr ctest es localmente, que es justo donde el build se cuelga con LTO. Sigue pendiente encontrar una forma de compilar+testear localmente sin disparar el cuelgue (ej: targets restringidos como se hizo en sesion 32 para el updater, aplicado ahora al launcher principal + tests).

**3. `RELEASE_SIGNING_KEY` en GitHub Actions - CONFIRMADO con `gh secret list` (no solo confiando en el commit anterior).**

Aparece en la lista de secrets del repo, fecha `2026-07-09T01:02:19Z`, coincide con lo que dice sesion 32. Confirmado tambien que `CURSEFORGE_API_KEY` (el secret actual, rotado) sigue presente desde `2026-07-04T16:33:10Z` - no se toco, sigue activo para CI.

**4. `known-hashes.json` - CORRECCION DE ESTA MISMA SESION (era un error propio, corregido en sesion 34): SI EXISTE, esta en el repo equivocado de busqueda.**

`find . -name "known-hashes.json"` no devolvio resultado porque se corrio dentro del checkout de `BetelineyLauncher` (el launcher). El archivo nunca vivio ahi — vive en `~/Descargas/meta_beteliney`, el clon local del repo `meta` (rama `gh-pages`), tal como ya documentaba correctamente la sesion 27. Verificado en sesion 34 leyendo el archivo real: existe, arrays `sha256`/`sha512` vacios por diseno honesto (sesion 14), `comment` con la investigacion completa, MD5 de Bitdefender documentados aparte sin usar por el scanner, y `sourcesChecked` con las 5 fuentes revisadas. No cambia la conclusion de fondo (el scanner sigue sin proteger nada en la practica porque los arrays reales estan vacios), pero la sesion 33 se equivoco al decir que el archivo no existia — solo busco en el repo que no correspondia.

**5. `.clang-format` - reconfirmado ausente.** `ls .clang-format` en la raiz: no existe. Sigue siendo la misma condicion preexistente documentada en sesion 32 (borrado a proposito en `ffe84d6ec`), sin cambios.

**6. Estado de git al cierre - todo limpio y sincronizado, verificado con comandos directos, no asumido:**
- `git status --porcelain`: sin salida (arbol limpio).
- `git stash list`: vacio.
- `git branch -vv`: `main` apunta a `350227d48`, exactamente igual que `origin/main` (`[origin/main]` sin `ahead`/`behind`).
- `git log -1` local y `git log -1 origin/main`: mismo hash (`350227d48`) en ambos.

**Nada de codigo se toco esta sesion - fue puramente de verificacion/auditoria externa.** Unico cambio: esta seccion de documentacion.

**Pendiente real, actualizado y sin cambios de fondo respecto a sesion 32 (solo mas evidencia, ninguno de estos items se cerro):**
1. Meta server (`ElPibeCapo/meta`) como fuente de verdad - sigue sin verificar linea por linea.
2. `known-hashes.json` - existe en `~/Descargas/meta_beteliney` (repo `meta`, no el launcher), arrays vacios por diseno, sin cambios de fondo desde sesion 27 - bloqueado por API key de abuse.ch/MalwareBazaar, requiere que el usuario la consiga. (Correccion sesion 34: la afirmacion de que "no existe" en esta misma sesion 33 fue un error de busqueda en el repo equivocado.)
3. Purga del historial de git de las 4 API keys viejas de CurseForge (confirmadas por hash en sesion anterior) - **sigue esperando confirmacion explicita del usuario**, irreversible.
4. Pruebas manuales GUI (backup de mundos, badge de mods) - sin cambios, no automatizables desde este entorno.
5. `ctest` local sobre el fix de GDLauncherMigrator - **ahora confirmado que CI nunca lo va a correr** (`BUILD_TESTING=OFF` en el workflow), la unica via es local, y local se cuelga con LTO en un build sin restringir targets. Pendiente: intentar con targets restringidos (patron ya usado en sesion 32 para el updater) para evitar el cuelgue y poder correr ctest.
6. Paso de firma real en CI nunca probado end-to-end - secret confirmado presente, falta que se dispare un release real.
7. **Nuevo:** causa raiz del cuelgue del build completo local con LTO - no investigada, solo reconfirmada su existencia. No bloqueante (CI cubre la verificacion de build), pero afecta la capacidad de correr ctest localmente (ver punto 5).

### Sesión 34 — Corrección: `known-hashes.json` sí existe, sesión 33 buscó en el repo equivocado (2026-07-08)

**Contexto:** el usuario pidió actualizar toda la información del documento. Al revisar el pendiente #2 de sesión 33 antes de tocar nada, se detectó que la afirmación "`known-hashes.json` no existe como archivo en ningún lado del repo" era un error de esa misma sesión, no un hallazgo real.

**Causa del error:** sesión 33 corrió `find . -name "known-hashes.json"` dentro del checkout de `BetelineyLauncher` (el repo del launcher). El archivo nunca vivió ahí. Vive en el repo separado `meta` (rama `gh-pages`), clonado localmente en `~/Descargas/meta_beteliney` — exactamente como ya documentaba correctamente la sesión 27 (`Repo: /home/pibe/Descargas/meta_beteliney (clon local separado del repo principal)`). Sesión 33 tenía esa info disponible en el mismo documento y no la cruzó antes de afirmar que el archivo "no existe".

**Verificación real hecha esta sesión:** se localizó y leyó el archivo completo en `~/Descargas/meta_beteliney/v1/malware/known-hashes.json`. Confirma exactamente lo documentado en sesiones 14 y 26-27, sin cambios de fondo:
- `hashes.sha256` y `hashes.sha512`: arrays vacíos, `"status": "no-public-hash-source-found"`.
- `comment` con la investigación completa (Fractureiser, junio 2023) documentada en el propio JSON.
- `knownSamplesMd5_notUsedByScanner`: los 2 MD5 de Bitdefender (Stage 0 y Stage 2), marcados explícitamente como no usados por el scanner (que solo soporta SHA-256/512).
- `sourcesChecked`: las 5 URLs revisadas en su momento (repo oficial fractureiser, docs/users.md, blog de Bitdefender, anuncio de Prism Launcher, artículo de soporte de CurseForge).
- Sigue bloqueado exactamente por lo mismo desde sesión 27: falta la API key de `abuse.ch`/MalwareBazaar para poder consultar o verificar hashes reales sin fabricar datos.

**Correcciones aplicadas en este documento:** 3 puntos donde sesión 33 afirmaba "no existe" (bloque `ESTADO ACTUAL` arriba, sección de detalle de sesión 33, y su lista de pendientes) corregidos con nota explícita de que fue un error de búsqueda, no un hallazgo nuevo.

**Nada de código se tocó esta sesión** — fue puramente corrección de documentación. El resto del estado (git limpio, CI verde en ambos commits, secret de firma presente, cuelgue del build local con LTO, `.clang-format` ausente, 4 keys viejas de CurseForge en el historial) sigue exactamente igual que al cierre de sesión 33, sin novedad.

**Pendiente real, sin cambios de fondo respecto a sesión 33 (misma lista, punto 2 con la ubicación correcta del archivo):**
1. Meta server (`ElPibeCapo/meta`) como fuente de verdad - sigue sin verificar línea por línea.
2. `known-hashes.json` (en `~/Descargas/meta_beteliney`, repo `meta`) - bloqueado por API key de abuse.ch/MalwareBazaar, requiere que el usuario la consiga.
3. Purga del historial de git de las 4 API keys viejas de CurseForge - **sigue esperando confirmación explícita del usuario**, irreversible.
4. Pruebas manuales GUI (backup de mundos, badge de mods) - sin cambios, no automatizables desde este entorno.
5. `ctest` local sobre el fix de GDLauncherMigrator - bloqueado por el cuelgue del build completo con LTO; camino recomendado: restringir targets como en sesión 32.
6. Paso de firma real en CI nunca probado end-to-end - secret presente, falta que se dispare un release real.
7. Causa raíz del cuelgue del build completo local con LTO - no investigada.

### Sesión 35 — Auditoría de 3 documentos externos (estrategia de IA, plan de UI/web, roadmap técnico) contra el código real (2026-07-09)

**Contexto:** el usuario pasó 3 documentos generados por otra IA, sin relación directa con ninguna sesión previa de este proyecto: `panorama-ia-julio-2026-verificado-v2.md` (comparación de modelos de IA para desarrollo), `Master Plan Beteliney 2026` (landing page con v0, rediseño de UI con Google Stitch/Recraft, "AI Chat Log Assistant" con LLM local) y `BetelineyLauncher: Hoja de Ruta de Excelencia Técnica 2026` (Qt 6.11, GraalVM, ZGC, sandboxing, Wayland nativo). Pidió documentar todo y mejorar todo. Metodología: no asumir ningún dato de los 3 documentos como cierto solo por venir de una IA — verificar cada afirmación relevante contra el código real, contra `README.md` y contra este mismo `ESTADO.md` antes de aceptarla o rechazarla, mismo estándar de rigor que sesiones 26/32/33/34.

**Verificaciones hechas y resultado, punto por punto:**

1. **ZGC como recolector por defecto (propuesto en el Roadmap Técnico como pendiente):** ya implementado desde el commit inicial — es el perfil 6 ("iGPU ZGC Java21+") de `BetelineyProfiles.h`. El documento estaba desactualizado, no hace falta ninguna acción.

2. **"Integración con Wayland Nativo" para el juego (Roadmap Técnico):** verificado contra `README.md`/`lanzar.sh` del propio proyecto — **el documento está directamente equivocado.** LWJGL (la librería que usa Minecraft para OpenGL/input) no soporta Wayland nativo hoy, por eso `lanzar.sh` fuerza `GLFW_PLATFORM=x11` explícitamente para el proceso del juego. Es una limitación upstream de LWJGL, no una decisión pendiente del launcher — el launcher Qt6 en sí ya corre sobre Wayland nativo sin problema. Descartado, con esta nota para no volver a proponerlo hasta que LWJGL lo soporte.

3. **"AI Chat Log Assistant" con LLM local (Llama 3.3 8B / Mistral Small 7B vía llama.cpp), propuesto en el Master Plan:** descartado por conflicto directo con el hardware target del propio proyecto. `BetelineyLogAnalyzer` ya cubre 18 tipos de error de forma determinista, sin costo de RAM, sin red, sin latencia. Correr un LLM local de 7-8B (aun cuantizado, ~4-5GB de RAM) en una máquina de 16GB compartidos con la Vega 10 — la misma RAM que los perfiles JVM del propio launcher ya reservan hasta 12GB en el perfil extremo — compite por el recurso más escaso del sistema para reemplazar algo que ya funciona gratis y sin fallos conocidos. No se agrega al backlog.

4. **Rediseño completo de UI con Google Stitch + Recraft (Master Plan):** descartado como reescritura total. `BetelineyTheme.cpp` ya tiene ~810 líneas de QSS custom con estética neón coherente, verificada con captura real en sesión 19. Recraft queda como herramienta válida solo para assets puntuales nuevos (íconos faltantes), no para regenerar el theme desde cero.

5. **Landing page con v0 by Vercel (Master Plan):** idea válida, sin conflicto técnico con nada del código. Prioridad baja — no reemplaza ni se mete en medio de las Fases 1-5 del backlog ya decidido en sesión 25, queda anotada para después de cerrarlas.

6. **Qt 6.11 "Canvas Painter" (Roadmap Técnico):** no verificable con la información disponible en esta sesión (sin acceso a documentación oficial de Qt para confirmarlo). No se incorpora como hecho ni se actúa sobre esto sin verificarlo directamente contra `doc.qt.io` primero.

7. **GraalVM con "+15-20% de FPS" (Roadmap Técnico):** la cifra no tiene fuente verificable — mismo patrón de dato de vendor sin auditar que el propio `panorama-ia-julio-2026-verificado-v2.md` marca como red flag en otros contextos (ej. los benchmarks de METR sobre GPT-5.6 revisados bajo NDA). La idea en sí (GraalVM como motor de ejecución alternativo a OpenJDK) es técnicamente real y no descabellada, pero queda como investigación futura sin comprometerse con ninguna cifra de mejora hasta medirlo en el hardware real del proyecto.

8. **Sandboxing con Bubblewrap (`bwrap`), propuesto en el Roadmap Técnico:** la única idea genuinamente nueva de los 3 documentos que no estaba ya cubierta ni descartada por lo anterior. `bwrap` ya es parte del ecosistema del sistema (mismo mecanismo de sandboxing que usa Flatpak, que el proyecto ya empaqueta). Aislar el proceso de Minecraft con `bwrap` para que un mod malicioso no pueda leer tokens de sesión de Discord/navegador es una mejora de seguridad real y no trivial, coherente con el threat model completo de sesión 30 (que no la había contemplado). **Se agrega como ítem 11 al backlog de mejoras**, en cola después de la Fase 4 del plan de sesión 25 — no se reordena nada de lo ya decidido, solo se suma al final.

9. **Precios de modelos de IA (`panorama-ia-julio-2026-verificado-v2.md`) vs. tabla 3.1 de `ESTRATEGIA_IA.md` v4.0 (20 jun 2026):** la tabla de precios de `ESTRATEGIA_IA.md` quedó desactualizada frente a los datos de julio (Sonnet 5 con precio introductorio $2/$10 hasta el 31 de agosto en vez de $3/$15 fijo, GLM-5.2 a $1.40/$4.40 en vez de $1.20/$4.10, etc.). Se actualizó `ESTRATEGIA_IA.md` a v5.0 en esta misma sesión — ver ese archivo para el detalle completo, no se duplica la tabla acá para no crear una segunda fuente de verdad sobre lo mismo.

**Qué NO se hizo esta sesión, honestamente:** no se tocó ni una línea de código C++/Qt, no se corrió build ni `ctest` — no había ningún cambio de código que ameritara verificación de compilación. Es sesión de documentación/planificación pura, mismo tipo que sesiones 24, 25 y 30.

**Pendiente real, sin cambios de fondo respecto a sesión 34 (los mismos 7 puntos siguen exactamente igual), más lo nuevo de esta sesión:**
8. Backlog de mejoras: ítem 11 agregado — sandboxing con Bubblewrap para el proceso de Minecraft, esfuerzo estimado medio (requiere mapear qué directorios necesita ver el juego en runtime — assets, saves, mods, Java — sin romper nada), sin empezar.
9. Si en algún momento se quiere retomar la idea de landing page (v0) o investigar GraalVM como motor alternativo, quedan anotadas como opcionales de baja prioridad, no urgentes ni bloqueantes de nada.

### Sesión 36 — Sistema de logros de marca cerrado (Fase 3, ítem 8) + incidente de clang-format corregido en el momento (2026-07-11)

**Contexto:** continuación de una sesión anterior que había dejado 4 archivos nuevos escritos con la herramienta equivocada (sandbox aislado en vez del filesystem real) y luego los había recreado bien, pero corrido `clang-format` sobre los 2 archivos existentes tocados (`MainWindow.cpp`, `BaseInstance.cpp`) antes de comprobar que el `.clang-format` del repo coincidiera con el estilo real del código.

**Incidente encontrado y corregido antes de comitear:** `git diff --stat` mostraba 3261 líneas cambiadas en `MainWindow.cpp` y 632 en `BaseInstance.cpp` para un feature que solo debía tocar ~15 líneas de wiring + 2 includes. Causa: `clang-format` reformateó los archivos completos (indentación 4→2 espacios, `Type* x`→`Type *x`, etc.), no solo las líneas tocadas — el `.clang-format` del repo no coincide con el estilo real usado en el resto del código (confirmado también por `git-clang-format --diff` más tarde: mismo resultado, produce diffs que chocan con el estilo circundante).

**Fix aplicado:** `git checkout --` sobre los 3 archivos trackeados afectados (se perdió el diff exacto de `MainWindow.cpp` al no estar stageado — nunca se había hecho `git add`, así que git no lo tenía guardado en ningún lado) y reaplicación manual de los cambios reales, línea por línea, respetando el estilo de las secciones vecinas (4 espacios, `Tipo* variable`, mismo patrón que el bloque "Beteliney Fase 2" ya existente al lado). Confirmado con `git diff --stat` final: **26 líneas insertadas en total** (`BaseInstance.cpp` +6, `CMakeLists.txt` +6, `MainWindow.cpp` +12, `MainWindow.h` +2) — el tamaño real del cambio.

**Bug menor encontrado y corregido de paso:** `AchievementToast.h`/`.cpp` tenían el header de licencia copiado de un archivo Apache-2.0 en vez de `SPDX-License-Identifier: GPL-3.0-only` (la licencia real del proyecto, confirmada contra archivos hermanos como `LabeledToolButton.cpp`). Corregido antes de comitear — cambio de comentario únicamente, sin impacto funcional.

**Qué hace la feature:** 5 logros de marca por tiempo jugado en una misma instancia (1h/10h/50h/100h/500h — `playtime_1h` … `playtime_500h`), definidos en `Beteliney::Achievements` (singleton, `BetelineyAchievements.h/.cpp`). Se chequean en `BaseInstance::setMinecraftRunning()` justo después de que `totalTimePlayed()` se actualiza al cerrar el juego — sin trackeo nuevo, reusa el campo que ya existía. Persistencia de logros desbloqueados en `SettingsObject` (`BrandAchievementsUnlocked`, `QStringList`). Notificación visual: `AchievementToast` (widget flotante sin marco, esquina inferior derecha de `MainWindow`, 6s por logro, cola FIFO si se desbloquean varios de una — por ejemplo la primera vez que alguien abre una instancia con más de 500h ya jugadas antes de instalar el launcher).

**Verificación de compilación — nota operativa:** el primer intento de build completo (`ninja -C build -j$(nproc)`) con LTO tardó lo suficiente como para que la herramienta de terminal reportara error de timeout dos veces seguidas (mismo síntoma ya documentado en sesiones 20/27/29/31/32/33 — el proceso del lado del servidor sigue corriendo bien, solo la herramienta no espera lo suficiente). Se relanzó en background con `nohup ... & disown` + log a archivo, sondeado sin bloquear. **Build final: 35/35 sin errores ni warnings** (`-Werror` activo). `ctest`: **29/29 tests pasando**, 2.89s.

**Commit:** `b49a3cbc8` ("feat(achievements): sistema de logros de marca por tiempo jugado (Fase 3)"), 8 archivos, 288 inserciones.

**Con esto, Fase 3 del plan de sesión 25 queda con el sistema de logros cerrado.** **Backlog restante:** ítem 11 (sandboxing Bubblewrap, sesión 35) sin empezar; los 7 pendientes reales de sesión 34 sin cambios (2 de ellos bloqueados exclusivamente por acción del usuario: purgar API keys viejas del historial de git, y conseguir API key de abuse.ch).

**Lección operativa reforzada:** antes de correr `clang-format`/`git-clang-format` sobre código existente en este repo, comprobar primero que el resultado no diverja del estilo real circundante (`git diff --stat` chico y localizado = buena señal; si el diff es enorme y toca líneas no relacionadas, parar y revisar antes de comitear, no después).

---

### Sesión 37 — Causa raíz del cuelgue histórico del build local con LTO, resuelta (2026-07-11)

**Contexto:** continuación directa de una sesión anterior cortada por límite de mensajes justo mientras se aplicaba el fix y se lanzaba el rebuild de verificación en background. Al retomar, se confirmó primero que la sesión 36 (logros de marca) había quedado cerrada y pusheada correctamente sin intervención (`b49a3cbc8` + `f5a2476a1` + `122ea57a3`, árbol limpio en ese momento) antes de tocar nada nuevo.

**Diagnóstico confirmado:** el proyecto compila con `-flto=auto` (GCC). Este flag hace que **cada link individual** paralelice internamente su propia fase LTRANS hasta `nproc()` hilos. Sin un límite a nivel de Ninja, varios de esos links (el ejecutable principal `beteliney` + las ~16 herramientas chicas del repo) corrían en simultáneo, cada uno reclamando hasta 8 hilos propios — en una máquina de 8 cores / 13GB RAM esto multiplicaba el paralelismo real muy por encima de lo disponible, generando *swap thrashing* que se manifestaba como un cuelgue total del build. Confirmado contra el histórico: mismo síntoma documentado en sesiones 20, 27, 29, 31, 32, 33 y 36.

**Fix aplicado (`CMakeLists.txt`, tras el bloque `if(ENABLE_LTO)`):**
```cmake
set_property(GLOBAL PROPERTY JOB_POOLS lto_link_pool=2)
set(CMAKE_JOB_POOL_LINK lto_link_pool)
```
Limita a 2 los links simultáneos permitidos por Ninja cuando LTO está activo. Cada uno sigue usando sus propios hilos de LTRANS internamente, pero ya no se pisan entre sí por cores/RAM.

**Verificación:** reconfiguración de CMake confirmó el pool aplicado a todos los link edges del `build.ninja` generado. Rebuild forzado completo (tocando un header compartido para invalidar todos los targets): **72/72 objetivos compilados y linkeados sin cuelgue**, memoria estable entre 6.3GB y 7.5GB de 13GB durante todo el proceso (nunca tocó swap). Cero errores, cero warnings en el log completo. `ctest` corrido después: **29/29 tests pasando**, incluyendo `GDLauncherMigrator` — bloqueado desde sesión 32 precisamente por este mismo cuelgue, ahora verificado sin intervención manual.

**Limpieza de paso:** dos archivos huérfanos y vacíos (`buildconfig/BuildConfig.h.in`, `launcher/BuildConfig.h.in`, 0 bytes cada uno) quedaron sin trackear de una operación de sesión anterior — no correspondían a ningún artefacto real del sistema de build (el archivo que el proyecto trackea es `buildconfig/BuildConfig.h`, sin `.in`; el `.in` real es `BuildConfig.cpp.in`). Eliminados antes de comitear.

**Commit:** `3bba3a54c` ("fix(build): job pool para links con LTO, resuelve cuelgue histórico"), 1 archivo, 11 inserciones.

**Con esto quedan cerrados los pendientes 5 (`ctest` de GDLauncherMigrator) y 7 (causa raíz del cuelgue de LTO) que venían arrastrándose desde sesión 32.** Pendientes reales restantes: 6 puntos (ver bloque ESTADO ACTUAL), de los cuales solo 2 dependen de acción del usuario (API key de abuse.ch, confirmación de purga de historial de git). El resto (meta server sin auditar línea por línea, pruebas manuales de GUI, firma real en CI end-to-end, sandboxing con Bubblewrap) se puede seguir trabajando sin bloqueos.

---

### Sesión 38 — Sandboxing Bubblewrap + fix parcial de path traversal en el feed de meta remoto (2026-07-11) — **documentada retroactivamente en sesión 39**

> **Nota:** esta entrada se escribió en sesión 39, no en el momento. La sesión 38 terminó (aparentemente por corte de contexto) sin dejar su entrada en `ESTADO.md`, a pesar de haber hecho 2 commits reales el mismo día que el cierre de sesión 37. Sesión 39 detectó el hueco comparando `git log` contra este archivo y reconstruyó el contenido a partir de los diffs reales de los commits — no hay reporte original de la sesión 38 de dónde tomar contexto adicional (decisiones descartadas, alternativas consideradas, etc.), así que lo de abajo es lo que el código y los mensajes de commit permiten verificar con certeza.

**1) Sandboxing opcional con Bubblewrap (`bwrap`) para el proceso de Minecraft en Linux.** Cierra el ítem 11 del backlog de mejoras (sesión 35), pendiente 8. Archivos nuevos: `launcher/minecraft/launch/BubblewrapSandbox.h`/`.cpp`. Opción configurable (no forzada por defecto — no todos los sistemas tienen `bwrap` instalado ni todas las configuraciones de instancia son compatibles con un sandbox estricto, p.ej. mods que necesitan acceso a rutas fuera de la instancia).

**2) Fix de seguridad: path traversal en `uid`/`version` del feed de meta remoto (`ElPibeCapo/meta`).** Commit `254f05760`. Hallazgo: `VersionList::localFilename()`/`Version::localFilename()` construyen rutas de archivo de cache concatenando directamente `uid`/`version` sin sanitizar; `FS::RemoveInvalidPathChars` solo filtra caracteres inválidos de NTFS/FAT en Windows y **no bloquea `/` ni `..` en Linux**. Un feed comprometido (o un `MetaURLOverride` apuntando a un host hostil) podía inyectar un `uid`/`version` tipo `"../../../.ssh"` y forzar lectura/escritura/borrado fuera del directorio de cache esperado, a través de `BaseEntityLoadTask` y `HttpMetaCache::resolveEntry()`.

**Fix:** nuevas funciones `isSafePathComponent()` (rechaza vacío, `.`/`..`, cualquier valor con `..`, `/`, `\`, o byte nulo, tamaño >256) y `requireSafePathComponent()` (envuelve `Json::requireString` + la validación, lanza `Meta::ParseException` si falla) en `meta/JsonFormat.cpp`, ambas `static` en ese momento (session 39 expuso la primera públicamente — ver esa entrada). Aplicadas en `parseIndexInternal` (uid de cada package), y en `parseCommonVersion` (uid + version del objeto Version). **Cobertura real en ese momento: solo el feed remoto — NO cubría `mmc-pack.json` local ni `Require::uid` compartido, ver hallazgo de sesión 39 más abajo.**

**Test añadido en el mismo commit:** `tests/MetaPathTraversal_test.cpp` (nuevo archivo), 9 casos: 5 sub-casos maliciosos parametrizados contra `Meta::parseIndex` (dotdot slash, `..`, dotdot embebido, backslash, vacío) + `parseVersionList` rechaza uid malicioso + `parseVersion` rechaza version maliciosa + `parseIndex` acepta uid legítimo. Todos verificados pasando en sesión 39 al correr `ctest` completo (antes de la extensión de sesión 39, este archivo ya tenía 9 tests; después de la extensión, 16).

**Commits de la sesión (orden real en `git log`):** `da70d0e6b` (bubblewrap) y `254f05760` (fix path traversal), ambos `2026-07-11`, ambos posteriores a `8cbdfffed` (cierre docs de sesión 37).

**Estado al cerrar sesión 38 (reconstruido):** pendiente 8/ítem 11 (bubblewrap) cerrado. Pendiente 1 (meta server) parcialmente cerrado — el path traversal del feed remoto sí, el resto de la auditoría línea por línea, no.

---

### Sesión 39 — Auditoría de continuación del meta server: 2 vectores de path traversal adicionales encontrados y cerrados, documentación de sesión 38 reconstruida (2026-07-14)

**Contexto:** el usuario pidió continuar el trabajo pendiente del proyecto ("revisa sugerencias... haz lo mejor de lo mejor"). Antes de tocar código, `git log --oneline -8` mostró 2 commits (`254f05760`, `da70d0e6b`) posteriores a la última entrada documentada en `ESTADO.md` (sesión 37, `8cbdfffed`) — hueco de documentación real, cerrado primero con la entrada de "Sesión 38" de arriba, reconstruida desde los diffs de esos commits (ver nota ahí).

**Con la documentación al día, se retomó la auditoría del meta server (pendiente 1) desde donde el fix de sesión 38 la había dejado.** Ese fix cerró el path traversal de `uid`/`version` **solo en `meta/JsonFormat.cpp`** (parser del feed remoto). Rastreando todos los consumidores de `uid` en el código (`Index::get()`, `VersionList`, `Component::getFilename()`, `PackProfile.cpp`, `ComponentUpdateTask.cpp`) se encontraron dos rutas de entrada de datos **locales** no confiables que ese fix no tocaba:

**Hallazgo 1 — `Component::m_uid` desde `mmc-pack.json` de la instancia.** `PackProfile.cpp::componentFromJsonV1` leía `uid` con `Json::requireString` plano, sin ninguna validación, y lo usaba para construir un `Component`. Ese `m_uid` alimenta `Component::getFilename()` → `PackProfile::patchFilePathForUid(uid)` → `FS::PathCombine(instanceRoot(), "patches", uid + ".json")` — sin sanitizar. `Component::customize()` escribe en esa ruta; `Component::revert()` la **borra** con `FS::deletePath()` si existe. `mmc-pack.json` no es un archivo remoto: viaja dentro de instancias exportadas/compartidas, y puede venir de modpacks de terceros (CurseForge, Modrinth, GDLauncher, zips manuales) — un `uid` tipo `"../../../../home/usuario/.bashrc"` en un componente de un modpack malicioso habría permitido borrar (o sobrescribir, si el parseo de JSON del archivo objetivo fallara de forma explotable) un archivo arbitrario del sistema del usuario que lo instala, con solo que el componente pase por `customize()`/`revert()`.

**Hallazgo 2 — `Meta::Require::uid`, más grave por ser automático.** `Meta::parseRequires()` (en `JsonFormat.cpp`) parsea el campo `"uid"` de cada entrada de los arrays `"requires"`/`"conflicts"` con `requireString` plano — nunca se le aplicó la validación de sesión 38 porque es una función genérica, no específica del feed remoto. Esta MISMA función se usa en 3 lugares: (a) el feed remoto (ya protegido indirectamente porque el resto del objeto Version sí se valida, pero el propio `Require::uid` no); (b) `PackProfile.cpp` leyendo `cachedRequires`/`cachedConflicts` de `mmc-pack.json` local; (c) `OneSixVersionFormat.cpp` leyendo `patches/*.json`. El vector real: `ComponentUpdateTask::resolveDependencies()` recorre `component->m_cachedRequires`, y si una dependencia falta, la inyecta automáticamente: `makeShared<Component>(d->m_profile, add.uid)` seguido de `insertComponent(...)` — **esto corre en cada resolve/launch de instancia, sin que el usuario interactúe con nada**, e igual que el hallazgo 1, termina en `Component::getFilename()` y en `metadataIndex()->get(uid)` (`Index::get()`, que tampoco validaba nada al construir `std::make_shared<VersionList>(uid)`).

**Fix aplicado (mínimo, en el punto de entrada, mismo patrón que sesión 38):**
- `meta/JsonFormat.h`: se expone `bool isSafePathComponent(const QString&)` (antes `static`/interna a `JsonFormat.cpp`) para que otros consumidores de datos no confiables puedan usarla.
- `meta/JsonFormat.cpp`: se quita `static` de la definición; `parseRequires()` ahora valida el `uid` de cada `Require` con la ya existente `requireSafePathComponent()` en vez de `requireString()` plano — esto cierra el hallazgo 2 en los 3 call sites de una sola vez, porque `parseRequires` es la única implementación compartida.
- `minecraft/PackProfile.cpp`: `componentFromJsonV1` valida el `uid` de nivel superior con `Meta::isSafePathComponent()` antes de construir el `Component`; si es inseguro, `throw JSONValidationError(...)` (mismo tipo de excepción que ya usa este archivo para otros errores de formato). El `catch` de `loadPackProfile` se amplió de `catch (const JSONValidationError&)` a `catch (const Exception&)` — `Meta::ParseException` (lo que lanza `parseRequires` internamente) no hereda de `JSONValidationError`/`Json::JsonException`, sino de la clase base común `Exception`; sin ampliar el catch, un `mmc-pack.json` malicioso con un `Require::uid` inseguro habría lanzado una excepción no capturada y tumbado la app en vez de fallar de forma controlada (mismo resultado final — instancia rechazada — pero de forma segura, no un crash).
- Se verificó que `OneSixVersionFormat.cpp` (el tercer call site de `parseRequires`, vía `ProfileUtils::parseJsonFile` → `guardedParseJson`) **ya envolvía la llamada en `catch (const Exception& e)`**, así que quedó protegido sin ningún cambio ahí — se confirmó leyendo el código, no se asumió.

**Test:** 7 casos nuevos en `tests/MetaPathTraversal_test.cpp` (mismo archivo del hallazgo de sesión 38, mismo dominio de seguridad): `test_requiresRejectsTraversalUid_data/test` (5 sub-casos: dotdot slash, `..`, dotdot embebido, backslash, vacío) + `test_requiresAcceptsLegitimateUid` (caso feliz). No se testeó `componentFromJsonV1` directamente porque es `static`/interna a `PackProfile.cpp` — probarla habría requerido construir una `MinecraftInstance` completa en disco, y no hay precedente de eso en la suite de tests actual; se dejó como pendiente honesto (ver bloque ESTADO ACTUAL, no se infló la cobertura de tests para aparentar más de lo hecho).

**Verificación:** build completo `ninja` desde cero tras los cambios: **180/180 sin errores**. `ctest` completo: **30/30 pasan** (antes de esta sesión eran 29 según sesión 37 + 1 nuevo target `MetaComponentParse` que ya existía sin documentar — no relacionado con esta auditoría, es un parser de componentes de chat de Minecraft tipo `{"text":"foo"}`, se verificó su código para descartar relación). `MetaPathTraversal` específicamente: **16/16** (9 preexistentes de sesión 38 + 7 nuevos de esta sesión).

**Commit:** pendiente de crear (ver diff: `launcher/meta/JsonFormat.h` +5, `launcher/meta/JsonFormat.cpp` +11/-2, `launcher/minecraft/PackProfile.cpp` +23/-1, `tests/MetaPathTraversal_test.cpp` +65 — 4 archivos, ~101 inserciones netas).

**Con esto, el path traversal de `uid`/`version` en TODO el sistema de metadatos (feed remoto, `mmc-pack.json` local, `patches/*.json`, y `Require` en cualquiera de los tres) queda cerrado.** El pendiente 1 (meta server) sigue abierto pero acotado: falta la auditoría del resto (tamaño/estructura de respuestas HTTP, manejo de errores de red, TLS si aplica) — el vector de path traversal específicamente ya no es un pendiente real.

**Lección operativa reforzada (la misma que sesión 38 no siguió):** `git log --oneline -N` contra la última entrada de `ESTADO.md` es el primer paso de CUALQUIER sesión nueva en este proyecto, antes de leer código o planear trabajo — es lo que permitió detectar el hueco de sesión 38 en 30 segundos en vez de re-auditar por las dudas.


