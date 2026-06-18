# ESTADO — BetelineyLauncher
> Documento único, autocontenido. Cualquier chat nuevo lee SOLO esto y continúa.
> Última actualización: sesión 5 (Fase 4 completa, Fase 5 en progreso).

---

## IDENTIDAD

| | |
|---|---|
| **Nombre** | BetelineyLauncher |
| **Versión actual** | v8.2.0 (próxima: v8.3.0) |
| **Base** | Prism Launcher (GPL-3.0), fork extensamente modificado |
| **Autor** | El_PibeCapo — `elpibecapoofficial@gmail.com` |
| **Repo launcher** | https://github.com/ElPibeCapo/BetelineyLauncher |
| **Repo meta** | https://github.com/ElPibeCapo/meta |
| **Código fuente local** | `/home/pibe/Descargas/Beteliney Launcher [Minecraft]/BetelineyLauncher/source/` |
| **META server URL** | https://ElPibeCapo.github.io/meta/v1/ |

---

## STACK

C++20 · Qt 6 Widgets+QSS · CMake 3.25+ · Ninja · GitHub Actions
Linux CI: Ubuntu 24.04 · Windows CI: MSYS2/MinGW64
`-O3 -march=znver1 -mtune=znver1` en Release
Auth: MSA Device Code Flow → Xbox Live → XSTS → Minecraft token
SQLite: Qt QSQLITE driver (Qt::Sql, usado en GDLauncherMigrator)
Hashing: MD4/MD5/SHA1/SHA256/SHA512/Murmur2 async

---

## COMMITS

```
c9d13d8  feat: Fase 4.4 — GDLauncher Carbon importer
11bfe87  feat: Fase 4.3+4.5 — CrashReporter + Optimizar botón VersionPage
b5f0c3b  feat: Fase 4.1+4.2 — CheckModConflicts + MalwareScanner
c01b787  feat: Fase 3 completa — BetelineyPacks + presets + RSS
5a70db3  docs: ESTADO.md v2
4174c4d  feat: BetelineyLogAnalyzer — 18 checks
ae1ddd6  fix: Q_INIT_RESOURCE dup, BUILD_TESTING OFF, CurseForge env, BUILD_ARTIFACT CI
2915f18  BetelineyLauncher v8.2.0 — commit inicial
```

---

## RELEASE

```bash
cd "/home/pibe/Descargas/Beteliney Launcher [Minecraft]/BetelineyLauncher/source"
git add -A && git commit -m "descripción"
git tag v8.X.Y && git push && git push --tags
# CI compila Linux + Windows y publica Release en ~15 min
```

---

## API KEYS

| | Estado | Valor |
|---|---|---|
| CurseForge | ✅ | `***CURSEFORGE_KEY_ROTADA_PURGADA***` — CI via `CURSEFORGE_API_KEY` secret |
| Microsoft Azure | ✅ | App ID: `4b945c78-d30b-489e-915f-b361bf9c933b` |
| Imgur | ⚠️ | Key vacía — registrar si se activa screenshots |

---

## ACCIONES MANUALES PENDIENTES

**#1** — Secret CurseForge en CI:
`github.com/ElPibeCapo/BetelineyLauncher/settings/secrets/actions` → `CURSEFORGE_API_KEY`

**#2** — GitHub Pages del META server:
`github.com/ElPibeCapo/meta/settings/pages` → Deploy from branch → `gh-pages` → `/(root)`

**#3** — Crear feed de noticias (opcional, cuando haya noticias que publicar):
Crear `gh-pages/v1/news/feed.atom` en repo meta. El NewsChecker ya lo consume.

**#4** — Crear archivos de BetelineyPacks en repo meta (cuando haya packs):
Crear `gh-pages/v1/beteliney-packs/index.json` y `{id}.json` por cada pack.

**#5** — Crear lista negra de malware:
Crear `gh-pages/v1/malware/known-hashes.json` en repo meta. El MalwareScanner ya la consume.

---

## FASES

### ✅ FASE 0 — Estabilización
`Q_INIT_RESOURCE` dup · `BUILD_TESTING=OFF` · CurseForge key en env · `BUILD_ARTIFACT` en CI.

### ✅ FASE 1 — Motor de diagnóstico de logs (commit 4174c4d)
18 checks, panel visual integrado en LogPage. Ver sección AUDITORÍA.

### ✅ FASE 2 — Selector de perfiles JVM (commit inicial 2915f18)
7 perfiles, auto-detección iGPU, badge GraalVM, warnings RAM. En `JavaSettingsWidget.cpp`.

