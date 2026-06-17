# ESTADO — BetelineyLauncher
> Documento único, autocontenido. Cualquier chat nuevo lee SOLO esto y puede continuar.
> Actualizar al cerrar cada sesión. Última actualización: sesión 3.

---

## IDENTIDAD

| | |
|---|---|
| **Nombre** | BetelineyLauncher |
| **Versión actual** | v8.2.0 |
| **Base** | Prism Launcher (GPL-3.0) — fork extensamente modificado |
| **Autor** | El_PibeCapo — `elpibecapoofficial@gmail.com` |
| **Repo launcher** | https://github.com/ElPibeCapo/BetelineyLauncher |
| **Repo meta** | https://github.com/ElPibeCapo/meta |
| **Rama principal** | `main` |
| **META server** | https://ElPibeCapo.github.io/meta/v1/ |
| **Código fuente local** | `/home/pibe/Descargas/Beteliney Launcher [Minecraft]/BetelineyLauncher/source/` |

---

## STACK TÉCNICO

- **Lenguaje**: C++20
- **UI**: Qt 6 — Widgets + QSS (no QML)
- **Build**: CMake 3.25+ + Ninja
- **CI/CD**: GitHub Actions — Ubuntu 24.04 (Linux) + MSYS2/MinGW64 (Windows)
- **Optimización**: `-O3 -march=znver1 -mtune=znver1` en Release
- **Auth**: MSA Device Code Flow → Xbox Live → XSTS → Minecraft token
- **Hashing**: MD4/MD5/SHA1/SHA256/SHA512/Murmur2 async — Murmur2 es el hash de CurseForge, SHA512 el de Modrinth
- **Librerías**: libarchive, zlib, zstd, lz4, lzma, bz2, libcmark, libqrencode, tomlplusplus, libxml2

---

## COMMITS (más recientes primero)

```
4955ada  docs: ESTADO.md — documento maestro único, depreca DEVLOG y MASTER_PLAN
44ee765  docs: MASTER_PLAN — Fase 1 completada
4174c4d  feat: BetelineyLogAnalyzer — motor diagnóstico logs v1.0 (639 líneas)
bb3ba44  docs: MASTER_PLAN actualizado sesion 2
ae1ddd6  fix: Q_INIT_RESOURCE duplicado, BUILD_TESTING OFF, CurseForge key via env, BUILD_ARTIFACT en CI
b844c53  docs: DEVLOG actualizado
16bd9bf  fix: Q_INIT_RESOURCE(multimc) -> beteliney_icons en main.cpp
0b4b519  ci: BUILD_TESTING=OFF + verbose ninja windows
a9a98cb  fix: add_custom_command -> add_custom_target (CMake 4.x)
255dfe5  ci: stub gamemode + quitar java-openjdk-devel de msys2
a424fb3  ci: cambiar windows a msys2+mingw64 (sin aqtinstall)
[...16 iteraciones de CI anteriores...]
2915f18  BetelineyLauncher v8.2.0 — commit inicial
```

---

## HACER UNA RELEASE

```bash
cd "/home/pibe/Descargas/Beteliney Launcher [Minecraft]/BetelineyLauncher/source"
git add -A && git commit -m "descripción"
git tag v8.X.Y
git push && git push --tags
# CI compila Linux + Windows y publica la Release en ~15 min automáticamente
```

Artefactos generados:
- `BetelineyLauncher-{VER}-Linux-x86_64.tar.gz`
- `BetelineyLauncher-{VER}-Windows-x64.zip`

---

## API KEYS Y SERVICIOS

| Servicio | Estado | Detalle |
|---|---|---|
| **CurseForge** | ✅ Key propia | Valor real: `***CURSEFORGE_KEY_ROTADA_PURGADA***` — En CI via secret `CURSEFORGE_API_KEY`. Local: `export CURSEFORGE_API_KEY="..."` antes de cmake |
| **Microsoft Azure** | ✅ App registrada | App ID: `4b945c78-d30b-489e-915f-b361bf9c933b` |
| **Imgur** | ⚠️ Key vacía | Código existe. Registrar en api.imgur.com/oauth2/addclient si se activa screenshots |
| **META server** | ✅ Contenido generado | Rama `gh-pages` tiene: net.minecraft, Forge, NeoForge, Fabric, Quilt, Java Adoptium/Azul/IBM. CI corre cada 6h, último OK 2026-06-16 |

---

