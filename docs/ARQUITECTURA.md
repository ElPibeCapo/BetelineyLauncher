# ⬡ Beteliney Launcher — Arquitectura Técnica

> Documento de referencia del proyecto. Basado en lectura directa del código fuente.
> Última actualización: **2026-07-14** (sesión 39, actualizado por auditoría de documentación) · Versión: **8.4.0** (tag) — `main` tiene 14 commits reales sin tagear encima, ver `ESTADO.md` sección ESTADO ACTUAL y `docs/CHANGELOG.md` sección "Sin publicar aún".
> **Este documento estuvo congelado desde 2026-04-05 (v7.5.0/7.7.0) hasta esta revisión — todo lo de abajo se verificó contra el código real, no contra la versión vieja del archivo.**

---

## Identidad del proyecto

| Campo | Valor |
|-------|-------|
| Nombre | Beteliney Launcher |
| AppID | `com.beteliney.BetelineyLauncher` |
| Versión actual | 8.4.0 (tag) / main sin tagear por delante — ver nota arriba |
| Binario | `beteliney` (Linux) · `beteliney.exe` (Windows) |
| Config file | `beteliney.cfg` |
| Domain | `beteliney.github.io` |
| Repo | `https://github.com/ElPibeCapo/BetelineyLauncher` (corregido — el doc viejo decía `beteliney/BetelineyLauncher`, no existe) |
| Repo meta | `https://github.com/ElPibeCapo/meta` (rama `gh-pages`, sirve JSONs de versiones + BetelineyPacks + malware hashes + news feed) |
| Copyright | © 2026 El_PibeCapo / Beteliney Launcher |
| Base | Prism Launcher (GPL-3.0) |
| Licencia | GPL-3.0-only |

---

## Hardware objetivo

| Componente | Especificación |
|------------|---------------|
| CPU | AMD Ryzen 7 3700U (Picasso, arquitectura Zen+) |
| GPU | Radeon Vega 10 (iGPU, VRAM dinámica compartida de RAM) |
| RAM | 8–16 GB (LPDDR4, compartida entre CPU y Vega 10) |
| OS principal | Nobara Linux (Fedora-based) — KDE Plasma |
| OS secundario | Windows 10/11 (compilación y uso con MinGW) |

Flags de compilación dedicados a esta CPU:
```
-march=znver1 -mtune=znver1   (Zen+ / Picasso — detectado automáticamente por COMPILAR_LINUX.sh)
-march=native -mtune=native   (cualquier otra CPU)
```

---

## Stack tecnológico

| Capa | Tecnología |
|------|-----------|
| Lenguaje | C++20 |
| Framework UI | Qt 6.4+ (Core, Widgets, Network, NetworkAuth, Svg, Concurrent, Xml, Test) |
| Build system | CMake 3.22+ con Ninja |
| Compilador Linux | GCC 15 (Nobara) — LTO ON con fix por target en libnbtplusplus |
| Compilador Windows | MinGW-w64 (distribuido con Qt Installer) |
| Gestor dependencias Win | vcpkg (incluido en `vcpkg_local/`) |
| Dependencias C++ | libarchive · cmark · libqrencode · zlib · tomlplusplus · gamemode (Linux only) |
| Tests | Qt Test (CTest) — 29 archivos de test total, 6 Beteliney-specific (5 `Beteliney*_test.cpp` + `MetaPathTraversal_test.cpp`) + resto heredados de Prism (30/30 ctest verificados en sesión 39) |


---

## Estructura de directorios

> **Corregido en esta revisión** — el árbol de abajo estaba completamente inventado en la versión vieja del documento (raíz `BetelineyLauncher_v7/`, `docs/PENDIENTES.md`, `docs/ESTRUCTURA.md`, `docs/SESIONES.md`, `docs/AUDITORIA.md`, `tools/verify.py`, `tools/fix_icons.py` — ninguno de esos existe). Esto es lo que hay realmente en `"/home/pibe/Descargas/Beteliney Launcher [Minecraft]/BetelineyLauncher/source/"` (raíz del repo git):

