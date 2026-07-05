# ESTADO — BetelineyLauncher
> Documento único y autocontenido. Cualquier chat nuevo lee SOLO esto y puede continuar.
> Última actualización: sesión 22 (2026-07-05) — limpieza de estructura del repo (stubs deprecated eliminados, carpeta `tools/pc` renombrada). Sesión 21: ícono macOS corregido + bug en `genicons.sh`. Sesión 20: backport de 3 fixes reales de Prism Launcher 11.0.0→11.0.2 confirmado en CI (verde) y testeado en la práctica: build local limpio (0 errores/warnings con -Werror), binario ejecutado estable, descarga real desde el meta server confirmada en producción (no solo con curl), 29/29 tests pasando. Corregido además un problema de configuración de `gh` (repo default apuntaba a PrismLauncher/PrismLauncher por el remote `upstream`).

---

## IDENTIDAD DEL PROYECTO

| | |
|---|---|
| **Nombre** | BetelineyLauncher |
| **Versión actual en código** | v8.3.0 |
| **Próxima release** | v8.3.0 — tagear con `git tag v8.3.0 && git push --tags`
| **Base** | Prism Launcher (GPL-3.0), fork extensamente modificado |
| **Autor** | El_PibeCapo — `elpibecapoofficial@gmail.com` |
| **Repo launcher** | https://github.com/ElPibeCapo/BetelineyLauncher |
| **Repo meta** | https://github.com/ElPibeCapo/meta |
| **Rama principal** | `main` |
| **META server** | https://ElPibeCapo.github.io/meta/v1/ |
| **Código fuente local** | `/home/pibe/Descargas/Beteliney Launcher [Minecraft]/BetelineyLauncher/source/` |

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

```bash
cd "/home/pibe/Descargas/Beteliney Launcher [Minecraft]/BetelineyLauncher/source"
git add -A
git commit -m "descripción del cambio"
git tag v8.3.0
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
| **CurseForge** | 🔴 | Key EXPUESTA públicamente (commits viejos, repo público) — ROTACIÓN PENDIENTE, no hecha aún. Valor sacado de los archivos actuales (ver Sesión 15). El secret `CURSEFORGE_API_KEY` en GitHub Actions NO existe (`gh secret list` vacío) — CI no tiene la key hasta que se rote y se cargue la nueva. |
| **Microsoft Azure** | ✅ | App ID: `4b945c78-d30b-489e-915f-b361bf9c933b` |
| **Imgur** | ⚠️ | Key vacía. El código de upload existe. Registrar en `api.imgur.com/oauth2/addclient` si se activa. |
| **META server** | ✅ | Rama `gh-pages` del repo meta tiene todos los JSONs generados. CI corre cada 6h. |

---

## ACCIONES MANUALES PENDIENTES (requieren navegador con sesión GitHub)

**#1 — Secret CurseForge en CI** (sin esto el build de CI no tiene la key):
```
https://github.com/ElPibeCapo/BetelineyLauncher/settings/secrets/actions
→ New repository secret
→ Nombre:  CURSEFORGE_API_KEY
→ Valor:   (la key NUEVA tras rotar en CurseForge — NUNCA la antigua, fue expuesta públicamente, ver Sesión 15)
```

**#2 — GitHub Pages del META server** (el contenido ya existe, falta activarlo):
```
https://github.com/ElPibeCapo/meta/settings/pages
→ Source: "Deploy from a branch"
→ Branch: gh-pages → / (root) → Save
→ URL resultante: https://elpibecapo.github.io/meta/v1/
```

**#3 — Crear feed de noticias** (cuando haya algo que anunciar):
Crear `gh-pages/v1/news/feed.atom` en el repo meta con formato Atom estándar.
El `NewsChecker.h` ya lo consume automáticamente.

**#4 — Crear packs de BetelineyPacks** (cuando haya packs para publicar):
- `gh-pages/v1/beteliney-packs/index.json` → `{"formatVersion":1,"ids":["id1","id2"]}`
- `gh-pages/v1/beteliney-packs/{id}.json` → ver formato en sección Fase 3

**#5 — Crear lista negra de malware** (para que MalwareScanner funcione en producción):
Crear `gh-pages/v1/malware/known-hashes.json`:
```json
{
  "formatVersion": 1,
  "updated": "2026-06-17",
  "hashes": { "sha256": [], "sha512": [] },
  "sources": ["fractureiser"]
}
```

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

**Actual en código:** v8.3.0
**Para publicar:** `git tag v8.3.0 && git push --tags`

---

## QUÉ SIGUE (IDEAS FUTURAS, NO PLANIFICADAS)

- **Flathub** — submitear el manifest Flatpak para revisión oficial
- **macOS** — el código heredado existe (Sparkle updater, entitlements), sin CI activo
- **Sincronización en nube de instancias** — GDLauncher Carbon lo tiene, requiere backend propio
- **Verificación de mods en instancias existentes** — escanear mods ya instalados con MalwareScanner
- **Soporte ARM64** — cambiar `-march=znver1` por detección automática en CI
- **i18n propio** — el sistema de traducciones de Prism existe, conectar a Weblate o similar

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