## ACCIONES MANUALES PENDIENTES (requieren tu sesión GitHub en el navegador)

**#1 — Secret CurseForge en CI** (sin esto el build de CI no tiene la key de CurseForge):
```
https://github.com/ElPibeCapo/BetelineyLauncher/settings/secrets/actions
→ New repository secret
→ Nombre:  CURSEFORGE_API_KEY
→ Valor:   ***CURSEFORGE_KEY_ROTADA_PURGADA***
```

**#2 — Activar GitHub Pages del META server** (el contenido ya existe en `gh-pages`, solo falta habilitarlo):
```
https://github.com/ElPibeCapo/meta/settings/pages
→ Build and deployment → Source: "Deploy from a branch"
→ Branch: gh-pages → / (root) → Save
→ URL resultante: https://elpibecapo.github.io/meta/v1/
```

---

## AUDITORÍA COMPLETA DEL CÓDIGO — LO QUE EXISTE

### Sistema visual — BetelineyTheme
- `launcher/ui/themes/BetelineyTheme.cpp` — 810 líneas de QSS
- Paleta: fondo `#080912`, neón `#39FF14`, cyan `#00D4FF`, fuente JetBrains Mono
- 5 iteraciones (backups `.v3bak`, `.v4bak`)
- Cards de instancias con `drawGamerCard()`: bordes neón, hover cyan, QPainter nativo
- Todos los widgets cubiertos: toolbar, buttons, inputs, lists, tabs, scrollbars, menus, combobox, checkboxes, progress bars, sliders, dialogs, dock widgets, wizard, rubber band
- Otros temas: BrightTheme, DarkTheme, FusionTheme, CatPack, CustomTheme, SystemTheme

### Perfiles JVM — BetelineyProfiles + JavaSettingsWidget (COMPLETO)
Struct `BetelineyJVMProfile` con campos: `name`, `description`, `minMemMB`, `maxMemMB`, `jvmArgs`.

7 perfiles en `BETELINEY_PROFILES` (inline, header-only):

| Índice | Nombre | Rango RAM | Notas clave |
|---|---|---|---|
| 0 | Personalizado (sin flags) | 0/0 | Limpia todo para config manual |
| 1 | iGPU / RAM compartida | 384–1536 MB | G1HeapRegionSize=1M, MaxGCPauseMillis=100, MinHeapFreeRatio=5, sin UseTransparentHugePages |
| 2 | Ligero Vanilla | 512–2048 MB | G1HeapRegionSize=1M, sin UseStringDeduplication |
| 3 | Balanceado | 2048–4096 MB | G1HeapRegionSize=2M, UseStringDeduplication ON |
| 4 | Pesado 100–300 mods | 2048–6144 MB | G1HeapRegionSize=8M, AlwaysPreTouch ON |
| 5 | Extremo ≥300 mods | 6144–12288 MB | G1HeapRegionSize=16M, MaxGCPauseMillis=200 |
| 6 | iGPU ZGC Java21+ | 384–1536 MB | UseZGC+ZGenerational, pausas <1ms, SoftMaxHeapSize=1280m |

`JavaSettingsWidget.cpp` (541 líneas) implementa:
- ComboBox con los 7 perfiles + etiquetas de rango de RAM
- Perfil ZGC marcado con `⚡` en la lista
- Auto-detección de iGPU: Linux via `lspci -mm` (keywords: Vega, Picasso, Renoir, UHD Graphics, Iris...), Windows via `wmic Win32_VideoController`
- Auto-sugerencia de perfil según RAM total del sistema + iGPU detectada
- Botón "Aplicar perfil": confirma si hay args previos, setea minMemSpinBox/maxMemSpinBox, setea jvmArgsTextBox, advierte si hay -Xmx/-Xms en los args que conflictúen
- Badge GraalVM: detecta GraalVM al cambiar ruta Java (ejecuta `java -version` en background), muestra borde neón + tooltip con info de rendimiento
- `updateThresholds()`: warning rojo si maxMem >= RAM total, warning amarillo si > 90%, warning iGPU si > 50%
- PermGen oculto (deprecado desde Java 8)

Settings keys usadas (INI):
- `MinMemAlloc` — Xms
- `MaxMemAlloc` — Xmx
- `JvmArgs` — argumentos JVM como string
- `OverrideMemory` — bool, si la instancia sobreescribe la config global
- `OverrideJavaArgs` — bool, si la instancia sobreescribe los JVM args globales
- `OverrideJavaLocation` — bool, si la instancia usa un Java distinto
- `JavaPath` — ruta al ejecutable java
- `IgnoreJavaCompatibility` — bool
- `IgnoreJavaWizard` — bool
- `AutomaticJavaSwitch` — bool
- `AutomaticJavaDownload` — bool