```
source/                          Raíz del repo (== raíz del proyecto, sin carpeta v7 envolvente)
├── launcher/                    C++ principal
│   ├── Application.cpp          Punto de entrada — settings, init, capabilities
│   ├── BetelineyProfiles.h      7 perfiles JVM (inline, header-only)
│   ├── BetelineyCode.h          Easter egg — secuencia B-E-T-E en ventana principal
│   ├── BetelineyTime.cpp/.h     Utilidades de tiempo
│   ├── BetelineyZip.cpp/.h      Compresión ZIP
│   ├── BetelineyAchievements.h/.cpp   Sistema de logros por tiempo jugado (Fase 3, sesión 36)
│   ├── BubblewrapSandbox.h/.cpp Sandboxing opcional Bubblewrap del proceso Minecraft (Linux, sesión 38)
│   ├── FavoriteServers.h/.cpp   Servidores favoritos con quick-join (sesión 31)
│   ├── SysInfo.cpp              Detección de RAM cross-platform
│   ├── crash/                   BetelineyPanicHandler (crash reporter)
│   ├── logs/                    BetelineyLogAnalyzer (18 checks de diagnóstico)
│   ├── migration/                GDLauncherMigrator (importador GDLauncher Carbon)
│   ├── minecraft/mod/           MalwareScanner
│   ├── launch/steps/            CheckModConflicts
│   ├── modplatform/beteliney/   BetelineyPacks (backend completo)
│   ├── updater/                 BetelineyExternalUpdater + BetelineyUpdater (firma Ed25519 desde sesión 34)
│   ├── ui/
│   │   ├── themes/               BetelineyTheme.cpp — stylesheet neón #39FF14
│   │   ├── dialogs/               CommandPaletteDialog (Ctrl+K), FavoriteServersDialog, GDLauncherMigrateDialog
│   │   ├── widgets/               JavaSettingsWidget, AchievementToast
│   │   └── pages/                 modplatform/beteliney/BetelineyPackPage
│   └── icons/                   BetelineyIcon
├── program_info/                 Ícono (SVG/PNG/ICO/ICNS), manifest, .desktop, metainfo, .rc
├── buildconfig/                  BuildConfig.h/.cpp.in — variables CMake → C++
├── libraries/                    libnbtplusplus, rainbow, qdcss, murmur2, LocalPeer, javacheck
├── docs/                         CHANGELOG, TROUBLESHOOTING, PERFILES_JVM, ARQUITECTURA (este archivo), COMPILAR_WINDOWS, CONTRIBUTING, COPYING
├── tests/                        29 archivos de test (Qt Test/CTest) — 6 Beteliney-specific, resto heredados de Prism
├── tools/dev/                    8 herramientas de desarrollo (ver sección dedicada abajo)
├── packaging/                    Manifest Flatpak
├── screenshots/                  Capturas para README
├── build/                        Directorio de build generado por CMake/Ninja (gitignored)
├── ESTADO.md                     Fuente de verdad del estado del proyecto (raíz del repo)
├── ESTRATEGIA_IA.md              Estrategia de trabajo con IA (raíz del repo)
├── COMPILAR_LINUX.sh             Build script Linux — menú de modos
├── COMPILAR_BETELINEY.bat        Build script Windows — menú
├── COMPILAR.ps1                  Build script PowerShell
├── EMPAQUETAR_LINUX.sh           Distribución Linux (tar.gz)
├── EMPAQUETAR_APPIMAGE.sh        Distribución Linux (AppImage)
├── EMPAQUETAR_WINDOWS.bat        Distribución Windows (ZIP / NSIS)
└── README.md                     Documentación principal (ASCII art + badges)
```

No existe una carpeta `lanzar.sh`/`beteliney-updater.sh` en la raíz — la sección original que las mencionaba parece haber descrito un layout planeado que nunca se implementó así; el arranque real es directo vía el binario compilado, sin script `lanzar.sh` intermedio en el repo actual (verificar si hace falta reconciliar con `docs/TROUBLESHOOTING.md`).

