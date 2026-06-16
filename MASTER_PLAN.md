# MASTER PLAN — BetelineyLauncher

> Documento maestro unificado. Reemplaza y supera al DEVLOG.md.
> Combina: registro histórico + auditoría completa del código + investigación de referencia + plan de mejora.
> Mantener actualizado en cada sesión de desarrollo.

---

## 1. IDENTIDAD DEL PROYECTO

| Campo | Valor |
|---|---|
| **Nombre** | BetelineyLauncher |
| **Versión actual** | v8.2.0 |
| **Base** | Prism Launcher (GPL-3.0), fork extensamente modificado |
| **Autor** | El_PibeCapo (`elpibecapoofficial@gmail.com`) |
| **Repositorio** | https://github.com/ElPibeCapo/BetelineyLauncher |
| **Licencia** | GPL-3.0 |
| **Rama principal** | `main` |
| **META server** | https://ElPibeCapo.github.io/meta/v1/ |

---

## 2. STACK TÉCNICO

| Capa | Tecnología |
|---|---|
| **Lenguaje** | C++20 |
| **UI framework** | Qt 6 (Qt Widgets + QSS) |
| **Build system** | CMake 3.25+, Ninja |
| **CI/CD** | GitHub Actions (Ubuntu 24.04 + MSYS2/MinGW64) |
| **Plataformas** | Linux x86_64, Windows x64 (macOS heredado, sin mantener) |
| **Optimizaciones** | `-O3 -march=znver1 -mtune=znver1` (Linux Release) |
| **Windows toolchain** | MinGW-w64 vía MSYS2, sin MSVC |
| **Hashing** | MD4, MD5, SHA1, SHA256, SHA512, Murmur2 (async) |
| **Networking** | Qt Network (QNetworkAccessManager) |
| **Compresión** | libarchive, zlib, zstd, bz2, lzma, lz4 |
| **Markdown** | libcmark |
| **QR codes** | libqrencode |
| **Auth** | MSA Device Code Flow → Xbox Live → XSTS → Minecraft token |
| **Instalador Windows** | NSIS (win_install.nsi.in) |
| **Updater** | BetelineyUpdater (GitHub Releases API, diferencial) |

---

## 3. AUDITORÍA COMPLETA DEL CÓDIGO

### 3.1 LO QUE EXISTE Y FUNCIONA

**Sistema visual — BetelineyTheme (DESTACADO)**
- `launcher/ui/themes/BetelineyTheme.cpp` — 810 líneas de QSS personalizado
- Paleta: deep-space `#080912` / neón `#39FF14` / cyan `#00D4FF`
- Fuente: JetBrains Mono (fallback: Noto Sans)
- 5 iteraciones confirmadas (backups `.v3bak`, `.v4bak`)
- Todos los widgets cubiertos: toolbar, buttons, inputs, lists, tabs, scrollbars, menus, combobox, checkboxes, progress bars, sliders, dialogs, dock widgets, wizard, rubber band
- Cards de instancias con `drawGamerCard()` — bordes neón, rounded corners, hover cyan, selected verde con QPainter nativo
- Botón Launch con acento fuerte diferenciado
- Otros temas heredados: BrightTheme, DarkTheme, FusionTheme, CatPack, CustomTheme, SystemTheme
- `AppearancePage` completamente funcional (header-only, delega a AppearanceWidget)

**Perfiles JVM — BetelineyProfiles (DESTACADO)**
- `launcher/BetelineyProfiles.h` — 7 perfiles documentados técnicamente
- `iGPU / RAM compartida` (384–1536 MB): GCTimeRatio=99, heap pequeño, libera RAM agresivo al OS
- `iGPU ZGC (Java 21+)` (384–1536 MB): ZGC generacional, SoftMaxHeapSize=1280m, pausas <1ms
- `Ligero Vanilla` (512–2048 MB): Aikar's flags calibrados para heap pequeño, sin UseStringDeduplication (no vale en <2GB)
- `Balanceado` (2048–4096 MB): G1HeapRegionSize=2M (2048 regiones óptimo), UseStringDeduplication activado
- `Pesado 100–300 mods` (2048–6144 MB): G1HeapRegionSize=8M, AlwaysPreTouch activado
- `Extremo ≥300 mods` (6144–12288 MB): G1HeapRegionSize=16M, MaxGCPauseMillis=200
- Todos incluyen: G1GC, ParallelRefProcEnabled, DisableExplicitGC, PerfDisableSharedMem, MaxTenuringThreshold=1, GCTimeRatio=99, AutoBoxCacheMax=20000, UseTransparentHugePages (donde aplica)
- Superan en calibración a Prism Launcher y GDLauncher Carbon
- **Estado**: código existe, pendiente verificar conexión a UI en JavaPage

