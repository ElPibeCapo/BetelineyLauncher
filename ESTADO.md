# ESTADO — BetelineyLauncher
> Documento único, autocontenido. Cualquier chat nuevo lee SOLO esto y continúa.
> Actualizar al cerrar cada sesión. Última actualización: sesión 4.

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

## STACK

C++20 · Qt 6 Widgets+QSS · CMake 3.25+ · Ninja · GitHub Actions
Linux: Ubuntu 24.04 CI · Windows: MSYS2/MinGW64 CI · `-O3 -march=znver1`
Auth: MSA Device Code Flow → Xbox Live → XSTS → Minecraft token
Hashing: MD4/MD5/SHA1/SHA256/SHA512/Murmur2 async (Murmur2=CurseForge, SHA512=Modrinth)

---

## COMMITS

```
[pendiente]  feat: Fase 3 — BetelineyPacks + presets + RSS
5a70db3  docs: ESTADO.md v2
4955ada  docs: ESTADO.md v1
44ee765  docs: MASTER_PLAN Fase 1 completada
4174c4d  feat: BetelineyLogAnalyzer — 18 checks
ae1ddd6  fix: Q_INIT_RESOURCE dup, BUILD_TESTING OFF, CurseForge env, BUILD_ARTIFACT CI
2915f18  BetelineyLauncher v8.2.0 — commit inicial
```

---

## HACER UNA RELEASE

```bash
cd "/home/pibe/Descargas/Beteliney Launcher [Minecraft]/BetelineyLauncher/source"
git add -A && git commit -m "descripción"
git tag v8.X.Y
git push && git push --tags
# CI compila Linux + Windows y publica Release en ~15 min
```

---

## API KEYS Y SERVICIOS

| Servicio | Estado | Detalle |
|---|---|---|
| **CurseForge** | ✅ | Valor: `***CURSEFORGE_KEY_ROTADA_PURGADA***` — CI via secret `CURSEFORGE_API_KEY` · local: `export CURSEFORGE_API_KEY="..."` |
| **Microsoft Azure** | ✅ | App ID: `4b945c78-d30b-489e-915f-b361bf9c933b` |
| **Imgur** | ⚠️ | Key vacía. Registrar si se activa screenshots. |
| **META server** | ✅ | `gh-pages` tiene todos los JSONs. CI cada 6h, OK. |

---

## ACCIONES MANUALES PENDIENTES

**#1 — Secret CurseForge** (sin esto CI no tiene la key):
```
https://github.com/ElPibeCapo/BetelineyLauncher/settings/secrets/actions
→ New repository secret → CURSEFORGE_API_KEY → ***CURSEFORGE_KEY_ROTADA_PURGADA***
```

**#2 — GitHub Pages del META server** (contenido listo en `gh-pages`, falta activarlo):
```
https://github.com/ElPibeCapo/meta/settings/pages
→ Deploy from a branch → gh-pages → / (root) → Save
```

**#3 — Feed de noticias en el repo meta** (Fase 3.2 ya apunta a esta URL, falta crear el archivo):
Crear `gh-pages/v1/news/feed.atom` en el repo https://github.com/ElPibeCapo/meta
Formato Atom estándar con `<entry>` por noticia: title, summary, link, updated.
El NewsChecker ya lo consume automáticamente cuando exista.

---

## FASES

### ✅ FASE 0 — Estabilización
`Q_INIT_RESOURCE` dup · `BUILD_TESTING=OFF` · CurseForge key en env · `BUILD_ARTIFACT` en CI · META server verificado.
Pendiente: acción manual #1 y #2 arriba.

### ✅ FASE 1 — Motor de diagnóstico de logs (commit 4174c4d)
`BetelineyLogAnalyzer` — 18 checks, panel visual en LogPage integrado.
Ver sección "AUDITORÍA" más abajo para detalle completo.

### ✅ FASE 2 — Selector de perfiles JVM en UI (en commit inicial 2915f18)
`JavaSettingsWidget.cpp` — ya implementado. 7 perfiles, auto-detección iGPU, badge GraalVM, warnings RAM.

### ✅ FASE 3 — Ecosistema Beteliney (SIN COMMITEAR — pendiente commit)

**Archivos creados/modificados:**