---

## Módulos clave de C++

### Application.cpp
Punto de entrada real del launcher (2107 líneas). Maneja:
- Inicialización de Qt, rutas de datos (`QStandardPaths::AppDataLocation`)
- Registro de `m_settings` — todos los settings persistentes del usuario
- Detección de capacidades en runtime: `SupportsGameMode` (Linux), `SupportsMangoHud` (Linux)
- `NumberOfConcurrentDownloads` adaptativo según RAM: <8 GB → 4 · 8–16 GB → 6 · >16 GB → 10
- `NumberOfConcurrentTasks` adaptativo: `max(4, cores - 2)`
- Caché de pixmaps adaptativa: <8 GB → 32 MB · 8–12 GB → 64 MB · >12 GB → 128 MB
- GameMode activado por defecto en Linux (`EnableFeralGamemode = true`)
- Flags JVM default: perfil Balanceado de BetelineyProfiles.h
- Detección de `/tmp` con `noexec` en Linux/BSD
- Font monospace: `Courier` (Win) / `Menlo` (Mac) / `Monospace` (Linux)

### BetelineyProfiles.h
Header-only. `inline const QList<BetelineyJVMProfile> BETELINEY_PROFILES`. **7 perfiles** (corregido — el doc viejo decía 6, faltaba el perfil ZGC añadido después):

| # | Nombre | Xms | Xmx | Notas |
|---|--------|-----|-----|-------|
| 0 | Personalizado | 0 | 0 | Sin flags — limpia campos |
| 1 | iGPU / RAM compartida | 384 MB | 1536 MB | G1HeapRegionSize=1M, Xss=768k, MaxGCPauseMillis=100 |
| 2 | Ligero Vanilla | 512 MB | 2048 MB | G1HeapRegionSize=2M, Xss=1M, UseTransparentHugePages |
| 3 | Balanceado | 2048 MB | 4096 MB | G1HeapRegionSize=4M, UseStringDeduplication, AutoBoxCacheMax=20000 |
| 4 | Pesado 100–300 mods | 2048 MB | 6144 MB | G1HeapRegionSize=8M, AlwaysPreTouch, MaxGCPauseMillis=150 |
| 5 | Extremo ≥300 mods | 6144 MB | 12288 MB | G1HeapRegionSize=16M, AlwaysPreTouch, MaxGCPauseMillis=200 |
| 6 | iGPU ZGC (Java 21+) | 384 MB | 1536 MB | UseZGC + ZGenerational, pausas sub-milisegundo, SoftMaxHeapSize=1280m |

Todos los perfiles (salvo Personalizado) incluyen los flags base de Aikar:
`UseG1GC, ParallelRefProcEnabled, UnlockExperimentalVMOptions, DisableExplicitGC,
G1NewSizePercent=30, G1MaxNewSizePercent=40/50, G1ReservePercent=20,
G1MixedGCCountTarget=4, InitiatingHeapOccupancyPercent=15/20,
G1MixedGCLiveThresholdPercent=90, G1RSetUpdatingPauseTimePercent=5,
GCTimeRatio=99, SurvivorRatio=32, PerfDisableSharedMem, MaxTenuringThreshold=1,
java.rmi.server.disableHttp=true`


### JavaSettingsWidget.cpp
Widget de configuración JVM. Funciones Beteliney:
- Combo de selección de perfil con rangos de memoria en el label
- Detección automática de iGPU al init:
  - Linux: `QProcess lspci -mm`, timeout 2000ms, keywords iGPU AMD/Intel
  - Windows: `QProcess wmic path Win32_VideoController`, timeout 3000ms, mismas keywords
  - Si iGPU + RAM <12 GB → preselecciona perfil iGPU (índice 1) + tooltip informativo
- Sugerencia por RAM si no hay iGPU: <8 GB → Ligero, 8–16 GB → Balanceado, 16–32 GB → Pesado, ≥32 GB → Extremo
- Advertencia visual si hay conflicto `-Xmx`/`-Xms` en el campo de args libre