**Auto-updater — BetelineyUpdater (COMPLETO)**
- `launcher/updater/betelineyupdater/BetelineyUpdater.h/cpp` — descarga releases de GitHub API
- Compara versión actual vs latest release por semver
- Soporta: pre-releases, canal beta, AppImage update nativo, backup antes de actualizar
- Descarga diferencial del asset seleccionado
- Log de actualizaciones a archivo dedicado
- `BetelineyExternalUpdater`: timer de auto-check con intervalo configurable, flag betaAllowed
- Frontend integrado en Application.cpp
- `GitHubRelease.h/cpp`: parser del JSON de GitHub Releases API
- `UpdaterDialogs.h/cpp` + `SelectReleaseDialog.ui`: UI para confirmar/seleccionar actualización

**Instalador Windows — NSIS (COMPLETO)**
- `program_info/win_install.nsi.in` — 516 líneas
- Páginas: Welcome, Components, Directory, Install, Finish + Uninstall
- 60+ idiomas incluyendo Spanish e SpanishInternational
- URL handlers: `curseforge://`, `beteliney://`, `beteliney-import://`
- Asociación de archivos: `.mrpack` (default), `.zip` (open-with)
- Start Menu shortcut (obligatorio), Desktop shortcut (opcional), Shell association (opcional)
- Desinstalador completo con limpieza de registro
- Versión embebida en metadata del exe (`VIProductVersion`)
- Placeholder para MSVC Redistributable (`@Launcher_MSVC_Redist_NSIS_Section@`)

**Cuentas offline**
- `AccountData.h`: `enum class AccountType { MSA, Offline }` — ambas implementadas
- `AccountState`: Unchecked, Offline, Working, Online, Disabled, Errored, Expired, Gone
- Soporte offline heredado y funcional

**Auth MSA completa**
- Cadena completa: `MSAStep` → `MSADeviceCodeStep` → `XboxUserStep` → `XboxAuthorizationStep` → `XboxProfileStep` → `EntitlementsStep` → `MinecraftProfileStep` → `GetSkinStep`
- Device Code Flow implementado (el usuario no ingresa credenciales en el launcher)
- Token storage con campos: `issueInstant`, `notAfter`, `token`, `refresh_token`, `validity`
- Skin y Cape descargados y cacheados

**Plataformas de mods — cobertura completa**
- Modrinth: `ModrinthAPI`, `ModrinthCheckUpdate`, `ModrinthInstanceCreationTask`, `ModrinthPackExportTask`, `ModrinthPackIndex`
- CurseForge/Flame: `FlameAPI`, `FlameCheckUpdate`, `FlameInstanceCreationTask`, `FlameFileResolvingTask`, `PackManifest`
- ATLauncher, FTB, Legacy FTB, Technic, PackWiz
- `HashUtils`: MD4, MD5, SHA1, SHA256, SHA512, Murmur2 async — CurseForge usa Murmur2

**Log parser**
- `logs/LogParser.cpp` — parser XML log4j completo + texto plano legacy
- Parsea el formato `<log4j:Event>` de Minecraft moderno
- `guessLevel()` con regex para formatos viejos de Forge (`[INFO]`, `[SEVERE]`, etc.)
- Log coloreado por nivel integrado en `LogPage.cpp` con colores del tema activo
- `anonymizeLog()` — elimina tokens, UUIDs, IPs antes de compartir logs

**Java auto-descarga (infraestructura)**
- `java/download/`: `ArchiveDownloadTask`, `ManifestDownloadTask`, `SymlinkTask`
- `java/JavaMetadata.h`: tipos `Manifest` y `Archive`, vendor, URL, checksum, versión, OS
- `JavaPage.cpp`: conecta `JavaInstallList` cuando `BuildConfig.JAVA_DOWNLOADER_ENABLED == true`
- `ui/java/InstallJavaDialog` — diálogo de instalación de Java gestionado
- **Estado**: infraestructura completa, verificar que `Launcher_ENABLE_JAVA_DOWNLOADER` está activado en CMakeLists

