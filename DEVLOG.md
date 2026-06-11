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