Jerarquía de settings pages:
```
InstanceSettingsPage (BasePage)
  └── MinecraftSettingsWidget (QWidget)
        ├── JavaSettingsWidget* m_javaSettings   ← aquí viven los perfiles
        └── [resto de settings de Minecraft]
```

### Motor de diagnóstico — BetelineyLogAnalyzer (COMPLETO)

`launcher/logs/BetelineyLogAnalyzer.h/cpp` — 639 líneas.

Struct `Diagnosis`: `severity` (Critical/Error/Warning/Info), `errorCode`, `title`, `explanation`, `solution`, `detail`, `affectedMod`, `actionLabel`, `actionTarget`.

`LogAnalyzer::analyze(fullLog, exitCode)` — corre todos los checks, ordena por severidad, devuelve lista.

**18 checks implementados:**

| Check | Detecta | Severidad | ActionTarget |
|---|---|---|---|
| checkOutOfMemory | `java.lang.OutOfMemoryError` (heap/GC overhead/Metaspace) | Critical | `"java"` |
| checkHeapReservation | `Could not reserve enough space` / `Unable to create native thread` | Critical | `"java"` |
| checkDuplicateMod | `Duplicate mod id` (Fabric) / `Found duplicate mod` (Forge) — extrae nombre | Critical | `"mods-folder"` |
| checkMissingDependency | `requires mod X to be loaded` / `Missing Mods:` — extrae nombre | Critical | `"search-modrinth:MODNAME"` |
| checkIncompatibleMods | `Incompatible mods found` / `conflicts with mod` | Error | — |
| checkMixinConflict | `Mixin transformation failed` / `MixinApplyError` — extrae mod causante | Error | — |
| checkFabricIncompatible | Versiones incompatibles Fabric loader | Error | — |
| checkJavaNotFound | `Failed to start the minecraft runtime` / `No such file` en JVM path | Critical | `"java"` |
| checkUnsupportedJavaVersion | `UnsupportedClassVersionError` | Critical | `"java"` |
| checkForgeJavaRequirement | `Forge requires Java` | Critical | `"java"` |
| checkOpenGLNotAccelerated | `Pixel format not accelerated` | Error | — |
| checkOpenGLError | OpenGL errors genéricos | Warning | — |
| checkNativesCrash | Exit code -1073741819 (Windows AV) / SIGSEGV | Error | — |
| checkWindowsLoadLibrary | `LoadLibrary failed` | Error | — |
| checkNetworkError | Timeout / SSL / connection refused en auth | Warning | — |
| checkFractureiser | Hashes y firmas conocidos del malware Fractureiser | Critical | — |
| checkForgeEarlyWindow | `Failed to create early progress window` | Error | — |
| checkForgeCoremods | Coremod error durante init de Forge | Error | — |

Integración en `LogPage.cpp`:
- Se activa cuando `Task::finished` y `gameExitCode != 0`
- Panel `diagnosisPanel` en LogPage.ui (row 1, entre controles y log)
- Severidad visual: Critical=rojo (`#FF4444`), Error=naranja (`#FF8C00`), Warning=amarillo (`#FFD700`), Info=cyan (`#00D4FF`)
- Contador "1/3", "2/3"... si hay múltiples diagnósticos, botón "Siguiente ▶"
- ActionTargets: `"java"` → `APPLICATION->ShowGlobalSettings(this, "java-settings")`, `"mods-folder"` → `QDesktopServices::openUrl` de `mcInstance->modsRoot()`, `"search-modrinth:X"` → URL Modrinth
- Botón ✕ para descartar. Se oculta al iniciar nueva sesión de juego.

### Updater — BetelineyUpdater (COMPLETO)
- `BetelineyUpdater.h/cpp` — GitHub Releases API, semver, pre-releases, canal beta, AppImage update, backup diferencial
- `BetelineyExternalUpdater` — timer auto-check con intervalo configurable
- `GitHubRelease.h/cpp` — parser JSON de la API
- `UpdaterDialogs.h/cpp` + `SelectReleaseDialog.ui` — UI de confirmación
- `Launcher_BUILD_ARTIFACT` configurado en CI (`"Linux-x86_64"` / `"Windows-x64"`) → updater activo en builds