**Server ping de Minecraft**
- `ui/pages/instance/McClient.h/cpp` — implementación directa del protocolo Minecraft sobre TCP
- `McResolver.h/cpp` — resolución de hostname + SRV records
- Obtiene: MOTD, versión, jugadores online/max, latencia
- Alimenta `ServersPage.cpp` en la UI de instancias

**Setup wizard completo**
- `AutoJavaWizardPage` — detecta/instala Java en primera ejecución
- `JavaWizardPage` — configuración manual de Java
- `LanguageWizardPage` — selección de idioma
- `LoginWizardPage` — primer login MSA
- `ThemeWizardPage` — selección de tema inicial
- `PasteWizardPage` — configuración de paste service

**Build system avanzado**
- C++20, `/GS /EHsc` MSVC, `-fstack-protector-strong` GCC
- MSVC Release: `/GL /Gw /Gy /guard:cf` (whole program optimization + control flow guard)
- Linux: `-O3 -D_FORTIFY_SOURCE=2`
- Windows MinGW: `-O3 -march=znver1 -mtune=znver1 -ffunction-sections -fdata-sections` + `--gc-sections`
- Address Sanitizer opcional (`-DDEBUG_ADDRESS_SANITIZER=ON`)
- `CMAKE_EXPORT_COMPILE_COMMANDS` activo para LSP/clangd

**CI/CD (GitHub Actions)**
- `build-linux`: Ubuntu 24.04, Qt6 vía apt, cmark fix CMake 4.x en Python, stub gamemode, Ninja
- `build-windows`: MSYS2 MinGW64, Qt6 vía repos MSYS2 (sin aqtinstall), empaquetado con 7z
- `release`: se dispara solo en tags `v*.*.*`, descarga ambos artefactos y crea GitHub Release
- Artefactos: `BetelineyLauncher-{VER}-Linux-x86_64.tar.gz`, `BetelineyLauncher-{VER}-Windows-x64.zip`

**Easter egg**
- `BetelineyCode.h` (extiende KonamiCode): escribir B-E-T-E en MainWindow dispara señal `triggered()`

**API keys configuradas**
- CurseForge: key propia en CMakeLists.txt
- Microsoft Azure: App ID `4b945c78-d30b-489e-915f-b361bf9c933b`
- Imgur: vacío (registrar si se activa screenshots)

**META server**
- Fork de PrismLauncher/meta → `github.com/ElPibeCapo/meta`
- Genera JSONs de Mojang, Fabric, Forge, NeoForge, Quilt, Java cada 6h via GitHub Actions
- Despliega a GitHub Pages → `https://ElPibeCapo.github.io/meta/v1/`

---

### 3.2 LO QUE EXISTE PERO INCOMPLETO O DESCONECTADO

**LogParser → sin motor de diagnóstico**
- El parser XML log4j funciona y colorea por nivel
- `anonymizeLog()` existe para compartir logs seguros
- **Falta**: motor que detecte errores conocidos (`OutOfMemoryError`, `Duplicate mod ID`, `Mixin transformation failed`, `Failed to start the minecraft runtime`, `Could not reserve enough space for object heap`, etc.) y muestre al usuario un mensaje legible + acción sugerida

**NewsChecker → apunta a GitHub Releases atom**
- `NewsChecker.h` lee un RSS/Atom genérico
- `NEWS_RSS_URL` configurado a `https://github.com/ElPibeCapo/BetelineyLauncher/releases.atom`
- **Falta**: feed de noticias propio (no releases de GitHub), integración visual más rica en MainWindow

**BetelineyProfiles → pendiente verificar conexión a UI**
- El struct `BETELINEY_PROFILES` existe con 7 perfiles completos
- `JavaPage.cpp` muestra Java, pero no hay evidencia de un selector de perfiles en la UI
- **Falta**: conectar `BETELINEY_PROFILES` al selector de argumentos JVM en la página de Java o en InstanceSettings, con descripción visible y auto-apply de Xmx

**JAVA_DOWNLOADER_ENABLED → verificar en CMakeLists**
- `JavaPage.cpp` usa `BuildConfig.JAVA_DOWNLOADER_ENABLED`
- La infraestructura de descarga existe
- **Falta**: verificar que `-DLauncher_ENABLE_JAVA_DOWNLOADER=ON` está en CMakeLists.txt y que el META server incluye los JRE manifests correctamente

**Screenshots / Imgur**
- `ImgurUpload.cpp/h` y `ImgurAlbumCreation.cpp/h` existen
- `IMGUR_CLIENT_ID` vacío
- **Falta**: registrar key o desactivar el feature en UI si no se va a usar