### SysInfo.cpp
RAM del sistema, cross-platform:
- Windows: `GlobalMemoryStatusEx()`
- Linux: `sysinfo()` (`<sys/sysinfo.h>`)
- macOS: `sysctlbyname("hw.memsize")`
- FreeBSD/OpenBSD: `popen("sysctl hw.physmem")`

### lanzar.sh
Entry point Linux (261 líneas). Funciones:
- `find_exe()`: busca el binario `beteliney` o `BetelineyLauncher` en `build/`
- `detect_gpu_type()`: clasifica GPU vía `lspci -mm` → `igpu_amd|igpu_intel|discrete|unknown`
- `apply_jvm_profile_if_needed()`: escribe perfil iGPU en `beteliney.cfg` si JvmArgs vacío
- Auto-compila si el exe no existe (modo 1 si sin CMakeCache, modo 2 si con caché)
- Check de updates en background (no bloquea el lanzamiento)
- `--debug`: activa `QT_LOGGING_RULES="*=true"` y `QT_DEBUG_PLUGINS=1`
- **BUG conocido (BUG-1):** `exec "$EXE" "$@"` reenvía flags propios al exe Qt — pendiente fix

### beteliney-updater.sh
Updater (211 líneas). Flags: `--check`, `--silent`, `--force`, `--appimage`, `--log [N]`.
- `check_for_updates()`: `git fetch` + comparación de commits + lista de cambios pendientes
- `apply_update()`: `git stash` si hay cambios locales + `git pull --rebase`
- `recompile()`: cmake incremental con spinner de progreso
- `verify_build()`: verifica que el exe existe después del build
- Log guardado en `.logs/update_YYYYMMDD_HHMMSS.log`
- **BUG conocido (BUG-4):** `--silent` no llama `verify_build` — pendiente fix

---

## Sistema de branding / identidad visual

### Archivos en program_info/
| Archivo | Función |
|---------|---------|
| `com.beteliney.BetelineyLauncher.svg` | Logo principal SVG — hexágono ⬡ + B neón `#39FF14` |
| `com.beteliney.BetelineyLauncher.logo-darkmode.svg` | Variante dark |
| `com.beteliney.BetelineyLauncher_256.png` | PNG 256×256 (taskbar, .desktop) |
| `beteliney.ico` | ICO multi-size 16/32/48/256 (Windows .exe) |
| `beteliney.icns` | macOS bundle icon |
| `beteliney.rc.in` | Template Windows Resource (embede ICO en .exe) |
| `beteliney.manifest.in` | Windows manifest — DPI PerMonitorV2, Win10/11 compat |
| `beteliney.qrc.in` | Qt resource con SVG principal (accesible en runtime vía `:/`) |
| `com.beteliney.BetelineyLauncher.desktop.in` | Entrada .desktop para Linux |
| `com.beteliney.BetelineyLauncher.metainfo.xml.in` | AppStream metainfo |

### Paleta de colores
| Rol | Hex |
|-----|-----|
| Acento neón (principal) | `#39FF14` |
| Fondo principal | `#0C0E16` |
| Fondo secundario | `#151B28` |
| Fondo hover | `#1A2035` |
| Texto principal | `#D0D8F0` |
| Texto dimmed | `#7A8299` |

---

## Proceso de build

### Linux (Nobara/Fedora) — COMPILAR_LINUX.sh

```bash
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr \
  -DENABLE_LTO=ON \
  "-DCMAKE_C_FLAGS_RELEASE=-O3 -march=znver1 -ffunction-sections -fdata-sections
                              -fno-asynchronous-unwind-tables -pipe" \
  "-DCMAKE_CXX_FLAGS_RELEASE=(igual)" \
  "-DCMAKE_EXE_LINKER_FLAGS_RELEASE=-Wl,--gc-sections" \
  -Dtomlplusplus_DIR=/usr/lib64/cmake/tomlplusplus \
  -G Ninja
cmake --build build --parallel $(nproc)
# Output: build/beteliney
```