### Instalador Windows — NSIS (COMPLETO)
- `program_info/win_install.nsi.in` — 516 líneas
- 60+ idiomas, SpanishInternational incluido
- URL handlers: `curseforge://`, `beteliney://`, `beteliney-import://`
- Asocia: `.mrpack` (default), `.zip` (open-with)
- Start Menu + Desktop + Uninstaller completo

### Auth y cuentas (COMPLETO)
- `AccountType { MSA, Offline }` — ambas funcionales
- Cadena MSA: MSAStep → MSADeviceCodeStep → XboxUserStep → XboxAuthorizationStep → XboxProfileStep → EntitlementsStep → MinecraftProfileStep → GetSkinStep

### Plataformas de mods (COMPLETO)
- Modrinth: API + CheckUpdate + InstanceCreation + PackExport + PackIndex
- CurseForge/Flame: API + CheckUpdate + InstanceCreation + FileResolving + PackManifest
- También: ATLauncher, FTB, Legacy FTB, Technic, PackWiz

### Otros features heredados de Prism (COMPLETO)
- Symlinks entre instancias (assets compartidos — mayor ventaja de Prism)
- Symlinks: 50 instancias de 1.21.x usan el espacio de una sola más sus mods
- `McClient.h/cpp` + `McResolver.h/cpp` — ping de servidores Minecraft sobre TCP (MOTD, versión, jugadores, SRV records)
- `HashUtils`: Murmur2/SHA1/SHA256/SHA512 async
- Setup wizard: AutoJava, Java, Language, Login, Theme, Paste pages
- Java auto-descarga: `java/download/` infraestructura completa, ON en Windows por defecto
- `BetelineyCode.h` — easter egg: escribir B-E-T-E en MainWindow
- `AnonymizeLog` — elimina tokens, UUIDs, IPs antes de compartir logs
- Imgur upload (código existe, key vacía)

### Lo que NO existe aún

| Feature | Fase | Notas |
|---|---|---|
| BetelineyPacks | 3 | Plataforma propia de modpacks |
| RSS / news feed propio | 3 | Hoy apunta a releases.atom de GitHub |
| Presets de instancia por tipo | 3 | "Vanilla Optimizado", "PvP", "Modpack Pesado" |
| Detección conflictos de mods pre-lanzamiento | 4 | Leer fabric.mod.json / mods.toml antes de lanzar |
| Verificación hash vs base de malware | 4 | Ampliar lo de Fractureiser |
| Crash reporter del launcher (opt-in) | 4 | Stack trace → GitHub Issues |
| Importar desde GDLauncher Carbon | 4 | Formato propio de GDLauncher |
| Inyección auto de mods de rendimiento | 4 | Sodium+Lithium+Starlight en instancias Fabric |
| Flatpak oficial | 5 | Flathub |
| Firma de código Windows | 5 | EV cert o workaround SmartScreen |
| macOS activo | 5 | Código heredado, sin CI, baja prioridad |

---

## FASES DE DESARROLLO

### ✅ FASE 0 — Estabilización (COMPLETADA)
- `Q_INIT_RESOURCE(beteliney_icons)` duplicado eliminado
- `BUILD_TESTING=OFF` por defecto en CMakeLists
- CurseForge API key sacada del código → lee `$ENV{CURSEFORGE_API_KEY}`
- CI inyecta key desde `secrets.CURSEFORGE_API_KEY`
- `Launcher_BUILD_ARTIFACT` en CI → auto-updater activo en builds
- META server: rama `gh-pages` verificada con contenido completo
- Pendiente manual: acciones #1 y #2 de arriba

### ✅ FASE 1 — Motor de diagnóstico de logs (COMPLETADA · commit 4174c4d)
Ver sección "BetelineyLogAnalyzer" arriba — 18 checks, panel visual integrado.

### ✅ FASE 2 — Selector de perfiles JVM en UI (COMPLETADA · en commit inicial 2915f18)
Ver sección "BetelineyProfiles + JavaSettingsWidget" arriba — ya estaba implementado.
Incluye: auto-detección iGPU, auto-sugerencia de perfil, badge GraalVM, warnings de RAM.

---

### 🔄 FASE 3 — Ecosistema Beteliney (SIGUIENTE)

#### 3.1 BetelineyPacks — plataforma propia de modpacks