---

### 3.3 LO QUE NO EXISTE AÚN

| Feature | Prioridad | Notas |
|---|---|---|
| **Motor de diagnóstico de logs** | CRÍTICA | El mayor salto cualitativo posible |
| **BetelineyPacks** | ALTA | Plataforma propia de modpacks |
| **Selector de perfiles JVM en UI** | ALTA | El struct existe, falta el widget |
| **Verificación de integridad de mods** | MEDIA | Hash check antes de instalar, base de hashes maliciosos |
| **Perfiles de rendimiento por tipo de juego** | MEDIA | "Modo PvP", "Modo Modpacks", "Modo Vanilla" |
| **Sincronización en la nube de instancias** | BAJA | GDLauncher Carbon lo tiene, complejo de implementar |
| **Inyección automática de mods de rendimiento** | MEDIA | Sodium+Lithium+Starlight en instancias Fabric/Quilt |
| **Crash reporter del launcher mismo** | MEDIA | Sentry o equivalente, opt-in |
| **Soporte macOS activo** | BAJA | El código existe (heredado), sin CI ni mantenimiento |
| **SQLite** | BAJA | INI files son suficientes por ahora |

---

## 4. REGISTRO HISTÓRICO (DEVLOG)

### Sesión 1 — Junio 2026 (Setup inicial)

**Compilación cruzada Windows desde Linux**
- Configurado en CachyOS: mingw-w64-gcc 16.1.0, Qt 6.9.3 Windows (MinGW 64-bit) vía aqtinstall
- Problemas resueltos: CMake 4.x IMPORTED_IMPLIB, cmark config, ECM_DIR, incompatibilidad Qt host/target, headers Windows case-insensitive, stub argc, CopyJars CMake 4.x, DLLs faltantes

**Paquetes generados:**
- `BetelineyLauncher-8-Linux-x86_64.tar.gz` (51 MB) → `/home/pibe/Descargas/Linux - Beteliney [Minecraft]/`
- `BetelineyLauncher-Windows-x64.zip` (29 MB) → `/home/pibe/Descargas/Windows - Beteliney [Minecraft]/`

**Limpieza de rastros Prism:**
- CMakeLists.txt: todas las URLs, API keys, META_URL
- `resources/multimc/` → `resources/beteliney-icons/`
- `multimc.qrc` → `beteliney-icons.qrc`
- Application.cpp: rutas de íconos
- credits.html, metainfo.xml.in, README.md
- LocalResourceUpdateTask.cpp: comentario interno

**URLs configuradas en CMakeLists.txt:**
```
META_URL         = https://ElPibeCapo.github.io/meta/v1/
NEWS_RSS_URL     = https://github.com/ElPibeCapo/BetelineyLauncher/releases.atom
NEWS_OPEN_URL    = https://github.com/ElPibeCapo/BetelineyLauncher/releases
WIKI_URL         = https://github.com/ElPibeCapo/BetelineyLauncher/wiki
HELP_URL         = https://github.com/ElPibeCapo/BetelineyLauncher/issues
BUG_TRACKER_URL  = https://github.com/ElPibeCapo/BetelineyLauncher/issues
UPDATER_GITHUB_REPO = https://github.com/ElPibeCapo/BetelineyLauncher
LEGACY_FMLLIBS_BASE_URL = github.com/ElPibeCapo/BetelineyLauncher/releases/download/fmllibs/
MSA_CLIENT_ID    = 4b945c78-d30b-489e-915f-b361bf9c933b
CURSEFORGE_API_KEY = (key propia)
IMGUR_CLIENT_ID  = (vacío)
```

### CI/CD — 16 iteraciones

| # | Fix aplicado |
|---|---|
| 1 | Ubuntu 22.04 → 24.04 (libtomlplusplus-dev no existe en 22.04) |
| 2 | libgamemode-dev: intentar instalar + stub pkg-config si falla |
| 3 | `-DBUILD_TESTING=OFF` (ECMAddTests.cmake:109 falla en CMake 4.x) |
| 4 | `add_custom_command(TARGET)` → `add_custom_target(CopyJars ALL DEPENDS)` (CMake 4.x cross-subdir) |
| 5 | `Q_INIT_RESOURCE(multimc)` → `Q_INIT_RESOURCE(beteliney_icons)` |
| 6-16 | Windows: jurplel/install-qt-action todos los mirrors de Qt → MSYS2/MinGW64 |

