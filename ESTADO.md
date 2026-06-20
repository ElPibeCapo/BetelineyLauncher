# ESTADO — BetelineyLauncher
> Documento único y autocontenido. Cualquier chat nuevo lee SOLO esto y puede continuar.
> Última actualización: sesión 11 (2026-06-20) — crash crítico en ejecución real corregido y verificado, Día 1-2 del plan de lanzamiento cerrados, repo publicado en v8.3.0.

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
| **CurseForge** | ✅ | `***CURSEFORGE_KEY_ROTADA_PURGADA***` — CI usa secret `CURSEFORGE_API_KEY`. Local: `export CURSEFORGE_API_KEY="..."` antes de cmake. |
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
→ Valor:   ***CURSEFORGE_KEY_ROTADA_PURGADA***
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

Servidor Discord: https://discord.gg/2JdB7pvBq3 — badge añadido al README (commit `6346499`).

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
| 4 | Verificar test de traducción con `BUILD_TESTING=ON` (código fuente ya se ve correcto) | ⏳ |
| 6 | Publicar en r/feedthebeast, r/Minecraft, Discord de Prism Launcher | ⏳ |
| 7 | Formulario claude.com/contact-sales/claude-for-oss (deadline 30/06/2026) | ⏳ |

**Estado real del launcher a cierre de esta sesión:** compila limpio (403/403), arranca sin crashear, GUI funcional confirmada visualmente. Repo público al día (`8cf6afc`), release v8.3.0 publicada vía CI. El bloqueo principal para los Días 3-4 sigue siendo activar GitHub Pages en el repo `meta` (acción manual de navegador, pendiente #2 de la sección "Acciones manuales pendientes").