| Archivo | Líneas | Qué hace |
|---|---|---|
| `launcher/modplatform/beteliney/BetelineyPack.h` | 48 | Structs: `Pack`, `PackMod`, `PackIndex`, enum `PackProvider` |
| `launcher/modplatform/beteliney/BetelineyPackListModel.h/cpp` | 52+177 | Descarga `index.json` + cada `{id}.json`, parsea packs, emite `packLoaded()` y `loadingFinished()`, ordena featured primero |
| `launcher/modplatform/beteliney/BetelineyPackInstallTask.h/cpp` | 42+164 | `InstanceCreationTask`: crea instancia con loader correcto (fabric/forge/neoforge/quilt/vanilla), descarga mods via NetJob, verifica SHA-512 |
| `launcher/modplatform/beteliney/BetelineyPresets.h` | 144 | Presets built-in sin red: Vanilla Optimizado (Sodium+Lithium+Iris+ModernFix Fabric 1.21.1), PvP Competitivo (Sodium+Lithium+FerriteCore), Modpack Pesado (NeoForge base limpia) |
| `launcher/ui/pages/modplatform/beteliney/BetelineyPackPage.h/cpp/.ui` | 59+217+98 | Página UI completa: lista izquierda (icon 64px + nombre), panel derecho (icon 96px, nombre, meta, tags, descripción), barra de búsqueda con filtro, carga presets built-in inmediatamente + remotos desde red, descarga iconos async con QNetworkAccessManager |
| `launcher/ui/dialogs/NewInstanceDialog.cpp` | +2 líneas | `BetelineyPackPage` añadida como primera pestaña en `getPages()` |
| `launcher/CMakeLists.txt` | +11 líneas | Todos los .cpp/.h/.ui de beteliney registrados |
| `CMakeLists.txt` | +1 línea | `Launcher_BETELINEY_PACKS_URL` apunta a `https://ElPibeCapo.github.io/meta/v1/beteliney-packs/` |
| `CMakeLists.txt` | modif | `NEWS_RSS_URL` cambiado a `https://ElPibeCapo.github.io/meta/v1/news/feed.atom` |
| `buildconfig/BuildConfig.h/cpp.in` | +5 líneas | Campo `BETELINEY_PACKS_URL` añadido |

**Formato del índice** (`gh-pages/v1/beteliney-packs/index.json`):
```json
{ "formatVersion": 1, "ids": ["beteliney-survival-1", "beteliney-pvp-1"] }
```

**Formato de cada pack** (`gh-pages/v1/beteliney-packs/{id}.json`):
```json
{
  "formatVersion": 1, "id": "beteliney-survival-1", "name": "Beteliney Survival",
  "description": "...", "longDescription": "...", "version": "1.0.0",
  "minecraft": "1.21.1", "loader": "fabric", "loaderVersion": "0.16.9",
  "icon": "https://elpibecapo.github.io/meta/v1/beteliney-packs/icons/survival.png",
  "screenshots": [], "tags": ["survival"], "featured": true,
  "mods": [
    { "provider": "modrinth", "projectId": "AANobbMI",
      "version": "mc1.21.1-0.6.4+build.5",
      "url": "https://cdn.modrinth.com/data/AANobbMI/versions/PfHePkD4/sodium-fabric-0.6.4%2Bmc1.21.1.jar",
      "sha512": "", "filename": "sodium-fabric-0.6.4+mc1.21.1.jar" }
  ]
}
```

**Presets built-in** (sin red, disponibles siempre):
- `builtin-vanilla-optimized`: Fabric 1.21.1 + Sodium + Lithium + Iris + ModernFix — `featured=true`
- `builtin-pvp-competitive`: Fabric 1.21.1 + Sodium + Lithium + FerriteCore
- `builtin-heavy-modpack`: NeoForge 1.21.1 base limpia (sin mods, para modpacks manuales)

**Pendiente de hacer para que Fase 3 esté 100% funcional:**
- [ ] Hacer commit de todo (ver sección "ACCIÓN INMEDIATA" abajo)
- [ ] Crear `gh-pages/v1/beteliney-packs/index.json` en repo meta (acción manual en repo meta)
- [ ] Crear primer pack JSON real en `gh-pages/v1/beteliney-packs/beteliney-survival-1.json`
- [ ] Acción manual #3: crear `gh-pages/v1/news/feed.atom` en repo meta

---

### 🔄 FASE 4 — Features avanzados (SIGUIENTE después de commitear Fase 3)