**Estado del CI al cierre de sesión 1:** fix 5 (Q_INIT_RESOURCE) pendiente de confirmar en CI

### Commits del repositorio

```
2915f18  BetelineyLauncher v8.2.0 — commit inicial
9828ea0  ci: GitHub Actions Linux + Windows (primer intento)
e894a01  config: META_URL propio, URLs limpias
ae0ad3e  docs: DEVLOG
e893d8d  ci: fix Ubuntu 24.04 + Qt 6.8.3
2f14402  ci: fix tomlplusplus + setup-java continue-on-error
488ad1e  ci: fix cmark cmake4 + Qt 6.7.3
d46651c  ci: fix sintaxis cmark-config-version
f67ed07  ci: cmark robusto con python + install-qt-action v3
c9b6fae  ci: gamemode linux + Qt 6.5.3
32a495d  ci: cmark python + aqtinstall directo
beccde7  ci: apt separado + Qt 6.6.3
a424fb3  ci: msys2+mingw64 en windows + universe en linux
255dfe5  ci: stub gamemode + quitar java-openjdk-devel
a9a98cb  fix: add_custom_command -> add_custom_target (CMake 4.x)
0b4b519  ci: BUILD_TESTING=OFF + verbose ninja
16bd9bf  fix: Q_INIT_RESOURCE(multimc) -> beteliney_icons
```

### Cómo hacer una release

```bash
cd "/home/pibe/Descargas/Beteliney Launcher [Minecraft]/BetelineyLauncher/source"
git add -A
git commit -m "descripción"
git tag v8.3.0
git push && git push --tags
```
El CI compila Linux + Windows y publica la release en ~15 minutos automáticamente.

---

## 5. REFERENCIA TÉCNICA DE COMPETIDORES

Basada en investigación de Manus + verificación propia.

### Lo mejor de cada launcher

| Launcher | Stack | Lo que hace mejor |
|---|---|---|
| **Prism** | C++/Qt6 | Gestión de instancias con symlinks, META server propio, control total |
| **GDLauncher Carbon** | Rust+SolidJS+Electron | UI/UX, auto-detección y descarga de Java, sync en nube |
| **HMCL** | Java/JavaFX | Velocidad de instalación, parcheo on-the-fly para ARM/RISC-V |
| **ATLauncher** | Java | Mods opcionales en modpacks, máxima flexibilidad |
| **Modrinth App** | Tauri/Rust | Integración nativa .mrpack, ecosistema unificado |
| **Lunar Client** | Propietario | Optimización FPS (Turbo Entities), input lag mínimo |
| **Badlion** | Propietario | Mods integrados para Hypixel, interfaz in-game |

### Técnicas clave verificadas

**Symlinks (Prism):** activos en BetelineyLauncher (heredado). Instancias comparten assets y librerías. 50 instancias de 1.21.x usan el espacio de una sola más sus mods específicos.

**Resolución de dependencias modpacks:**
- `.mrpack` (Modrinth): `index.json` incluye URLs directas + hashes SHA1/SHA512 → descarga paralela directa, sin API calls extra
- CurseForge ZIP: `manifest.json` solo tiene IDs → requiere llamada a API por cada archivo → más lento, cuello de botella
- HashUtils.h tiene Murmur2 para CurseForge y SHA512 para Modrinth

**Auth MSA (Device Code Flow):** el usuario escanea un QR o visita una URL, no ingresa contraseña en el launcher. Más seguro. Ya implementado.

**Forge processors:** `install_profile.json` en Forge 1.20+ incluye `processors` — JARs que deben ejecutarse en cadena para generar el entorno de ejecución. Un launcher que no los ejecuta no puede instalar Forge correctamente. Prism (y por tanto BetelineyLauncher) los ejecuta correctamente.

**Aikar's Flags:** ya implementados en `BetelineyProfiles.h` con calibración superior a cualquier otro launcher open source documentado.

**Detección de conflictos de mods:**
- Fabric: leer `fabric.mod.json` → campo `id`
- Forge: leer `META-INF/mods.toml` → campo `modId`
- No hay librería universal. Conflictos Mixin son solo detectables en runtime.

---

## 6. PLAN DE MEJORA POR FASES

### FASE 0 — Estabilización (INMEDIATO)

Objetivo: dejar el proyecto en estado limpio y verificado antes de añadir features.