### ✅ FASE 3 — Ecosistema Beteliney (commit c01b787)

**BetelineyPacks** (`modplatform/beteliney/`):
- `BetelineyPack.h` — structs Pack, PackMod, PackIndex, enum PackProvider
- `BetelineyPackListModel.h/cpp` — descarga index.json + cada pack.json, async, ordena featured primero
- `BetelineyPackInstallTask.h/cpp` — InstanceCreationTask: crea instancia con loader correcto, descarga mods, verifica SHA-512
- `BetelineyPresets.h` — 3 presets built-in sin red: Vanilla Optimizado (Sodium+Lithium+Iris+ModernFix Fabric 1.21.1), PvP Competitivo (Sodium+Lithium+FerriteCore), Modpack Pesado NeoForge
- `BetelineyPackPage.h/cpp/.ui` — UI: lista izquierda, panel derecho, búsqueda, iconos async
- Registrado en NewInstanceDialog como primera pestaña

**RSS propio**: `NEWS_RSS_URL` apunta a `gh-pages/v1/news/feed.atom` (acción manual #3)
`BETELINEY_PACKS_URL` = `https://ElPibeCapo.github.io/meta/v1/beteliney-packs/`

### ✅ FASE 4 — Features avanzados (commits b5f0c3b, 11bfe87, c9d13d8)

**4.1 CheckModConflicts** (`launch/steps/CheckModConflicts.h/cpp`):
- LaunchStep que corre ANTES de que Minecraft inicie, después de ScanModFolders
- Lee todos los .jar con `ModUtils::process(BasicInfoOnly)`, extrae `mod_id`
- Detecta IDs duplicados, los imprime como warnings en el log de lanzamiento
- No bloquea — avisa y continúa

**4.2 MalwareScanner** (`minecraft/mod/MalwareScanner.h/cpp`):
- Singleton, descarga `gh-pages/v1/malware/known-hashes.json` al inicio
- Cachea hashes SHA-256 y SHA-512 en `QSet<QString>`, búsqueda O(1)
- Hook en `ResourceDownloadTask::downloadSucceeded()`: si hash en lista negra → borra archivo y emite error
- Fallo silencioso si no hay red
- `loadIfNeeded()` llamado en `Application::showMainWindow()`

**4.3 BetelineyPanicHandler** (`crash/BetelineyPanicHandler.h/cpp`):
- Linux: `sigaction` para SIGSEGV/SIGABRT/SIGFPE/SIGILL → escribe `/tmp/beteliney_crash_<pid>.txt` con `backtrace_symbols_fd()`
- Windows: `SetUnhandledExceptionFilter` + `MiniDumpWriteDump` (dbghelp, ya en CMakeLists)
- `checkAndShowCrashReport()`: al siguiente inicio busca crashfiles de sesiones anteriores, muestra QDialog con backtrace + botón "Reportar en GitHub" (URL con template pre-llenado), borra el archivo después
- Hook en `main()`: `installPanicHandler()` antes de `Application`
- Hook en `Application::showMainWindow()`: `checkAndShowCrashReport()`

**4.4 GDLauncherMigrator** (`migration/GDLauncherMigrator.h/cpp`) + `GDLauncherMigrateDialog`:
- Detecta `~/.local/share/gdlauncher_next/` (Linux) o `%APPDATA%/gdlauncher_next/` (Windows), 4 rutas candidatas
- Abre `data.sqlite` con Qt QSQLITE, usa `PRAGMA table_info` para detectar schema automáticamente
- Lee id/name/mc_version/modloader/modloader_version/shortpath de la tabla instances
- Importa: crea `instance.cfg` + `mmc-pack.json` en formato Prism, copia `.minecraft/` buscando 3 subrutas posibles
- `GDLauncherMigrateDialog`: lista con selección múltiple, QProgressDialog con cancelación
- Acceso: File → "Importar desde GDLauncher Carbon..."

**4.5 Botón "Optimizar (rendimiento)"** en `VersionPage`:
- Visible solo cuando loader es Fabric/Quilt/LegacyFabric
- Lee preset `builtin-vanilla-optimized` de `BetelineyPresets.h`
- Detecta qué mods del preset faltan en `modsRoot()`
- Pide confirmación, descarga mods faltantes via NetJob + QProgressDialog
- No reinstala mods ya presentes (check por nombre de archivo)

### 🔄 FASE 5 — Distribución profesional (EN PROGRESO)

**5.1 Flatpak** — `dist/com.beteliney.BetelineyLauncher.json`
- Manifest para Flatpak Builder, runtime `org.kde.Platform//6.6`
- CI job `build-flatpak` en `build.yml` (solo en tags)
- Distribuir como `.flatpakref` desde GitHub Pages hasta tener Flathub aprobado

**5.2 SmartScreen Windows**
- Sin firma: Windows bloquea el `.exe` con "Windows protegió tu PC"
- Fix en el NSIS installer: mensaje en pantalla Welcome explicando el bypass
- Fix en el CI: añadir advertencia en el body del GitHub Release

**5.3 AppImage Linux**
- `dist/beteliney.AppDir/` estructura, `appimagetool`
- CI job `build-appimage` en `build.yml`

---

## AUDITORÍA COMPLETA DEL CÓDIGO

### BetelineyLogAnalyzer — 18 checks

| Check | Detecta | Severidad | ActionTarget |
|---|---|---|---|
| checkOutOfMemory | `java.lang.OutOfMemoryError` (heap/GC/Metaspace) | Critical | `"java"` |
| checkHeapReservation | `Could not reserve enough space` | Critical | `"java"` |
| checkDuplicateMod | Duplicate mod Fabric/Forge | Critical | `"mods-folder"` |
| checkMissingDependency | requires mod X / Missing Mods | Critical | `"search-modrinth:X"` |
| checkIncompatibleMods | Incompatible mods | Error | — |
| checkMixinConflict | Mixin transformation failed | Error | — |
| checkFabricIncompatible | versiones incompatibles Fabric | Error | — |
| checkJavaNotFound | Failed to start runtime | Critical | `"java"` |
| checkUnsupportedJavaVersion | UnsupportedClassVersionError | Critical | `"java"` |
| checkForgeJavaRequirement | Forge requires Java | Critical | `"java"` |
| checkOpenGLNotAccelerated | Pixel format not accelerated | Error | — |
| checkOpenGLError | OpenGL errors genéricos | Warning | — |
| checkNativesCrash | exit -1073741819 / SIGSEGV | Error | — |
| checkWindowsLoadLibrary | LoadLibrary failed | Error | — |
| checkNetworkError | Timeout/SSL/connection refused | Warning | — |
| checkFractureiser | Malware Fractureiser | Critical | — |
| checkForgeEarlyWindow | Failed to create early window | Error | — |
| checkForgeCoremods | Coremod error init | Error | — |

### BetelineyProfiles — 7 perfiles JVM

| Idx | Nombre | RAM |
|---|---|---|
| 0 | Personalizado | 0/0 |
| 1 | iGPU/RAM compartida | 384–1536 MB |
| 2 | Ligero Vanilla | 512–2048 MB |
| 3 | Balanceado | 2048–4096 MB |
| 4 | Pesado 100–300 mods | 2048–6144 MB |
| 5 | Extremo ≥300 mods | 6144–12288 MB |
| 6 | iGPU ZGC Java21+ | 384–1536 MB |

En `JavaSettingsWidget.cpp`: auto-detección iGPU via lspci/wmic, badge GraalVM, warnings RAM.
INI keys: `MinMemAlloc`, `MaxMemAlloc`, `JvmArgs`, `OverrideMemory`, `OverrideJavaArgs`.

### Resto del código heredado (todo funcional)
Auth MSA completa · Cuentas offline · Symlinks entre instancias · Modrinth+CurseForge+ATLauncher+FTB+Technic+PackWiz · Java auto-descarga · McClient/McResolver · NSIS installer · BetelineyUpdater · Easter egg B-E-T-E · AnonymizeLog · Imgur (key vacía)

---

## DECISIONES FIJAS

| Decisión | Razón |
|---|---|
| Fork Prism, no reescritura | Auth MSA + Forge processors + plataformas = meses reimplementando. 90% ya existe. |
| Qt Widgets + QSS, no QML | Migrar 100+ vistas = reescribir UI entera. QSS es production-quality. |
| GitHub Pages para META | Gratis, cero mantenimiento. Migrar a Cloudflare Pages si supera 100GB/mes. |
| MSYS2/MinGW64 Windows CI | aqtinstall falló en todos los mirrors. |
| INI + SQLite donde aplica | INI para settings simples. SQLite para GDLauncher migration. |

---

## VERSIONES

Actual: **v8.2.0** · Próxima: **v8.3.0** cuando Fase 5 esté lista.
Patch: bugfixes · Minor: feature completo · Major: cambio arquitectural.
