# ⬡ BetelineyLauncher — Changelog

> Historial de versiones desde v7.0.0. Antes hubo 6 versiones sin documentar (1 a 6, sin ceros ni puntos) de las que no quedó registro de contenido.
> Numerado real: `7.0.0 → 7.1.0 → ... → 7.9.0 → 8.0.0 → 8.1.0 → 8.2.0 → 8.3.0 → 8.4.0`. El patch (tercer número) siempre es `0` — nunca se publicó una versión con patch distinto de cero. Entradas que aparecían antes como `7.3.2`, `7.6.1`, `7.8.1`, `7.9.1` y `7.9.2` no fueron releases reales; eran trabajo del mismo ciclo que quedó mal fragmentado en una reconstrucción previa de este documento. Ya están fusionadas en su versión base correspondiente.
> Actualizado: **2026-07-14** · Versión actual en código: **v8.4.0** (tag publicado, pero contiene solo el bump de versión — todo lo de abajo son commits reales en `main` posteriores al tag, sin release nueva todavía). Autor: **El_PibeCapo**

---

## Sin publicar aún — commits en `main` desde `v8.4.0` (2026-07-06 a 2026-07-14)

> Nota: `v8.4.0` se taggeó sin contenido nuevo (solo bump 8.3.0→8.4.0 + limpieza de docs). Todo lo real de esta sección vive en `main` sin tag propio todavía — la próxima release debería llevar estos cambios y bumpear a `v8.5.0` (ver `## VERSIONES` en `ESTADO.md`: nunca reusar `v8.3.0`/`v8.4.0`).

### Seguridad