**0.1 Verificar CI**
- Confirmar que el fix `Q_INIT_RESOURCE(beteliney_icons)` pasa en GitHub Actions
- Verificar que `build-linux` y `build-windows` generan artefactos correctos
- Verificar que el job `release` funciona al crear un tag

**0.2 Verificar META server**
- Confirmar que `https://ElPibeCapo.github.io/meta/v1/` sirve los JSONs correctamente
- Probar que el launcher compilado los consume (listar versiones de Minecraft)
- Si el workflow del fork no corrió: dispararl manualmente

**0.3 Verificar JAVA_DOWNLOADER_ENABLED**
- Buscar en `CMakeLists.txt` si `-DLauncher_ENABLE_JAVA_DOWNLOADER` está activado
- Si no: añadirlo y verificar que la pestaña de Java gestionado aparece en Settings

**0.4 Compilación local limpia**
- Recompilar localmente con todos los cambios acumulados
- Verificar que el launcher inicia, lista versiones de Minecraft, puede crear instancia vanilla

**0.5 Imgur**
- Decidir: ¿se activa? Si no → ocultar el botón de screenshot en la UI o mostrar mensaje "próximamente"
- Si sí → registrar en `api.imgur.com/oauth2/addclient` y añadir key

---

### FASE 1 — Motor de diagnóstico de logs (MAYOR IMPACTO)

Objetivo: que cuando el juego crashe, BetelineyLauncher le diga al usuario exactamente qué pasó y cómo arreglarlo. Ningún otro launcher open source hace esto bien.

**Implementación: `BetelineyLogAnalyzer`**

Crear `launcher/logs/LogAnalyzer.h/cpp`.

Estructura base:
```cpp
struct LogDiagnosis {
    QString errorCode;      // identificador interno ("OOM", "DUPLICATE_MOD", etc.)
    QString title;          // "Sin memoria suficiente"
    QString explanation;    // qué pasó en lenguaje humano
    QString solution;       // qué hacer exactamente
    QString affectedMod;    // mod que causó el problema (si aplica)
    bool isCritical;        // si el juego no puede iniciar por esto
};

class LogAnalyzer {
public:
    static QList<LogDiagnosis> analyze(const QString& fullLog);
};
```

**Errores a detectar (primera iteración):**

| Error en log | Diagnóstico | Solución sugerida |
|---|---|---|
| `java.lang.OutOfMemoryError` | Sin memoria suficiente | Aumentar RAM en Settings → Java → Max Memory |
| `Could not reserve enough space for` | RAM insuficiente para el heap | Cerrar aplicaciones + reducir Xmx |
| `Duplicate mod` / `duplicate mod id` | Dos mods con el mismo ID | Eliminar uno de los mods duplicados |
| `Mixin transformation failed` | Conflicto de Mixin entre mods | Actualizar todos los mods o eliminar el conflictivo |
| `Failed to start the minecraft runtime` | Java no encontrado o corrompido | Reinstalar Java gestionado desde Settings |
| `Pixel format not accelerated` | Driver de GPU no soporta OpenGL | Actualizar drivers de GPU |
| `UnsupportedClassVersionError` | Java demasiado viejo para esta versión de MC | Instalar Java correcto (17/21) desde Settings |
| `LoadLibrary failed` (Windows) | DLL faltante | Reinstalar launcher o instalar Visual C++ Redist |
| `Forge requires java` | Java incorrecto para Forge | Cambiar versión de Java en la instancia |
| `Mod X requires Mod Y` | Dependencia faltante | Instalar mod Y desde Modrinth/CurseForge |
| `OpenGL error` | Problema de GPU/drivers | Añadir `-Dfml.earlyprogresswindow=false` en JVM args |
| `EXCEPTION_ACCESS_VIOLATION` (Windows) | Crash de memoria en la JVM | Cambiar build de Java (Adoptium/Azul) |

**Integración en LogPage:**
- Después de que el proceso Java termina con código != 0, ejecutar `LogAnalyzer::analyze(fullLog)`
- Mostrar los diagnósticos en un banner expandible sobre el log con título, explicación y botón de acción
- Botón "Copiar log anonimizado" visible siempre (usa el `anonymizeLog()` existente)

---

### FASE 2 — Selector de perfiles JVM en UI

Objetivo: exponer `BetelineyProfiles.h` en la interfaz para que el usuario los aplique con un clic.

**Implementación:**