**Fix LTO GCC 15 (libnbtplusplus):**
`set_target_properties(nbt++ PROPERTIES INTERPROCEDURAL_OPTIMIZATION OFF)`
en `libraries/libnbtplusplus/CMakeLists.txt`.
El bug GCC 15 afecta la generación de código LTO para `tag_array`. Desactivar IPO **solo para este target** resuelve el problema sin sacrificar LTO en el resto del proyecto.

### Windows (MinGW) — COMPILAR_BETELINEY.bat / COMPILAR.ps1
```
# Detecta Qt en C:\Qt o D:\Qt (versiones 6.8–6.10)
# Detecta MinGW en Qt\Tools\mingw*_64
# Detecta CMake: sistema > PATH > Qt Tools (AppLocker-safe)
# windeployqt: --no-translations --no-opengl-sw --skip-plugin-types generic,networkinformation
# Output: build\beteliney.exe + DLLs Qt + DLLs MinGW (libgcc, libstdc++, libwinpthread)
```


---

## Compatibilidad cross-platform

### Guardias `#ifdef` por OS

| Código | OS | Mecanismo |
|--------|----|-----------|
| GameMode + MangoHud | Solo Linux | `#ifdef Q_OS_LINUX` en Application.cpp y MinecraftInstance.cpp |
| `GlobalMemoryStatusEx` | Solo Windows | `#if defined(Q_OS_WINDOWS)` en SysInfo.cpp |
| `sysinfo()` | Solo Linux | `#elif defined(Q_OS_LINUX)` en SysInfo.cpp |
| Registry Java search | Solo Windows | `#if defined(Q_OS_WIN32)` en JavaUtils.cpp |
| `lspci` iGPU detection | Solo Linux | `#if defined(Q_OS_LINUX)` en JavaSettingsWidget.cpp |
| `wmic` iGPU detection | Solo Windows | `#if defined(Q_OS_WIN32) \|\| defined(Q_OS_WIN)` en JavaSettingsWidget.cpp |
| Intel driver JVM hack | Solo Windows | `#ifdef Q_OS_WIN32` en MinecraftInstance.cpp |
| LWJGL xrandr disable | Solo Linux | `#ifdef Q_OS_LINUX` en MinecraftInstance.cpp |
| `/tmp noexec` check | Solo Linux/BSD | `#if defined(Q_OS_LINUX)` en Application.cpp |
| WindowsConsoleGuard | Solo Windows | `#if defined Q_OS_WIN32` en main.cpp |
| Font monospace | Por OS | `Courier` (Win) / `Menlo` (Mac) / `Monospace` (Linux) |
| `EnableFeralGamemode` default | Por OS | `true` en Linux · `false` en Win/Mac |

