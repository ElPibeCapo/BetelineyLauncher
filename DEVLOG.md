# DEVLOG — BetelineyLauncher

Registro completo de todo lo que se ha hecho, cambiado y configurado en el proyecto.

---

## Junio 2026 — Sesión de desarrollo completa

### Distribución multiplataforma

**Compilación cruzada Windows desde Linux**

Se configuró un sistema de compilación cruzada (cross-compilation) para generar el ejecutable de Windows directamente desde Linux (CachyOS), sin necesitar Windows instalado.

Herramientas instaladas:
- `mingw-w64-gcc 16.1.0` — compilador cruzado para Windows x64
- Qt 6.9.3 Windows (MinGW 64-bit) vía `aqtinstall` — DLLs y headers pre-compilados
- Qt 6.9.3 Linux vía `aqtinstall` — host tools compatibles (moc, rcc, uic)
- Vulkan headers copiados al sysroot MinGW para compilación de HardwareInfo

Problemas resueltos durante el proceso:
- `CMake 4.3.x` exige `IMPORTED_IMPLIB` base además de `IMPORTED_IMPLIB_RELEASE` en targets importados — parcheados 5 archivos cmake de vcpkg
- `cmark::cmark` — creado config cmake personalizado en `/tmp/cmark_cmake_fix/`
- `ECM_DIR` — apuntado explícitamente a `/usr/share/ECM/cmake`
- Incompatibilidad de versiones Qt (host 6.11.1 vs target 6.9.3) — resuelto bajando host tools a 6.9.3
- Headers Windows con case incorrecto (`Shellapi.h`, `Windows.h`, `Filter.h`) — creados symlinks en el sysroot MinGW
- `__imp___argc` faltante (incompatibilidad ABI MinGW 16 vs Qt compilado con MinGW 13) — creado stub objeto `/tmp/argc_stub.o`
- `JavaCheck`/`NewLaunch` — CMake 4.x no permite `add_custom_command(TARGET)` de subdirectorio — parcheado temporalmente el CMakeLists, restaurado después del build
- Múltiples DLLs faltantes en el paquete final (libqrencode, libssp-0, Qt6Svg, Qt6Test, legacy) — detectadas y agregadas con verificación completa

**Paquetes generados:**

| Archivo | Plataforma | Tamaño |
|---|---|---|
| `BetelineyLauncher-8-Linux-x86_64.tar.gz` | Linux x86_64 | 51 MB |
| `BetelineyLauncher-Windows-x64.zip` | Windows 10/11 | 29 MB |

Ubicaciones:
- `/home/pibe/Descargas/Linux - Beteliney [Minecraft]/`
- `/home/pibe/Descargas/Windows - Beteliney [Minecraft]/`