En `InstanceSettingsPage` (configuración de instancia) y en `JavaPage` (configuración global):
- ComboBox "Perfil de rendimiento:" con los 7 perfiles de `BETELINEY_PROFILES`
- Al seleccionar: auto-rellenar Xmx, Xms, y los JVM args del perfil
- Descripción del perfil visible debajo del combo
- Advertencia si Xmx supera el 60% de RAM disponible del sistema
- Detectar RAM del sistema con `HardwareInfo` (ya existe) para sugerir el perfil adecuado automáticamente

---

### FASE 3 — Ecosistema Beteliney

Objetivo: features propios que no dependen de Prism upstream.

**3.1 BetelineyPacks**
Plataforma propia de modpacks, servida desde el META server o GitHub Pages.

Formato: JSON simple
```json
{
  "id": "beteliney-survival",
  "name": "Beteliney Survival",
  "description": "...",
  "version": "1.0.0",
  "minecraft": "1.21.1",
  "loader": "fabric",
  "loader_version": "0.16.0",
  "mods": [
    { "provider": "modrinth", "id": "AANobbMI", "version": "mc1.21.1-0.6.4+build.5" }
  ]
}
```

Integración en UI: nueva pestaña en el diálogo de nueva instancia "BetelineyPacks" con lista de modpacks, descripción, screenshot, botón instalar.

**3.2 News feed propio**
- Crear un RSS/Atom propio en GitHub Pages (generado estáticamente, sin servidor)
- Formato más rico que el atom de releases: noticias con imagen, categoría, texto completo
- Mostrar en MainWindow con más información visual que el ticker actual

**3.3 Perfil de instancia: tipo de juego**
Presets de configuración completa por tipo:
- `Vanilla Optimizado`: Sodium + Lithium + Starlight auto-instalados en Fabric
- `PvP Competitivo`: JVM flags agresivos + mods PvP esenciales
- `Modpack Pesado`: perfil JVM "Pesado" + advertencias de RAM
- `Servidor Local`: perfil JVM "Extremo" + puertos configurados

---

### FASE 4 — Features avanzados

**4.1 Verificación de integridad de mods**
- Antes de instalar cualquier mod: verificar su hash contra la API de Modrinth/CurseForge
- Base de hashes maliciosos conocidos (Fractureiser y posteriores) como JSON en el META server
- Si el hash no coincide o está en la lista negra: advertencia bloqueante con explicación

**4.2 Detección de conflictos de mods pre-lanzamiento**
- Leer `fabric.mod.json` / `META-INF/mods.toml` de todos los mods de la instancia
- Detectar IDs duplicados antes de lanzar el juego
- Mostrar qué mods conflictúan con exactamente qué otro
- No bloquear el lanzamiento pero sí advertir claramente

**4.3 Crash reporter opt-in del launcher**
- Si BetelineyLauncher mismo crashea: capturar stack trace, versión, OS, y enviarlo (con consentimiento)
- Opción en Settings: "Enviar reportes de error anónimos"
- Destino: GitHub Issues automáticos o webhook simple

**4.4 Migración desde otros launchers**
- Importar instancias desde Prism (ya funciona, heredado)
- Importar instancias desde GDLauncher Carbon (formato propio, requiere implementación)
- Wizard guiado de migración en primera ejecución

---

### FASE 5 — Distribución profesional

**5.1 Firma de código Windows**
- Sin firma: SmartScreen bloquea el instalador en Windows 10/11
- Solución actual: el usuario tiene que "Más info → Ejecutar de todas formas"
- Solución ideal: certificado EV (requiere empresa) o certificado básico (~$100-200/año)
- Solución gratuita temporal: documentar el proceso en pantalla dentro del instalador mismo

**5.2 Linux: Flatpak oficial**
- Crear `com.beteliney.BetelineyLauncher.json` para Flatpak
- Subir a Flathub (requiere revisión del equipo de Flathub)
- Alternativa más rápida: distribuir `.flatpakref` directo desde GitHub Pages

**5.3 Gestión de capacidad del META server**
- GitHub Pages: 100GB/mes de ancho de banda, límite de 1GB de tamaño de repo
- Para el tamaño actual del proyecto: suficiente por años
- Monitorear: si el launcher crece en usuarios, migrar meta a Cloudflare Pages (gratis, sin límites de ancho de banda)
- Las API keys de CurseForge y Azure tienen límites de rate — documentar y monitorear