**4.1 Detección de conflictos de mods pre-lanzamiento**
- Archivo a crear: `launcher/minecraft/mod/ModConflictChecker.h/cpp`
- Lee todos los `fabric.mod.json` (campo `id`) y `META-INF/mods.toml` (campo `modId`) de la carpeta mods
- Busca IDs duplicados antes de lanzar
- Hook: en `LaunchTask` antes del paso `MinecraftProcess::start()` — buscar en `launch/steps/CheckJava.cpp` como referencia para insertar un paso nuevo
- UI: `QMessageBox::warning` con lista de conflictos, botón "Lanzar de todas formas"

**4.2 Verificación de hash de mods al instalar**
- Archivo en META server: `gh-pages/v1/malware/known-hashes.json` — lista negra de SHA-256 de malware conocido
- Hook: en `ModrinthInstanceCreationTask` y `FlameInstanceCreationTask`, después de cada descarga
- Usar `HashUtils::getSha256` (ya existe en `modplatform/helpers/HashUtils.h`)
- Si hash está en lista negra: `QMessageBox::critical` bloqueante con explicación

**4.3 Crash reporter del launcher (opt-in)**
- Configuración: setting `SendCrashReports` en Application settings
- Linux: signal handler con `backtrace()` + `backtrace_symbols()`
- Windows: `SetUnhandledExceptionFilter` + `MiniDumpWriteDump`
- Destino: GitHub Issues API (`POST /repos/ElPibeCapo/BetelineyLauncher/issues`) con template automático
- UI: opción en Settings → General → "Enviar reportes de error anónimos"

**4.4 Importar desde GDLauncher Carbon**
- GDLauncher guarda en: Linux `~/.local/share/gdlauncher_next/`, Windows `%APPDATA%/gdlauncher_next/`
- Base de datos: SQLite `data.sqlite` con tablas `instances` y `mods`
- Leer con SQLite (Qt tiene `QSqlDatabase` con driver QSQLITE)
- Convertir instancias al formato Prism (`instance.cfg` + `mmc-pack.json`)
- Archivo a crear: `launcher/migration/GDLauncherMigrator.h/cpp`
- UI: wizard de migración, accesible desde File → Import → GDLauncher Carbon

**4.5 Inyección auto de mods de rendimiento en instancias Fabric existentes**
- Hook: en `VersionPage` (la página de componentes de una instancia), botón "Optimizar" aparece cuando el loader es Fabric
- Al pulsar: usa `BetelineyPackInstallTask` con el preset `builtin-vanilla-optimized` para instalar los mods en la instancia existente (no crea instancia nueva)
- Solo añade mods que no están ya presentes (check por nombre de archivo)

---

### FASE 5 — Distribución profesional

**5.1 Flatpak**
- Crear `com.beteliney.BetelineyLauncher.json`
- Runtime: `org.kde.Platform//6.6`
- Subir a Flathub o distribuir `.flatpakref`

**5.2 Firma de código Windows**
- Sin firma: SmartScreen bloquea con "Windows protegió tu PC"
- Documentar el bypass dentro del instalador NSIS en la pantalla Welcome

**5.3 Monitoreo de límites**
- GitHub Pages: 100 GB/mes. Si se supera → migrar META a Cloudflare Pages
- CurseForge API: tiene rate limiting, monitorear con usuarios reales

---

## AUDITORÍA DEL CÓDIGO

### BetelineyLogAnalyzer — 18 checks

| Check | Detecta | Severidad | ActionTarget |
|---|---|---|---|
| checkOutOfMemory | `java.lang.OutOfMemoryError` (heap/GC/Metaspace) | Critical | `"java"` |
| checkHeapReservation | `Could not reserve enough space` | Critical | `"java"` |
| checkDuplicateMod | `Duplicate mod id` Fabric / `Found duplicate mod` Forge | Critical | `"mods-folder"` |
| checkMissingDependency | `requires mod X to be loaded` / `Missing Mods:` | Critical | `"search-modrinth:X"` |
| checkIncompatibleMods | `Incompatible mods found` | Error | — |
| checkMixinConflict | `Mixin transformation failed` | Error | — |
| checkFabricIncompatible | versiones incompatibles Fabric loader | Error | — |
| checkJavaNotFound | `Failed to start the minecraft runtime` | Critical | `"java"` |
| checkUnsupportedJavaVersion | `UnsupportedClassVersionError` | Critical | `"java"` |
| checkForgeJavaRequirement | `Forge requires Java` | Critical | `"java"` |
| checkOpenGLNotAccelerated | `Pixel format not accelerated` | Error | — |
| checkOpenGLError | OpenGL errors genéricos | Warning | — |
| checkNativesCrash | exit code -1073741819 / SIGSEGV | Error | — |
| checkWindowsLoadLibrary | `LoadLibrary failed` | Error | — |
| checkNetworkError | Timeout/SSL/connection refused en auth | Warning | — |
| checkFractureiser | Hashes/firmas del malware Fractureiser | Critical | — |
| checkForgeEarlyWindow | `Failed to create early progress window` | Error | — |
| checkForgeCoremods | Coremod error Forge init | Error | — |