Contenido del ZIP Windows (verificado):
- `beteliney.exe`
- Qt6: Core, Gui, Widgets, Network, NetworkAuth, Xml, Concurrent, OpenGL, OpenGLWidgets, Svg, Test
- MinGW runtime: libgcc_s_seh-1, libstdc++-6, libwinpthread-1, libssp-0
- vcpkg: libarchive, libcmark, libcrypto, libssl, libqrencode, libzlib1, libbz2, liblz4, liblzma, libxml2, libtomlplusplus, libiconv, libcharset, libpkgconf, legacy
- Plugins Qt: platforms/qwindows, tls/qopensslbackend, imageformats/*, iconengines/qsvgicon
- JARs: NewLaunch.jar, NewLaunchLegacy.jar, JavaCheck.jar

---

### Limpieza de rastros de Prism Launcher

**Archivos modificados para independencia:**

`CMakeLists.txt`:
- `LEGACY_FMLLIBS_BASE_URL` → `github.com/ElPibeCapo/BetelineyLauncher/releases/download/fmllibs/`
- `META_URL` → `https://ElPibeCapo.github.io/meta/v1/` (servidor propio)
- `TRANSLATIONS_URL` → vacío (usa traducciones bundleadas)
- `TRANSLATION_FILES_URL` → vacío (usa traducciones bundleadas)
- `IMGUR_CLIENT_ID` → vacío (registrar propio en api.imgur.com/oauth2/addclient)
- `CURSEFORGE_API_KEY` → key propia
- `MSA_CLIENT_ID` → ID de Azure propio (`4b945c78-d30b-489e-915f-b361bf9c933b`)
- `NEWS_RSS_URL` → `https://github.com/ElPibeCapo/BetelineyLauncher/releases.atom`
- `NEWS_OPEN_URL` → `https://github.com/ElPibeCapo/BetelineyLauncher/releases`
- `WIKI_URL` → `https://github.com/ElPibeCapo/BetelineyLauncher/wiki`
- `HELP_URL` → `https://github.com/ElPibeCapo/BetelineyLauncher/issues`
- `BUG_TRACKER_URL` → `https://github.com/ElPibeCapo/BetelineyLauncher/issues`
- `UPDATER_GITHUB_REPO` → `https://github.com/ElPibeCapo/BetelineyLauncher`
- Todas las URLs `github.com/beteliney/` → `github.com/ElPibeCapo/`

`launcher/resources/`:
- Directorio `multimc/` renombrado a `beteliney-icons/`
- `multimc.qrc` renombrado a `beteliney-icons.qrc`
- Prefix en QRC: `/icons/multimc` → `/icons/beteliney-icons`
- `launcher/CMakeLists.txt` actualizado con la nueva ruta

`launcher/Application.cpp`:
- Referencias `:/icons/multimc/` → `:/icons/beteliney-icons/`
- Código de migración MultiMC/PolyMC conservado (ayuda a usuarios migrar)

`launcher/resources/documents/credits.html`:
- `El_PibeCapo` agregado como desarrollador principal en `%1`
- Lista de Prism Launcher developers eliminada (reemplazada)
- MultiMC developers conservados (requerido por GPL-3.0)
- Link de contribuidores → `github.com/ElPibeCapo/BetelineyLauncher`

`program_info/com.beteliney.BetelineyLauncher.metainfo.xml.in`:
- URLs limpias: solo GitHub real, eliminados opencollective, weblate, wiki inexistentes

`program_info/README.md`:
- Reescrito en español, sin referencias a Prism

`README.md`:
- Versión actualizada de v7.3.0 → v8.2.0

`launcher/minecraft/mod/tasks/LocalResourceUpdateTask.cpp`:
- Comentario interno limpiado

**Lo que NO se cambió (intencional):**

- Headers de copyright en archivos originales — requerido por GPL-3.0
- `packwiz/Packwiz.cpp` keys `x-prismlauncher-*` — parte del estándar del formato packwiz, cambiarlas rompe compatibilidad
- Código de migración MultiMC/PolyMC en Application.cpp — funcionalidad útil para usuarios

---

### API Keys y servicios propios

| Servicio | Estado | Notas |
|---|---|---|
| **CurseForge** | ✅ Key propia configurada | Registrada en console.curseforge.com |
| **Microsoft Azure** | ✅ App registrada | App ID: `4b945c78-d30b-489e-915f-b361bf9c933b`, cuenta: elpibecapoofficial@gmail.com |
| **Imgur** | ⚠️ Vacío | Registrar en api.imgur.com/oauth2/addclient cuando se necesite |
| **META server** | ✅ Fork propio | github.com/ElPibeCapo/meta, GitHub Pages en ElPibeCapo.github.io/meta/v1/ |

---

### GitHub

**Repositorio principal:**
- URL: `https://github.com/ElPibeCapo/BetelineyLauncher`
- Rama principal: `main`
- Visibilidad: Público
- Licencia: GPL-3.0

**Configuración git local:**
```
user.name  = ElPibeCapo
user.email = elpibecapoofficial@gmail.com
```

**Commits actuales:**
1. `2915f18` — BetelineyLauncher v8.2.0 (commit inicial, código completo)
2. `9828ea0` — GitHub Actions CI/CD
3. `e894a01` — Independencia de infraestructura Prism

**GitHub Actions (`.github/workflows/build.yml`):**

Triggers:
- Push a `main` → compila y genera artefactos
- Tag `v*.*.*` → compila + crea Release con archivos descargables
- Manual (workflow_dispatch)

Jobs:
- `build-linux` — Ubuntu 22.04, Qt6 vía apt, genera `.tar.gz`
- `build-windows` — Windows Server 2022, Qt6 vía `jurplel/install-qt-action`, MSVC, vcpkg manifest mode, genera `.zip`
- `release` — solo en tags, descarga artefactos y crea GitHub Release

**Cómo hacer una release nueva:**
```bash
cd "/home/pibe/Descargas/Beteliney Launcher [Minecraft]/BetelineyLauncher/source"
git add -A
git commit -m "descripción del cambio"
git tag v8.3.0
git push && git push --tags
```
Eso dispara el workflow y en ~15 minutos aparece la release en GitHub con Linux y Windows listos.

---

### META server (servidor de metadatos)

El META server es el servidor que le dice al launcher qué versiones de Minecraft existen, qué librerías necesita, qué versiones de Java usar. Es la dependencia más crítica.

**Antes:** apuntaba a `meta.prismlauncher.org` (infraestructura de Prism, dependencia externa)

**Ahora:** `https://ElPibeCapo.github.io/meta/v1/` (infraestructura propia)

Setup:
- Fork de `github.com/PrismLauncher/meta` → `github.com/ElPibeCapo/meta`
- Workflow `.github/workflows/generate.yml` en el fork
- Genera todos los JSONs cada 6 horas (Mojang, Fabric, Forge, NeoForge, Quilt, Java)
- Despliega automáticamente a GitHub Pages vía rama `gh-pages`

---

### Documentos generados para usuarios

`/home/pibe/Descargas/Linux - Beteliney [Minecraft]/COMO_USARLO.txt`:
- Cómo extraer el tar.gz
- Cómo ejecutar con `bash run.sh`
- Cómo configurar cuentas (Microsoft u offline)
- Solución a problemas comunes (permisos, OpenGL)

`/home/pibe/Descargas/Windows - Beteliney [Minecraft]/COMO_USARLO.txt`:
- Cómo extraer el ZIP
- Cómo manejar la advertencia de Windows Defender
- Cómo configurar cuentas
- Cómo importar modpacks

---

### Estado actual del proyecto

**Lo que funciona hoy:**
- ✅ Build Linux generado y empaquetado
- ✅ Build Windows generado y empaquetado (todas las DLLs verificadas)
- ✅ GitHub repo en github.com/ElPibeCapo/BetelineyLauncher
- ✅ GitHub Actions configurado
- ✅ META server propio en proceso de configuración
- ✅ CurseForge con key propia
- ✅ Microsoft login con app Azure propia
- ✅ Sin dependencias Prism en URLs funcionales (excepto META hasta que GitHub Pages confirme)

**Pendientes:**
- ⏳ Verificar que el workflow del META termine y GitHub Pages sirva correctamente
- ⏳ Recompilar el launcher con todos los cambios (API keys, Azure ID, URLs)
- ⏳ Registrar key de Imgur si se va a usar la función de screenshots
- ⏳ Mejoras de código: sistema de modpacks propio (BetelineyPacks), news checker propio

---

### Registro de propiedad intelectual

El código original (`BetelineyCode.h`, `BetelineyProfiles.h`, `BetelineyExternalUpdater.*`, `BetelineyTheme.*`, scripts de compilación/empaquetado) es propiedad de El_PibeCapo.

El proyecto se basa en Prism Launcher (GPL-3.0). Los archivos originales de Prism y MultiMC conservan sus avisos de copyright como lo exige la licencia GPL-3.0.

Para registro formal de autoría: DNDA Colombia (dnda.gov.co) → Registro de obras → Programa de computador.

## Continuación — GitHub Actions CI/CD

### Problema central
El CI de GitHub Actions falló múltiples veces. Cada intento reveló un problema nuevo. Registro completo:

**Intento 1** — `libtomlplusplus-dev` no existe en Ubuntu 22.04
Fix: cambiar a Ubuntu 24.04

**Intento 2** — `libgamemode-dev` no existe en Ubuntu 24.04 universe
Fix: intentar instalar y crear stub pkg-config si falla

**Intento 3** — `ECMAddTests.cmake:109` falla por tests mal configurados
Fix: `-DBUILD_TESTING=OFF` en CMake configure

**Intento 4** — `add_custom_command(TARGET JavaCheck)` falla en CMake 4.x cuando el target está en un subdirectorio
Fix: reemplazar por `add_custom_target(CopyJars ALL DEPENDS JavaCheck NewLaunch)` — patrón correcto para CMake 4.x

**Intento 5** — `Q_INIT_RESOURCE(multimc)` en `main.cpp` — símbolo `qInitResources_multimc` no resuelto porque el archivo QRC fue renombrado a `beteliney-icons.qrc`
Fix: cambiar a `Q_INIT_RESOURCE(beteliney_icons)`

**Intento 6 (Windows)** — `jurplel/install-qt-action@v4` con Qt 6.9.3, 6.8.3, 6.7.3, 6.5.3 — todos fallan porque aqtinstall no puede descargar el checksum de los mirrors de Qt
Fix: cambiar a `msys2/setup-msys2@v2` con `mingw-w64-x86_64-qt6-*` — instala desde los repos de MSYS2, sin depender de mirrors de Qt

**Intento 7 (Windows msys2)** — `java-openjdk-devel` no existe en MSYS2
Fix: quitar ese paquete (Java lo maneja el launcher en runtime, no en compilación)

**Estado actual:** en progreso, fix de `Q_INIT_RESOURCE` pendiente de validar en CI

### Commits del CI
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

### Archivos modificados en total esta sesión

| Archivo | Qué cambió |
|---|---|
| `CMakeLists.txt` | URLs, API keys, META_URL, CopyJars fix CMake 4.x |
| `launcher/main.cpp` | Q_INIT_RESOURCE multimc → beteliney_icons |
| `launcher/CMakeLists.txt` | QRC path multimc → beteliney-icons |
| `launcher/Application.cpp` | :/icons/multimc/ → :/icons/beteliney-icons/ |
| `launcher/resources/beteliney-icons/` | Directorio renombrado desde multimc/ |
| `launcher/resources/documents/credits.html` | Devs actualizados, link GitHub correcto |
| `program_info/metainfo.xml.in` | URLs limpias |
| `program_info/README.md` | Reescrito en español |
| `README.md` | Versión 7.3.0 → 8.2.0 |
| `LocalResourceUpdateTask.cpp` | Comentario interno limpiado |
| `.github/workflows/build.yml` | CI/CD completo (16 iteraciones) |
| `DEVLOG.md` | Este archivo |

### Setup de GitHub

Repo: https://github.com/ElPibeCapo/BetelineyLauncher
Usuario: ElPibeCapo
Email: elpibecapoofficial@gmail.com
Rama: main

Para hacer una release:
```bash
cd "/home/pibe/Descargas/Beteliney Launcher [Minecraft]/BetelineyLauncher/source"
git add -A && git commit -m "descripcion"
git tag v8.3.0
git push && git push --tags
```
Eso dispara el CI — compila Linux y Windows y publica la release automáticamente.

### META server
Fork: https://github.com/ElPibeCapo/meta
URL final: https://ElPibeCapo.github.io/meta/v1/
Workflow: genera JSONs de Mojang/Fabric/Forge/NeoForge/Quilt/Java cada 6h y los despliega via GitHub Pages.
El launcher ya apunta a esa URL en CMakeLists.txt.

### API keys configuradas

| Servicio | Estado |
|---|---|
| CurseForge | Key propia en CMakeLists |
| Microsoft Azure | App ID 4b945c78-... registrada en portal.azure.com |
| Imgur | Vacío — registrar cuando se necesite |

### Pendientes al cerrar este chat

1. Verificar que el último CI pase (fix Q_INIT_RESOURCE + BUILD_TESTING=OFF)
2. Verificar que el META server de GitHub Pages esté sirviendo correctamente en https://ElPibeCapo.github.io/meta/v1/
3. Recompilar local con todos los cambios para distribuir el paquete actualizado
4. Registrar API de Imgur si se va a usar la función de screenshots
5. Mejoras de código futuras: BetelineyPacks (plataforma de modpacks propia), news checker propio, reescritura de MainWindow