**5.4 macOS**
- El código existe (heredado de Prism): Sparkle updater, security bookmarks, sandbox entitlements
- Sin CI, sin mantenimiento activo
- Para activarlo: añadir job `build-macos` en CI, resolver notarización de Apple

---

## 7. DECISIONES TÉCNICAS TOMADAS

| Decisión | Alternativa rechazada | Razón |
|---|---|---|
| Continuar con C++/Qt6 (fork de Prism) | Reescritura en Rust+Tauri desde cero | El 90% del trabajo duro ya existe y funciona. Reimplementar auth MSA + Forge processors + todas las plataformas de mods = meses de trabajo con alto riesgo de no terminar |
| QSS custom en Qt Widgets | Migrar a Qt Quick (QML) | Migrar 100+ vistas de Qt Widgets a QML es reescribir la UI entera. El QSS actual es production-quality y produce resultados excelentes con menos riesgo |
| GitHub Pages para META server | VPS propio | Gratis, auto-deploy, suficiente para el volumen actual. Migrar a Cloudflare Pages si se superan los límites |
| MSYS2/MinGW64 para Windows CI | aqtinstall / Qt Online Installer | aqtinstall falló en todos los mirrors disponibles. MSYS2 es más confiable y los repos están mantenidos activamente |
| NSIS como instalador Windows | Inno Setup / WiX | NSIS ya estaba implementado, funciona, tiene soporte multi-idioma completo |
| INI files para settings | SQLite | El volumen de datos de configuración no lo justifica todavía. SQLite cuando se necesite persistir historial de mods, caché de búsquedas, etc. |

---

## 8. PENDIENTES POR SESIÓN

### Pendientes de sesión 1 (VERIFICAR PRIMERO)

- [x] Fix `Q_INIT_RESOURCE(beteliney_icons)` duplicado en `main.cpp` — APLICADO + PUSHEADO
- [x] `BUILD_TESTING=OFF` como default en CMakeLists — APLICADO + PUSHEADO
- [x] API key CurseForge sacada del código — ahora lee `$ENV{CURSEFORGE_API_KEY}` — APLICADO + PUSHEADO
- [x] CI inyecta key desde `secrets.CURSEFORGE_API_KEY` — APLICADO + PUSHEADO
- [x] `Launcher_BUILD_ARTIFACT` configurado en CI (`"Linux-x86_64"` / `"Windows-x64"`) — APLICADO + PUSHEADO (activa el auto-updater en builds de CI)
- [x] META server verificado — rama `gh-pages` tiene TODO el contenido (net.minecraft, Forge, NeoForge, Fabric, Quilt, Java Adoptium/Azul/IBM). Workflows corren correctamente cada 6h. Último run: 2026-06-16 05:05 UTC exitoso.
- [ ] **ACCIÓN REQUERIDA #1**: Añadir secret `CURSEFORGE_API_KEY` en GitHub → Settings → Secrets → Actions → New repository secret. Valor: `$2a$10$wIJoeapbxkZ1tE8h2S/ojOLkDwqdEjAq9ZXqAUwFKlRUcZYx5iLsO`
- [ ] **ACCIÓN REQUERIDA #2**: Activar GitHub Pages en el repo meta → https://github.com/ElPibeCapo/meta/settings/pages → Source: "Deploy from a branch" → Branch: `gh-pages` → `/ (root)` → Save. El contenido está listo, solo falta habilitarlo. URL resultante: `https://elpibecapo.github.io/meta/v1/`
- [ ] Verificar que el CI del launcher pase tras el push (commit ae1ddd6)
- [ ] Compilación local limpia y prueba de inicio del launcher
- [ ] Decidir sobre Imgur

### Próximo a implementar

- [ ] FASE 1: `BetelineyLogAnalyzer` — motor de diagnóstico de crashes
- [ ] FASE 2: Selector de perfiles JVM en JavaPage e InstanceSettingsPage

---

## 9. ESTRUCTURA DE VERSIONES

| Versión | Criterio |
|---|---|
| **Patch** (x.x.+1) | Bug fixes, cambios menores de UI, actualización de dependencias |
| **Minor** (x.+1.0) | Feature nuevo completo (ej: LogAnalyzer, selector de perfiles) |
| **Major** (+1.0.0) | Cambio arquitectural, nueva fase completa, reescritura de subsistema |

Versión actual: v8.2.0
Próxima patch: v8.2.1 (cuando CI confirme pase)
Próxima minor: v8.3.0 (LogAnalyzer + selector de perfiles JVM)