Panel en LogPage: activa cuando `exitCode != 0`. Severidad por color. Navegación 1/N. Acciones: abre Settings Java, abre carpeta mods, busca en Modrinth. Botón ✕ para descartar.

### BetelineyProfiles — 7 perfiles JVM

| Idx | Nombre | RAM | Notas clave |
|---|---|---|---|
| 0 | Personalizado | 0/0 | Limpia todo |
| 1 | iGPU/RAM compartida | 384–1536 MB | G1HeapRegionSize=1M, sin UseTransparentHugePages |
| 2 | Ligero Vanilla | 512–2048 MB | G1HeapRegionSize=1M |
| 3 | Balanceado | 2048–4096 MB | G1HeapRegionSize=2M, UseStringDeduplication ON |
| 4 | Pesado 100–300 mods | 2048–6144 MB | G1HeapRegionSize=8M, AlwaysPreTouch |
| 5 | Extremo ≥300 mods | 6144–12288 MB | G1HeapRegionSize=16M |
| 6 | iGPU ZGC Java21+ | 384–1536 MB | ZGC generacional, pausas <1ms |

`JavaSettingsWidget.cpp`: detección iGPU via `lspci -mm` (Linux) / `wmic` (Windows), auto-sugerencia por RAM, badge GraalVM, warnings > 90%/50% RAM.
Settings INI keys: `MinMemAlloc`, `MaxMemAlloc`, `JvmArgs`, `OverrideMemory`, `OverrideJavaArgs`.

### Arquitectura de Settings
```
InstanceSettingsPage (BasePage)
  └── MinecraftSettingsWidget (QWidget)
        └── JavaSettingsWidget* m_javaSettings
```

### Resto del código (heredado de Prism, completo)
- Auth MSA completa (Device Code Flow)
- `AccountType { MSA, Offline }` — ambas funcionales
- Symlinks entre instancias (assets compartidos)
- Modrinth + CurseForge + ATLauncher + FTB + Technic + PackWiz
- Java auto-descarga (ON en Windows, OFF en Linux por defecto)
- Server ping TCP (`McClient` + `McResolver`)
- Imgur upload (código existe, key vacía)
- NSIS installer completo (60+ idiomas, URL handlers, asociaciones de archivo)
- BetelineyUpdater completo (GitHub Releases API, diferencial, AppImage)
- Easter egg: escribir B-E-T-E en MainWindow
- `AnonymizeLog` para compartir logs sin datos sensibles

---

## DECISIONES TÉCNICAS FIJAS

| Decisión | Razón |
|---|---|
| Fork de Prism, no reescritura | Auth MSA + Forge processors + plataformas = meses reimplementando. 90% ya existe. |
| Qt Widgets + QSS, no QML | Migrar 100+ vistas = reescribir la UI entera. QSS es production-quality. |
| GitHub Pages para META | Gratis, cero mantenimiento. Migrar a Cloudflare Pages si supera 100 GB/mes. |
| MSYS2/MinGW64 Windows CI | aqtinstall falló en todos los mirrors. MSYS2 funciona. |
| INI files para settings | Suficiente por ahora. SQLite cuando se necesite historial/caché. |
| `BUILD_TESTING=OFF` default | ECMAddTests.cmake falla en CMake 4.x en este entorno. |

---

## VERSIONES

Actual: **v8.2.0** · Próxima: **v8.3.0** cuando Fase 3 esté commiteada y probada.
Patch: bugfixes · Minor: feature completo · Major: cambio arquitectural.