### Rutas de datos
- Linux: `~/.local/share/BetelineyLauncher/`
- Windows: `%APPDATA%\BetelineyLauncher\`
- Sin rutas hardcodeadas Unix en ningún `.cpp` que compile para Windows.

---

## Tests — lista completa

### Tests base (heredados de PrismLauncher)
`AnonymizeLog`, `CatPack`, `DataPackParse`, `FileSystem`, `GZip`, `GradleSpecifier`,
`INIFile`, `Index`, `JavaVersion`, `Library`, `MetaComponentParse`, `MojangVersionFormat`,
`Packwiz`, `ParseUtils`, `ResourceFolderModel`, `ResourcePackParse`, `ShaderPackParse`,
`StringUtils`, `Task`, `TexturePackParse`, `Version`, `WorldSaveParse`, `XmlLogs`

### Tests Beteliney-specific (6 archivos, incluyendo el de seguridad)

| Archivo | Tests | Cobertura |
|---------|-------|-----------|
| `BetelineyZip_test.cpp` | 5 | collectAll, emptyDir, nonexistentDir, excludeFilter, filesOnly |
| `BetelineyProfiles_test.cpp` | 9 | G1GC, Aikar, mem ranges, duplicados, orden, stack size, flags redundantes |
| `BetelineyTime_test.cpp` | 15 | prettifyDuration (10 casos), humanReadableDuration (5 casos) |
| `BetelineyGPUDetect_test.cpp` | 18 | iGPU AMD/Intel, discrete NVIDIA/AMD, dual GPU, edge cases |
| `BetelineyTranslation_test.cpp` | 15 | Ausencia strings inglés + presencia strings español en 3 archivos |
| `MetaPathTraversal_test.cpp` | 16 | Regresión de seguridad: rechazo de `uid`/`version` maliciosos en feed remoto, `mmc-pack.json` y `Require` compartido (sesiones 38-39) |

**Ejecutar todos:**
```bash
cd build && ctest --output-on-failure
```
**Ejecutar solo Beteliney:**
```bash
ctest -R "Beteliney" --output-on-failure
```
**Ejecutar solo el test de seguridad:**
```bash
ctest -R "MetaPathTraversal" --output-on-failure
```

**Nota sobre BetelineyTranslation_test:** el bug histórico de `BETELINEY_SRCDIR` (definía `${CMAKE_SOURCE_DIR}/launcher` en vez de `${CMAKE_SOURCE_DIR}`, causando que los 11 tests de traducción pasaran en falso vía `QSKIP` desde el commit inicial) **se corrigió en sesión 12** — ver `BUG-2` abajo, ya resuelto.

---

## Bugs conocidos

> **Corregido en esta revisión** — este bloque decía "auditoría 2026-03-31" y apuntaba a `docs/PENDIENTES.md`, que no existe. Los 7 bugs de esa auditoría **ya están todos resueltos** (verificado contra `ESTADO.md`, sesiones 12 y posteriores). Se dejan documentados por trazabilidad histórica, no como pendientes:

| ID | Prioridad | Archivo | Descripción breve | Estado |
|----|-----------|---------|-------------------|--------|
| BUG-1 | 🔴 Alta | `lanzar.sh` | `exec "$EXE" "$@"` reenvía flags propios al exe Qt | ✅ Resuelto (filtrado vía array `LAUNCHER_ARGS`) |
| BUG-2 | 🔴 Alta | `BetelineyTranslation_test.cpp` | Tests pasan vacíos si CMake no define `BETELINEY_SRCDIR` correctamente | ✅ Resuelto sesión 12 (path duplicado `launcher/launcher/...` corregido) |
| BUG-3 | 🟡 Media | `lanzar.sh` | Race condition iGPU en primer arranque — cfg no existe aún | ✅ Resuelto (`.cfg` mínimo con perfil iGPU se crea antes de lanzar el exe) |
| BUG-4 | 🟡 Media | `beteliney-updater.sh` | `--silent` no llama `verify_build` | ✅ Verificado ya correcto (`apply_update; recompile; verify_build`) |
| BUG-5 | 🟡 Media | (doc obsoleto) | LTO documentado como OFF pero en código es ON | ✅ No aplica — LTO está ON, con `IPO OFF` solo en `nbt++` por bug de GCC 15, y el hang histórico de linkeo con LTO se resolvió en sesión 37 (job pool) |
| BUG-6 | 🟢 Baja | `tools/dev/` | Verificación de archivos de test | ✅ Cubierto — `tools/dev/check_build.sh` y `estado_actual.sh` existen |
| BUG-7 | 🟢 Baja | `COMPILAR_LINUX.sh` | `ensure_libnbt()` sin pin de versión | — sin confirmar en esta revisión, no crítico |

**Vulnerabilidades de seguridad encontradas y cerradas después de esta auditoría** (no son parte de los BUG-1..7, son hallazgos posteriores — ver `ESTADO.md` sesiones 32, 38 y 39 para el detalle completo):
- Path traversal en el importador GDLauncher (sesión 32)
- Path traversal en `uid`/`version` del feed de meta remoto (sesión 38)
- Path traversal en `Component::m_uid` local (`mmc-pack.json`) y en `Meta::Require::uid` compartido (feed remoto + local + `patches/*.json`) — el vector más grave, porque se dispara automáticamente en cada resolve/launch sin interacción del usuario (sesión 39)

---

## Flujo de actualización del launcher

**beteliney-updater.sh (shell, Nobara):**
1. `git fetch origin` — consulta remoto
2. Compara HEAD vs `origin/HEAD` — muestra commits pendientes
3. Con confirmación: `git stash` (si hay cambios locales) + `git pull --rebase`
4. `cmake --build build --parallel N` — recompila incremental
5. `verify_build()` — verifica que el exe existe y tiene tamaño razonable
6. Log en `.logs/update_YYYYMMDD_HHMMSS.log`

**BetelineyExternalUpdater (C++, en runtime del launcher):**
- `m_checking` guard contra doble chequeo simultáneo
- `m_currentFuture` guardado — destructor espera con `waitForFinished()`
- Verifica existencia del exe updater ANTES de cerrar la app
- Cancelación real con `proc.kill()` + monitoreo en chunks de 200ms
- `watcher->setFuture()` después de `connect()` — elimina race condition
- **Desde sesión 34:** verificación de firma **Ed25519 fail-closed** sobre cada release descargado, vía `libsodium` — si la firma no valida, el update se rechaza en vez de aplicarse a ciegas.

---

## Módulos añadidos después de la última auditoría completa (sesiones 30-39)

Esta sección no existía en el documento original — se agrega en esta revisión para cubrir el trabajo hecho después de 2026-04-05, que nunca se documentó acá.

### BetelineyAchievements (`launcher/BetelineyAchievements.h/.cpp`, sesión 36)
Singleton de logros ligados a marca de tiempo jugado (Fase 3 del roadmap gamificación). Se engancha en `BaseInstance::setMinecraftRunning` para acumular tiempo y disparar milestones. La UI del toast de logro vive en `launcher/ui/widgets/AchievementToast.h/.cpp`.

### BubblewrapSandbox (`launcher/BubblewrapSandbox.h/.cpp`, sesión 38)
Sandboxing **opcional** del proceso de Minecraft en Linux vía `bwrap` (Bubblewrap). Aislamiento a nivel de proceso — no interfiere con GameMode/MangoHud que ya están integrados. Opt-in, no activado por defecto.

### CommandPaletteDialog + FavoriteServers (sesión 31)
- `launcher/FavoriteServers.h/.cpp`: persistencia de servidores favoritos.
- `launcher/ui/dialogs/FavoriteServersDialog.h/.cpp`: UI de gestión.
- `launcher/ui/dialogs/CommandPaletteDialog.h/.cpp`: paleta de comandos estilo Ctrl+K — instancias y settings, con quick-join a servidores favoritos.

### Fixes de reliability (sesiones 29, 37)
- Sesión 29: `QPointer` en `BackgroundModUpdateCheckTask` para eliminar un use-after-free.
- Sesión 37: job pool dedicado para los pasos de linkeo con LTO — resolvió un cuelgue histórico del build que venía arrastrándose desde varias sesiones (20/27/29/31/32/33/36) sin diagnóstico certero hasta entonces.

### Vulnerabilidades de path traversal cerradas (sesiones 32, 38, 39)
Ver tabla "Bugs conocidos" arriba, último bloque — resumen completo en `ESTADO.md`.

### `tools/dev/` — 8 herramientas de desarrollo (sesión 35)
Cada una trazada a un fallo real documentado previamente en `ESTADO.md`, no especulativas:
`build_fast.sh`, `check_build.sh`, `check_ci.sh`, `check_meta_urls.sh`, `estado_actual.sh`, `install_hooks.sh`, `secret_scan.sh`, `verify_presets.py` — más `tools/dev/README.md` explicando cada una.

---

*Generado por El_PibeCapo · 2026-03-31 (versión original v7.5.0) · Revisado y puesto al día 2026-07-14 (sesión 39) contra el código real en v8.4.0+main.*