**Qué es:** lista de modpacks curados servidos desde GitHub Pages del repo meta.
Los usuarios los ven en una nueva pestaña dentro del diálogo "Nueva instancia".

**Formato del JSON** (un archivo por modpack en `gh-pages/v1/beteliney-packs/`):
```json
{
  "formatVersion": 1,
  "id": "beteliney-survival-1",
  "name": "Beteliney Survival",
  "description": "Survival vanilla-plus con mods de calidad de vida.",
  "version": "1.0.0",
  "minecraft": "1.21.1",
  "loader": "fabric",
  "loaderVersion": "0.16.9",
  "icon": "https://elpibecapo.github.io/meta/v1/beteliney-packs/icons/survival.png",
  "screenshots": ["https://..."],
  "tags": ["survival", "vanilla-plus"],
  "mods": [
    { "provider": "modrinth", "projectId": "AANobbMI", "version": "mc1.21.1-0.6.4+build.5" },
    { "provider": "modrinth", "projectId": "gvQqBUqZ", "version": "0.13.0+1.21" }
  ]
}
```

Un archivo índice en `gh-pages/v1/beteliney-packs/index.json` lista todos los IDs disponibles.

**Archivos a crear en el launcher:**
- `launcher/modplatform/beteliney/BetelineyPackIndex.h/cpp` — parser del JSON
- `launcher/modplatform/beteliney/BetelineyPackInstallTask.h/cpp` — descarga e instala los mods usando ModrinthAPI/FlameAPI existentes
- `launcher/ui/pages/modplatform/BetelineyPackPage.h/cpp/.ui` — página dentro del diálogo `NewInstanceDialog`

**Integración en NewInstanceDialog:**
- `launcher/ui/dialogs/NewInstanceDialog.cpp` ya tiene sistema de páginas (Vanilla, Modrinth, CurseForge...)
- Añadir `BetelineyPackPage` como primera pestaña (posición 0) para que sea la primera opción visible

**Cómo registrar el índice:** en `BuildConfig.cpp.in` ya hay `Launcher_BETELINEY_PACKS_URL` (verificar si existe, si no añadirlo) apuntando a `https://elpibecapo.github.io/meta/v1/beteliney-packs/index.json`

---

#### 3.2 RSS / noticias propias

**Qué es:** reemplazar el atom feed de GitHub Releases con un feed de noticias real que el equipo controla.

**Hoy:** `NEWS_RSS_URL = https://github.com/ElPibeCapo/BetelineyLauncher/releases.atom`
El `NewsChecker.h` ya lee cualquier RSS/Atom, solo hay que cambiar la URL.

**Plan:**
1. Crear `gh-pages/v1/news/feed.atom` generado estáticamente (editado a mano o con GitHub Actions)
2. Formato Atom estándar con `<entry>` para cada noticia: title, summary, content, link, updated
3. Cambiar en `CMakeLists.txt`: `Launcher_NEWS_RSS_URL` → `https://elpibecapo.github.io/meta/v1/news/feed.atom`
4. Cambiar `Launcher_NEWS_OPEN_URL` → URL de la página de noticias (GitHub Pages o wiki)

El `NewsChecker` y el widget ya funcionan — solo es cambiar la URL.

---

#### 3.3 Presets de instancia por tipo de juego

**Qué es:** al crear una instancia, el usuario puede elegir un "tipo de uso" que pre-configura mods recomendados y perfil JVM automáticamente.

**Tipos planeados:**
- `Vanilla Optimizado (Fabric)`: instala Sodium + Lithium + Iris + ModernFix en instancias Fabric — mejora FPS sin cambiar gameplay
- `PvP Competitivo`: JVM flags con GCTimeRatio más agresivo + mods PvP esenciales
- `Modpack Pesado (Forge/NeoForge)`: selecciona perfil JVM "Pesado" automáticamente + advertencia RAM

**Archivos a modificar:**
- Extender `BetelineyPackInstallTask` (de 3.1) con método `applyGamePreset(PresetType)`
- O implementar como paso adicional en el wizard de nueva instancia

---

### FASE 4 — Features avanzados

**4.1 Detección de conflictos de mods pre-lanzamiento**
- Leer `fabric.mod.json` (campo `id`) y `META-INF/mods.toml` (campo `modId`) de todos los mods de la instancia
- Antes de lanzar: buscar IDs duplicados, mostrar diálogo de advertencia con los conflictos
- Archivos a crear: `launcher/minecraft/mod/ModConflictChecker.h/cpp`
- Hook en `LaunchTask` antes del paso de lanzamiento efectivo