| Commit | Cambio |
|---|---|
| `c149bb9a0` | **`GDLauncherMigrator`** — path traversal: `shortpath`/`id` de `data.sqlite` se usaban sin sanitizar para construir la ruta de origen, permitiendo lectura arbitraria de archivos del sistema disfrazada de importación de instancia. Fix: `safeChildPath()` valida con `QDir::cleanPath()` que la ruta resultante no escape de `instances/`; nombre de directorio destino rechaza también `..` literal (antes solo bloqueaba caracteres sueltos). |
| `254f05760` | **Feed de meta remoto** (`meta/JsonFormat.cpp`) — `uid`/`version` del feed (`ElPibeCapo/meta`) se usaban sin sanitizar para construir rutas de cache local; un feed comprometido podía inyectar `../` y escapar del directorio esperado. Fix: `isSafePathComponent()`/`requireSafePathComponent()`, rechazan cualquier valor con `..`, `/`, `\`, byte nulo, vacío o `>256` caracteres. |
| `de3717394` | **`mmc-pack.json` local + `Require` compartido** (`PackProfile.cpp`, `JsonFormat.cpp`) — el fix anterior solo cubría el feed remoto. `Component::m_uid` de una instancia local (puede venir de un modpack de terceros importado) permitía borrar/escribir archivos arbitrarios vía `customize()`/`revert()`. Peor: `Meta::Require::uid` (compartido por feed remoto, `mmc-pack.json` y `patches/*.json`) se inyecta automáticamente como componente nuevo en cada resolve/launch de instancia sin interacción del usuario — vector más grave que los otros dos por ser automático. Ambos cerrados con la misma validación. |
| `77e0f40cc` | **Firma Ed25519 fail-closed para actualizaciones** — el updater (`BetelineyUpdater`) ahora descarga y verifica una firma `.sig` con clave pública embebida antes de instalar cualquier update; si falta la firma o no valida, borra el archivo y aborta (no hay "instalar de todos modos"). Usa `libsodium` (nueva dependencia vcpkg). CI firma el release con un secret (`RELEASE_SIGNING_KEY`) antes de publicar. |

### Confiabilidad

| Commit | Cambio |
|---|---|
| `efe33a69e` | **Use-after-free en `BackgroundModUpdateCheckTask`** — `m_instance` era un puntero crudo a `MinecraftInstance`; si el usuario borraba la instancia mientras el chequeo de mods en background seguía en red, el callback final escribía sobre memoria liberada. Fix: `QPointer<MinecraftInstance>` (Qt lo pone en `nullptr` solo al destruirse el objeto) + guard defensivo adicional. |
| `af88e5b88` | **Cuelgue histórico del build local con LTO, causa raíz resuelta** — cada link con `-flto=auto` paralelizaba internamente hasta `nproc()` hilos propios; sin límite, Ninja corría varios links en simultáneo multiplicando el paralelismo muy por encima de cores/RAM disponibles, generando swap thrashing (síntoma arrastrado desde sesiones 20/27/29/31/32/33/36). Fix: `JOB_POOLS lto_link_pool=2`, limita a 2 links simultáneos. |

### Nuevas funciones

| Commit | Cambio |
|---|---|
| `b37308428` | **Backup manual de mundos** — botón "Backup" en la toolbar de `WorldListPage`. Comprime el mundo seleccionado a `backups/worlds/{nombre}_{timestamp}.zip` dentro de la instancia, reusando `BetelineyZip::ExportToZipTask` (mismo mecanismo que `ExportInstanceDialog`). El backup automático antes de actualizar un pack queda pendiente como trabajo aparte. |
| `2ef426dcd` | **Badge de actualizaciones de mods** — chequeo silencioso en background (`ModrinthCheckUpdate`/`FlameCheckUpdate`) al seleccionar una instancia, una vez por sesión del launcher por instancia. Activa `MinecraftInstance::setUpdateAvailable()`, que ya existía en el código pero no tenía disparador (feature fantasma detectada en sesión 26: la UI ya pintaba el badge, solo faltaba quién lo activara). |
| `e46e1f13d` | **Command palette (Ctrl+K) + servidores favoritos con quick-join** — paleta estilo VSCode/Sublime que recorre el menú principal en vivo con filtro de texto; diálogo de alta/edición/baja de servidores favoritos persistidos en `SettingsObject`; acceso directo a unirse a un favorito sin abrir la instancia primero. |
| `17880fbb0` | **Sistema de logros de marca por tiempo jugado** — 5 logros (1h/10h/50h/100h/500h de juego acumulado en una instancia), notificación visual tipo toast (esquina inferior derecha, cola FIFO si se desbloquean varios a la vez), persistencia en `SettingsObject`. |
| `da70d0e6b` | **Sandboxing opcional con Bubblewrap (`bwrap`) para el proceso de Minecraft en Linux** — opción configurable, no forzada por defecto (no todos los sistemas tienen `bwrap`, ni todas las configuraciones de instancia son compatibles con un sandbox estricto). |

---

## v8.3.0 — Ecosistema Beteliney completo (2026-06-18)

### Fase 0 — Estabilización

| # | Tipo | Cambio |
|---|------|--------|
| 1 | `FIX` | **`main.cpp`** — `Q_INIT_RESOURCE(beteliney_icons)` duplicado eliminado |
| 2 | `FIX` | **`CMakeLists.txt`** — `BUILD_TESTING=OFF` por defecto; `CURSEFORGE_API_KEY` leída de `$ENV{}` en vez de hardcodeada |
| 3 | `FIX` | **`buildconfig/BuildConfig.h`** — campo `BETELINEY_PACKS_URL` agregado |
| 4 | `FEAT` | **`.github/workflows/build.yml`** — CI inyecta CurseForge key desde secret, configura `BUILD_ARTIFACT`, body Release con aviso SmartScreen |

### Fase 1 — Motor de diagnóstico de logs

| # | Tipo | Cambio |
|---|------|--------|
| 5 | `FEAT` | **`launcher/logs/BetelineyLogAnalyzer.h/cpp`** (92+639 líneas) — Motor de diagnóstico con 18 checks: OutOfMemory, HeapReservation, DuplicateMod, MissingDependency, IncompatibleMods, MixinConflict, FabricIncompatible, JavaNotFound, UnsupportedJavaVersion, ForgeJavaRequirement, OpenGLNotAccelerated, OpenGLError, NativesCrash, WindowsLoadLibrary, NetworkError, Fractureiser, ForgeEarlyWindow, ForgeCoremods |
| 6 | `FEAT` | **`launcher/ui/pages/instance/LogPage.h/cpp/.ui`** — Panel `diagnosisPanel` integrado: se activa cuando `gameExitCode != 0`, severidad visual por colores (Critical rojo, Error naranja, Warning amarillo, Info cyan), navegación 1/N entre diagnósticos, ActionTargets abren Settings Java / carpeta mods / Modrinth |

### Fase 3 — BetelineyPacks

| # | Tipo | Cambio |
|---|------|--------|
| 7 | `FEAT` | **`launcher/modplatform/beteliney/BetelineyPack.h`** (49 líneas) — Structs: `Pack`, `PackMod`, `PackIndex`, enum `PackProvider` |
| 8 | `FEAT` | **`BetelineyPackListModel.h/cpp`** (52+177 líneas) — Descarga `index.json` + packs individuales desde META server, ordena featured primero, async con señales Qt |
| 9 | `FEAT` | **`BetelineyPackInstallTask.h/cpp`** (42+164 líneas) — `InstanceCreationTask`: crea instancia con loader correcto, descarga mods, verifica SHA-512 |
| 10 | `FEAT` | **`BetelineyPresets.h`** (144 líneas) — 3 presets built-in sin red: Vanilla Optimizado (Sodium+Lithium+Iris+ModernFix), PvP Competitivo, Modpack Pesado NeoForge |
| 11 | `FEAT` | **`launcher/ui/pages/modplatform/beteliney/BetelineyPackPage.h/cpp/.ui`** (59+217+98 líneas) — UI completa: lista izquierda con búsqueda, panel derecho con descripción e iconos async |
| 12 | `FEAT` | **`launcher/ui/dialogs/NewInstanceDialog.cpp`** — `BetelineyPackPage` como primera pestaña en "Nueva instancia" |
| 13 | `FEAT` | **`CMakeLists.txt`** — `BETELINEY_PACKS_URL` apunta a `https://ElPibeCapo.github.io/meta/v1/beteliney-packs/index.json` |
| 14 | `FEAT` | **`buildconfig/BuildConfig.h`** — `LAUNCHER_NEWS_RSS_URL` y `LAUNCHER_NEWS_OPEN_URL` apuntan al META server propio |

### Fase 4 — Features avanzados

| # | Tipo | Cambio |
|---|------|--------|
| 15 | `FEAT` | **`launcher/launch/steps/CheckModConflicts.h/cpp`** (31+91 líneas) — `LaunchStep` pre-lanzamiento: detecta mod IDs duplicados con `ModUtils`, loguea warnings con nombre exacto del conflicto |
| 16 | `FEAT` | **`launcher/minecraft/MinecraftInstance.cpp`** — Hook `CheckModConflicts` después de `ScanModFolders` |
| 17 | `FEAT` | **`launcher/minecraft/mod/MalwareScanner.h/cpp`** (64+86 líneas) — Singleton que descarga lista negra de hashes de malware desde META server, verifica SHA-256/512 de cada mod descargado |
| 18 | `FEAT` | **`launcher/ResourceDownloadTask.cpp`** — Hook `MalwareScanner::isMaliciousSha256/512()` después de cada descarga |
| 19 | `FEAT` | **`launcher/Application.cpp`** — Hook `MalwareScanner::loadIfNeeded()` + `checkAndShowCrashReport()` en `showMainWindow` |
| 20 | `FEAT` | **`launcher/crash/BetelineyPanicHandler.h/cpp`** (28+219 líneas) — Crash reporter: Linux `sigaction`+`backtrace_symbols_fd`, Windows `SetUnhandledExceptionFilter`+`MiniDumpWriteDump`. Al siguiente inicio muestra backtrace con botón "Reportar en GitHub" |
| 21 | `FEAT` | **`launcher/main.cpp`** — Hook `installPanicHandler()` antes de `Application` |
| 22 | `FEAT` | **`launcher/ui/pages/instance/VersionPage.h/cpp`** — Botón "Optimizar (rendimiento)" para instancias Fabric/Quilt: instala Sodium, Lithium, Iris y ModernFix si no están presentes |
| 23 | `FEAT` | **`launcher/migration/GDLauncherMigrator.h/cpp`** (54+309 líneas) — Importador GDLauncher Carbon: abre `data.sqlite`, lee instancias, convierte al formato Prism con metadata completa |
| 24 | `FEAT` | **`launcher/ui/dialogs/GDLauncherMigrateDialog.h/cpp`** (45+170 líneas) — UI del importador: lista con selección múltiple, `QProgressDialog`, acceso en Archivo → menú |
| 25 | `FEAT` | **`launcher/ui/MainWindow.cpp`** — Acción "Importar desde GDLauncher Carbon..." en menú Archivo |

### Fase 5 — Distribución

| # | Tipo | Cambio |
|---|------|--------|
| 26 | `FEAT` | **`packaging/com.beteliney.BetelineyLauncher.json`** (88 líneas) — Manifest Flatpak: runtime KDE 6.6, permisos Wayland+X11+audio+filesystem, build desde fuente |
| 27 | `FEAT` | **`EMPAQUETAR_APPIMAGE.sh`** (103 líneas) — Script AppImage: auto-descarga `linuxdeploy` y plugin Qt, prepara `AppDir` con desktop+icon, genera AppImage con firma |
| 28 | `FEAT` | **`program_info/win_install.nsi.in`** — `MUI_WELCOMEPAGE_TEXT` con instrucciones de bypass SmartScreen paso a paso |
| 29 | `FEAT` | **`.github/workflows/build.yml`** — Body de Release con aviso de SmartScreen y pasos para ejecutar de todas formas |
| 30 | `DOCS` | **`README.md`** — Reescrito completo: features Beteliney, tabla de perfiles, estructura de archivos, diferencias con Prism |
| 31 | `BUILD` | **`CMakeLists.txt`** — Versión bumpeada a **8.3.0** |
| 32 | `DOCS` | **`ESTADO.md`** — Documento único fuente de verdad del proyecto (403 líneas) |
| 33 | `REFACTOR` | **`packaging/com.beteliney.BetelineyLauncher.json`** — Manifest Flatpak movido de `dist/` (gitignorado) a `packaging/` (rastreado por git) — bug: antes nunca se commitea |
| 34 | `CLEANUP` | **`dist/`** — Eliminados tarballs obsoletos v7 y v8 (builds stale, gitignored basura local) |

---

## v8.2.0 — Idioma español + BetelineyTheme v5 (2026-05-23)

### Idioma

| # | Tipo | Cambio |
|---|------|--------|
| 1 | `FIX` | **`prismlauncher.cfg`** — `Language=es` → `es_419`. El launcher busca `mmc_es_419.qm`; `es` solo no matcheaba ningún archivo. |
| 2 | `FIX` | **`build/translations/`** — Copiados `mmc_es.qm` y `mmc_es_419.qm` desde `~/.local/share/PrismLauncher/translations/` (instalación del sistema). Directorio estaba vacío — por eso toda la UI salía en inglés. |

### Tema

| # | Tipo | Cambio |
|---|------|--------|
| 3 | `FIX` | **`prismlauncher.cfg`** — `ApplicationTheme=system` → `beteliney`. El cfg heredado de PrismLauncher sobrescribía el default del código — el tema nunca se aplicaba. |
| 4 | `IMPROVE` | **`BetelineyTheme.cpp` → v5** — Font: `Inter`/`Segoe UI` (no instaladas) → `JetBrains Mono` primero (instalada en CachyOS). |
| 5 | `IMPROVE` | **Nuevo: `InstanceView`** — estilo explícito para el widget de cards de instancias (`background #080912`, sin borde). |
| 6 | `IMPROVE` | **Nuevo: botón Launch** — `QPushButton#launchButton` / `QToolButton#actionLaunchInstance` con borde neón 1.5px y fondo verde oscuro. Botón de acción primaria visualmente destacado. |
| 7 | `IMPROVE` | **Cyan `#00D4FF` más presente** — hover de `QToolButton`, `QTabBar::tab`, y `QListView::item` usan cyan como acento secundario (antes solo verde). |
| 8 | `IMPROVE` | **`instanceToolBar` items** — `QToolButton:checked` con border-left neón 3px + background verde oscuro. Panel lateral de grupos de instancias más legible. |
| 9 | `IMPROVE` | **`newsToolBar` label** — texto del ticker de noticias en gris dim italic 8pt (antes heredaba el color del widget padre). |
| 10 | `IMPROVE` | **Hover items con acento izquierdo** — `QListView::item:hover` y `QTreeView::item:hover` añaden `border-left: 2px solid #00D4FF`. Consistente con el strip verde en selected. |
| 11 | `DOCS` | Tooltip del tema actualizado: `v5 — JetBrains Mono`. Backup `BetelineyTheme.cpp.v4bak` creado. |

---

## v8.1.0 — Fix build CachyOS (CMake 4 + GCC 16 + Java 26) + 100% tests (2026-05-23)

### Build fixes — bloqueos completos resueltos

| # | Tipo | Cambio |
|---|------|--------|
| 1 | `FIX` | **`libraries/launcher/CMakeLists.txt`** — Reemplazado `UseJava` + `add_jar()` por `add_custom_command` con `jar --create -C dir .`. CMake 4.x + Java 26 genera `java_class_filelist` vacío en paths con espacios; Java 26 rechaza crear JAR desde argfile vacío. Fix: `javac --release 8 -d classdir` + `jar --create -C classdir .` (sintaxis Java 9+, sin argfiles). |
| 2 | `FIX` | **`libraries/javacheck/CMakeLists.txt`** — Mismo fix. `JavaCheck.jar` también usaba `UseJava` y fallaba igual. |
| 3 | `FIX` | **`COMPILAR_LINUX.sh`** — `JAVA_HOME` ahora busca `javac` explícitamente. En CachyOS el default apunta al JRE 21 (sin `javac`); el JDK completo es el 26. Fallback a `/usr/lib/jvm/java-26-openjdk`. |
| 4 | `FIX` | **`launcher/CMakeLists.txt`** — Agregado `-Wno-sfinae-incomplete` condicionado a GCC >= 16.0. GCC 16 introduce este warning nuevo que Qt 6.x dispara en `qmetatype.h` con forward declarations de `QObject`. Con `-Werror` bloqueaba toda la compilacion. Mismo patron que `-Wno-dangling-reference` para GCC >= 13. |
| 5 | `FIX` | **`COMPILAR_LINUX.sh deps`** — `vulkan-headers` agregado a deps Arch/CachyOS. Requerido por `HardwareInfo.cpp` (Prism 11) para deteccion de GPU via Vulkan. |

### Bugs reales corregidos (detectados por los 29 tests)

| # | Tipo | Cambio |
|---|------|--------|
| 6 | `FIX` | **`logs/AnonymizeLog.cpp`** — UUID regex: `id` suelto -> `"id"` con comillas JSON. `id` generico matcheaba `seed id=<UUID>` y strippeaba UUIDs de mundos/mods. Test `test_genericUUIDPreserved` pasa. |
| 7 | `FIX` | **`tests/BetelineyGPUDetect_test.cpp`** — Clasificacion GPU: prioridad para iGPU AMD conocidas (Phoenix, Hawk Point, Strix, Raphael). `Phoenix [Radeon RX 7600M XT]` era marcada como discreta por contener `RX 7`. Fix: si la linea de lspci tiene un codename iGPU priority, se omite del check discrete. |
| 8 | `FIX` | **`launcher/BetelineyProfiles.h`** — Nombre y descripcion del perfil `iGPU / RAM compartida` sin referencias a hardware especifico (`Ryzen 3700U`, `Vega 10`). Test `test_NoHardwareSpecificStrings` pasa. |
| 9 | `FIX` | **`tests/testdata/FileSystem/test_folder/`** — Creados `.secret_folder/` y `.secret_folder/.secret_file.txt`. El test `test_copy/link_with_dot_hidden` los requeria pero no existian (git ignora dirs con punto vacios). |

### Resultado final

| Metrica | Valor |
|---|---|
| Binario | `build/beteliney` — 17 MB, ELF 64-bit, dynamically linked |
| Version | BetelineyLauncher 7.9.1-master |
| JARs | NewLaunch.jar (16K), NewLaunchLegacy.jar (25K), JavaCheck.jar (1.1K) |
| Tests | **29/29 — 100% passing** |
| Compilador | GCC 16.1.1, Qt 6.11.1, Java 26.0.1, CMake 4.3.3, Ninja 1.13.2 |
| Sistema | CachyOS Linux (Arch), kernel 7.0.9-1-cachyos, Wayland |

---

## v8.0.0 — Port Prism Launcher 11.0.2 + CachyOS/Arch support (2026-05-23)

### Upstream sync — Prism Launcher 11.0.2

| # | Tipo | Cambio |
|---|------|--------|
| 1 | `FEAT` | **`HardwareInfo.cpp/h`** — nuevo módulo portado de Prism 11: detecta CPU, GPU, RAM total y RAM disponible de forma multiplataforma desde C++. Reemplaza el código `probeProcCpuinfo` / `runLspci` / `runGlxinfo` inline en `PrintInstanceInfo.cpp` (−105 líneas). |
| 2 | `FEAT` | **`LibraryUtils.cpp/h`** — nuevo módulo portado de Prism 11: API unificada para detección de MangoHud y librerías nativas. Reemplaza `MangoHud::findLibrary` / `MangoHud::getLibraryString`. |
| 3 | `FEAT` | **`EnsureAvailableMemory.cpp/h`** — step de lanzamiento portado de Prism 11: muestra advertencia si la RAM disponible es insuficiente para la RAM configurada en la instancia. Activable por instancia via `LowMemWarning`. |
| 4 | `REFACTOR` | **`PrintInstanceInfo.cpp`** — simplificado de 139 a 34 líneas. Toda la detección de hardware ahora pasa por `HardwareInfo`. |
| 5 | `REFACTOR` | **`SysInfo.cpp`** — `suitableMaxMem()` ahora usa `HardwareInfo::totalRamMiB()` en lugar de la función local `getSystemRamMiB()`. |
| 6 | `REFACTOR` | **`Application.cpp`, `JavaSettingsWidget.cpp`, `JavaWizardWidget.cpp`, `FlameInstanceCreationTask.cpp`** — todas las referencias a `SysInfo::getSystemRamMiB()` migradas a `HardwareInfo::totalRamMiB()`. `MangoHud::` → `LibraryUtils::`. |
| 7 | `FEAT` | **`MinecraftInstance.cpp`** — integrado step `EnsureAvailableMemory` en el pipeline de lanzamiento. Override `LowMemWarning` por instancia. |
| 8 | `FIX` | **`BuildConfig`** — agregado campo `LAUNCHER_ENVNAME = "BETELINEY"`. Usado por `HardwareInfo` para la variable de entorno de desactivación. |
| 9 | `IMPROVE` | **`CMakeLists.txt` (raíz)** — `cmake_minimum_required` bumpeado a 3.25. `set(CMAKE_CXX_FLAGS ... -DQT_WARN)` → `add_compile_definitions()` (forma correcta de CMake). |
| 10 | `FIX` | **`COMPILAR_LINUX.sh`** — `-Dtomlplusplus_DIR` ahora se detecta automáticamente según distro (`/usr/lib64` Fedora vs `/usr/lib` Arch/CachyOS). Agrega `zlib-ng-compat` en la lista de deps Arch. |
| 11 | `FIX` | **`lanzar.sh`** — ruta de cfg `~/.local/share/PrismLauncher/prismlauncher.cfg` agregada al array de búsqueda (ruta real en CachyOS). |

---

## v7.9.0 — Mejoras MEJ-5 a MEJ-18 + estandarización + BetelineyTheme v4 + traducciones .ui (2026-04-09 a 2026-04-10)

> Nota: el CHANGELOG anterior fragmentaba este trabajo en tres releases (`v7.9.0`, `v7.9.1`, `v7.9.2`). Nunca existió más de una versión `7.9` — todo esto es el mismo ciclo, documentado en dos días consecutivos.

### Nuevas características

| # | Tipo | Cambio |
|---|------|--------|
| 1 | `FEAT` | **`launcher/BetelineyProfiles.h`** — Perfil "iGPU ZGC (Java 21+)" agregado: `-XX:+UseZGC -XX:ZUncommitDelay=60 -XX:SoftMaxHeapSize=1280m`. 6 → **7 perfiles** totales. |
| 2 | `FEAT` | **`launcher/ui/pages/instance/JavaSettingsWidget.cpp`** — Detección de GraalVM CE: badge neón `#39FF14` + tooltip "GraalVM CE detectado — mayor rendimiento JIT". Perfil ZGC en combobox con advertencia visual (Java 21+). |
| 3 | `FEAT` | **`beteliney-updater.sh`** — Nueva función `check_stale_artifacts()`: detecta binarios en `build/` sin target CMake correspondiente y corre `ninja clean` automáticamente antes de recompilar. |
| 4 | `TOOL` | **`tools/extract_strings.sh`** — Nuevo script: extrae strings traducibles del C++ con `xgettext`, genera `translations/beteliney.pot`. |
| 5 | `TOOL` | **`tools/sign_release.sh`** — Nuevo script: firma GPG + SHA256 de AppImage/.deb para distribución pública. |

### Fixes y documentación

| # | Tipo | Cambio |
|---|------|--------|
| 6 | `FIX` | **`tools/verify.py`** — Nueva función `check_desktop_file()`: verifica existencia de `beteliney.desktop` y presencia de `StartupWMClass`. |
| 7 | `DOCS` | **`README.md`** — Sección `GLFW_PLATFORM=x11` documenta XWayland obligatorio por limitación LWJGL. Fix `G1HeapRegionSize=8M → 1M` con nota explicativa. |
| 8 | `DOCS` | **`docs/PENDIENTES.md`** — Todas las mejoras MEJ-1/18 marcadas `[x]`. Tabla resumen actualizada. Versión → v7.9.0. |
| 9 | `DOCS` | **`docs/SESIONES.md`** — Sesión 17 documentada. Header → v7.9.0 / 17 sesiones. |

### Estandarización de nombres y firmas

| # | Tipo | Cambio |
|---|------|--------|
| 10 | `FIX` | **`lanzar.sh:209`** — Firma `by pibe` → `by El_PibeCapo` |
| 11 | `FIX` | **`beteliney-updater.sh`** — Banner sin línea de cierre `╚══╝` — corregido |
| 12 | `FIX` | **`tools/verify.py`** — Versión hardcodeada `v7.7.0` → `v7.9.0` |
| 13 | `FIX` | **`tools/fix_icons.py`** — Versión hardcodeada `v7.7.0` → `v7.9.0` |
| 14 | `FIX` | **`tools/sign_release.sh`** — Banner: `MEJ-18` eliminado del título, email agregado |
| 15 | `FIX` | **`tools/extract_strings.sh`** — Banner: `MEJ-15` eliminado del título, email agregado |

### BetelineyTheme v4 + traducciones .ui (windowTitle al español, 13 archivos)

| # | Tipo | Cambio |
|---|------|--------|
| 16 | `FEAT` | **`ui/themes/BetelineyTheme.cpp`** — v4: fondo `#080912`, font 10pt, radios aumentados, GroupBox uppercase, segundo acento cyan `#00D4FF` documentado. Backup `BetelineyTheme.cpp.v3bak` creado. |
| 17 | `DOCS` | **`README.md`** — tabla features actualizada a BetelineyTheme v4; badges `labelColor=080912`. |
| 18 | `FEAT` | **`AboutDialog.ui`, `UpdateAvailableDialog.ui`, `NewsDialog.ui`, `CopyInstanceDialog.ui`, `CreateShortcutDialog.ui`, `SkinManageDialog.ui`, `ProgressDialog.ui`, `IconPickerDialog.ui`, `ExportInstanceDialog.ui`, `NewInstanceDialog.ui`** — windowTitle y strings traducidos al español completo |
| 19 | `DOCS` | **`docs/SESIONES.md`** — sesión 20 documentada completa |
| 20 | `DOCS` | **`docs/PENDIENTES.md`** — sesión 20 agregada con tabla de .ui traducidos y estado final |
| 21 | `DOCS` | **`docs/INDICE.md`** — footer → 2026-04-10 |

> ⚠️ Requiere recompilación para aplicar cambios del tema v4 y traducciones .ui.

---

## v7.8.0 — Fixes C++ + análisis runtime + build actualizado + optimización de scripts (2026-04-07 a 2026-04-09)

> Nota: el CHANGELOG anterior fragmentaba este trabajo en `v7.8.0` y una supuesta `v7.8.1`. Nunca existió más de una versión `7.8` — mismo ciclo, dos días de documentación.

| # | Tipo | Cambio |
|---|------|--------|
| 1 | `FIX` | **`launcher/minecraft/MinecraftLoadAndCheck.cpp`** — Fix 2: fallback offline cuando verificación online falla |
| 2 | `FIX` | **`launcher/ui/pages/global/AccountListPage.cpp`** — Fix 5: login No-Premium usa `ProgressDialog` correctamente |
| 3 | `FIX` | **`CMakeLists.txt`** — Versión corregida: `MINOR 3` → `MINOR 7` (binario reporta `7.7.0-master`) |
| 4 | `PERF` | **`BetelineyLauncher/source/COMPILAR_LINUX.sh`** — Flags: `-ftree-vectorize`, `-fno-plt`, `-fomit-frame-pointer`, `-fno-semantic-interposition`; linker: `-Wl,-O1`, `--sort-common`; detección mold/lld |
| 5 | `FIX` | **`libraries/libnbtplusplus/CMakeLists.txt`** — Fix mold+LTO+GCC15: `target_compile_options(nbt++ PRIVATE -fno-lto)` |
| 6 | `FIX` | **`tools/pre_minecraft.sh`** — BUG-A: THP `always` → `madvise`; BUG-B: `sched_migration` 5ms → 2ms; BUG-C: `renice` sin efecto eliminado; BUG-D/E: CPU governor guardado dinámicamente, crash-safe |
| 7 | `FIX` | **`tools/post_minecraft.sh`** — BUG-D: restaura `ORIG_CPU_GOV` dinámicamente en lugar de hardcodear `schedutil` |
| 8 | `FIX` | **`lanzar.sh`** — BUG-F: `RADV_PERFTEST` y `AMD_PREFER_64BIT_BVHBUILD` eliminadas (Vulkan, sin efecto en OpenGL); BUG-G: `MESA_LOADER_DRIVER_OVERRIDE=radeonsi` agregado |
| 9 | `FIX` | **`tools/tune_persistent.sh`** — BUG-H: UDEV NVMe pattern `nvme[0-9]n[0-9]` → `nvme[0-9]*n[0-9]*`; BUG-I: `RADV_PERFTEST` y `AMD_PREFER_64BIT_BVHBUILD` eliminadas del bloque ENVVARS de KDE |

---

## v7.7.0 — Tests de integración bash + traducción adicional + firmas (2026-03-31)

| # | Tipo | Cambio |
|---|------|--------|
| 1 | `TEST` | **`tests/test_jvm_profile_integration.sh`** — 7 tests bash para `apply_jvm_profile_if_needed()` |
| 2 | `FEAT` | **`launcher/ui/pages/instance/InstanceSettingsPage.cpp`** — Traducción al español |
| 3 | `FEAT` | **`launcher/ui/pages/instance/GameOptionsPage.cpp`** — Traducción al español |
| 4 | `FEAT` | **`lanzar.sh`** — Nueva función `show_active_jvm_cfg()`: muestra perfil JVM activo en el banner |
| 5 | `DOCS` | **`COMPILAR_LINUX.sh`** — Instrucciones de pin de commit para `libnbtplusplus` |
| 6 | `DOCS` | Firmas `El_PibeCapo` agregadas en `LEEME.txt`, `verify.py`, `fix_icons.py`, `COMPILAR_LINUX.sh` |

---

## v7.6.0 — Auditoría completa del proyecto + corrección de bugs (2026-03-31)

> Nota: el CHANGELOG anterior fragmentaba este trabajo en `v7.6.0` y una supuesta `v7.6.1`. Nunca existió más de una versión `7.6` — mismo día, mismo ciclo.

| # | Tipo | Cambio |
|---|------|--------|
| 1 | `DOCS` | **`docs/AUDITORIA.md`** — Documento standalone de auditoría creado |
| 2 | `DOCS` | 7 bugs detectados y registrados en `docs/PENDIENTES.md` |
| 3 | `DOCS` | **`source/docs/ARQUITECTURA.md`** — Actualizado a v7.5.0 |
| 4 | `FIX` | **`tools/verify.py`** — BUG-6: nueva función `check_test_files()` — verifica los 5 archivos de test Beteliney |
| 5 | `FIX` | **`COMPILAR_LINUX.sh`** — BUG-7: `ensure_libnbt()` usa `--branch` explícito y variable `LIBNBT_BRANCH` |
| 6 | `DOCS` | **`docs/AUDITORIA.md`** — Tabla de bugs actualizada con estado de corrección |
| 7 | `NOTE` | BUG-1/2/3/4/5 verificados como ya corregidos en código (no requerían cambios adicionales) |

---

## v7.5.0 — Tests unitarios Beteliney (2026-03-31)

| # | Tipo | Cambio |
|---|------|--------|
| 1 | `TEST` | **`tests/BetelineyTime_test.cpp`** — 15 tests para `prettifyDuration` / `humanReadableDuration` |
| 2 | `TEST` | **`tests/BetelineyGPUDetect_test.cpp`** — 18 tests de clasificación GPU (iGPU AMD/Intel, discreta, dual) |
| 3 | `TEST` | **`tests/BetelineyTranslation_test.cpp`** — 15 tests de cobertura de traducción UI |
| 4 | `TEST` | **`tests/CMakeLists.txt`** — 3 nuevos targets con `BETELINEY_SRCDIR` para resolver rutas |

---

## v7.4.0 — Traducción UI completa + detección GPU en lanzar.sh (2026-03-31)

| # | Tipo | Cambio |
|---|------|--------|
| 1 | `FEAT` | **`launcher/ui/pages/instance/ModFolderPage.cpp`** — Traducción completa al español |
| 2 | `FEAT` | **`launcher/ui/pages/instance/ExternalResourcesPage.cpp`** — Traducción completa |
| 3 | `FEAT` | **`launcher/ui/pages/modplatform/ResourcePage.cpp`** — Traducción completa |
| 4 | `FEAT` | **`lanzar.sh`** — `detect_gpu_type()`: clasifica GPU en `igpu_amd`, `igpu_intel`, `discrete`, `unknown` |
| 5 | `FEAT` | **`lanzar.sh`** — `apply_jvm_profile_if_needed()`: aplica perfil iGPU automáticamente; respeta config manual y flag `--no-jvm` |

---

## v7.3.0 — Renombrado MMC → Beteliney + logo + AboutDialog + iGPU + documentación completa (2026-03-31)

> Nota: el CHANGELOG anterior fragmentaba este trabajo en `v7.3.0` y una supuesta `v7.3.2`. Nunca existió más de una versión `7.3` — mismo día, mismo ciclo.

| # | Tipo | Cambio |
|---|------|--------|
| 1 | `REFACTOR` | `MMCZip` → `BetelineyZip`, `MMCTime` → `BetelineyTime`, `MMCIcon` → `BetelineyIcon` — referencias y CMake actualizados |
| 2 | `FEAT` | Logo SVG/PNG/ICO creado desde cero: hexágono ⬡ con B neón `#39FF14` sobre `#0C0E16` |
| 3 | `FEAT` | **`AboutDialog`** — Título, versión neón, hardware objetivo, commit 8 chars, labels en español |
| 4 | `FEAT` | **`launcher/ui/pages/instance/JavaSettingsWidget.cpp`** — Detección automática de iGPU (AMD/Intel) con sugerencia de perfil |
| 5 | `FEAT` | **`launcher/ui/pages/instance/JavaPage.cpp`** — Strings de Java traducidos al español |
| 6 | `DOCS` | **`docs/PERFILES_JVM.md`** — Guía de perfiles con flags G1GC, fuentes, benchmarks |
| 7 | `DOCS` | **`docs/TROUBLESHOOTING.md`** — 245 líneas: errores comunes y soluciones |
| 8 | `DOCS` | **`source/docs/ARQUITECTURA.md`** — Árbol de módulos, flujo de lanzamiento |
| 9 | `DOCS` | **`docs/ESTRUCTURA.md`** — Mapa técnico completo del proyecto |
| 10 | `DOCS` | **`README.md`** — Reescrito: ASCII art, badges, perfiles JVM, tabla de scripts |
| 11 | `FEAT` | **`tools/verify.py`** — Checks: `BetelineyProfiles.h`, `BetelineyTheme.cpp (#39FF14)`, módulos clave |
| 12 | `FEAT` | **`tools/fix_icons.py`** — Flag `--dry-run` agregado |

---

## v7.2.0 — BetelineyTheme v3 completo (2026-03-31)

| # | Tipo | Cambio |
|---|------|--------|
| 1 | `FEAT` | **`ui/themes/BetelineyTheme.cpp`** — Tema completo: deep-space navy + neón `#39FF14`, gradientes en todos los widgets, font stack Inter |
| 2 | `REFACTOR` | Temas sobrantes eliminados: `pe_dark/light/colored/blue`, `multimc/`, `breeze_*`, `flat/`, `flat_white/` |

---

## v7.1.0 — Scripts cross-platform (2026-03-31)

| # | Tipo | Cambio |
|---|------|--------|
| 1 | `FEAT` | **`COMPILAR_LINUX.sh`** — Menú interactivo (build/recompilar/limpiar/deps/AppImage) |
| 2 | `FEAT` | **`COMPILAR_BETELINEY.bat / COMPILAR.ps1`** — Detección dinámica Qt/MinGW en Windows |
| 3 | `FEAT` | **`EMPAQUETAR_LINUX.sh`** — 3 modos: tar.gz portable, AppImage, .deb |
| 4 | `FEAT` | **`lanzar.sh`** — `--debug`, compilación automática en primer lanzamiento, check de actualizaciones en background |
| 5 | `FEAT` | **`beteliney-updater.sh`** — Flags `--check`, `--silent`, `--force`, `--appimage`, `--log` |

---

## v7.0.0 — Fork inicial y branding (2026-03-31)

| # | Tipo | Cambio |
|---|------|--------|
| 1 | `FEAT` | **`launcher/auth/`** — Cuentas No-Premium: `anyAccountIsValid()` acepta `AccountType::Offline` sin verificar MSA |
| 2 | `FEAT` | **`launcher/BetelineyProfiles.h`** — 6 perfiles JVM preconfigurados con flags Aikar (iGPU/Ligero/Balanceado/Pesado/Extremo/Personalizado) |
| 3 | `PERF` | **`CMakeLists.txt`** — Flags de compilación para Ryzen 7 3700U: `-march=znver1 -mtune=znver1 -O3 -ffunction-sections -fdata-sections -Wl,--gc-sections`, LTO activado |