**4.2 Verificación de hash de mods**
- Al instalar cualquier mod desde Modrinth/CurseForge: verificar hash SHA512 (Modrinth) o Murmur2 (CurseForge) contra lo que devuelve la API
- Base de hashes de malware conocido (Fractureiser y variantes) como JSON en el META server: `gh-pages/v1/malware/known-hashes.json`
- Si el hash no coincide o está en lista negra: diálogo bloqueante con explicación

**4.3 Crash reporter del launcher (opt-in)**
- Si BetelineyLauncher mismo crashea (signal handler en Linux, UnhandledExceptionFilter en Windows): capturar stack trace + versión + OS
- Opción en Settings: "Enviar reportes de error anónimos"
- Destino: GitHub Issues API (sin necesitar cuenta del usuario)

**4.4 Importar desde GDLauncher Carbon**
- Formato de GDLauncher Carbon: SQLite (`gdlauncher_next/data.sqlite`) con tablas `instances` y `mods`
- Leer las instancias, convertir a formato Prism, copiar mods
- Archivo: `launcher/migration/GDLauncherMigrator.h/cpp`

**4.5 Inyección automática de mods de rendimiento**
- En instancias Fabric/Quilt nuevas: opción "Optimizar automáticamente" que instala Sodium + Lithium + Iris + ModernFix
- Usar ModrinthAPI existente para descargar, verificar hashes, instalar en carpeta mods
- Mostrar como paso opcional al crear una instancia Fabric

---

### FASE 5 — Distribución profesional

**5.1 Flatpak**
- Crear `com.beteliney.BetelineyLauncher.json` para Flatpak Builder
- Runtime: `org.kde.Platform//6.6`
- Subir a Flathub (requiere revisión del equipo)
- Alternativa rápida: distribuir `.flatpakref` directo desde GitHub Pages

**5.2 Firma de código Windows**
- Sin firma: SmartScreen bloquea el instalador con "Windows protegió tu PC"
- Solución ideal: certificado EV code signing (~$200-400/año)
- Solución gratuita: documentar el bypass (Más información → Ejecutar de todas formas) dentro del propio instalador en la pantalla de bienvenida

**5.3 Monitoreo de límites**
- GitHub Pages: 100 GB/mes bandwidth. Para el volumen actual: suficiente por años.
- Si se supera: migrar META server a Cloudflare Pages (gratis, sin límite)
- CurseForge API key: tiene rate limiting, monitorear si crece la base de usuarios
- Azure MSA: App ID `4b945c78-...` tiene límite de requests/segundo, monitorear

---

## DECISIONES TÉCNICAS FIJAS

| Decisión | Alternativa rechazada | Razón |
|---|---|---|
| Fork de Prism, no reescritura en Rust/Tauri | Reescritura desde cero | Auth MSA + Forge processors + todas las plataformas = meses de reimplementación con alto riesgo. El 90% ya existe y funciona. |
| Qt Widgets + QSS, no migrar a QML | Migrar a Qt Quick (QML) | Migrar 100+ vistas = reescribir la UI entera. El QSS actual es production-quality y el resultado visual es excelente. |
| GitHub Pages para META server | VPS propio | Gratis, auto-deploy, cero mantenimiento. Migrar a Cloudflare Pages si se superan los 100GB/mes. |
| MSYS2/MinGW64 para Windows CI | aqtinstall | aqtinstall falló en todos los mirrors disponibles durante el setup del CI. MSYS2 es más confiable. |
| INI files para settings | SQLite | Volumen de datos no lo justifica. SQLite cuando se necesite historial de mods, caché de búsquedas, analytics. |
| `BUILD_TESTING=OFF` por defecto | Tests en todo build | ECMAddTests.cmake falla en CMake 4.x en nuestro entorno. Tests siguen disponibles con `-DBUILD_TESTING=ON`. |

---

## VERSIONES

| Tipo | Criterio |
|---|---|
| **Patch** x.x.+1 | Bugfixes, cambios menores, actualización deps |
| **Minor** x.+1.0 | Feature nuevo completo, fase completa |
| **Major** +1.0.0 | Cambio arquitectural, reescritura de subsistema |

- **Actual**: v8.2.0
- **Próxima**: v8.3.0 — cuando Fase 3 (BetelineyPacks + RSS + Presets) esté completa
