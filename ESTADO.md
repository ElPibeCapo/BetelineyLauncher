# ESTADO — BetelineyLauncher
> Documento único de referencia. Autocontenido. Cualquier chat nuevo arranca desde aquí.
> Actualizar al cerrar cada sesión. Reemplaza DEVLOG.md y MASTER_PLAN.md.

---

## IDENTIDAD

| | |
|---|---|
| **Nombre** | BetelineyLauncher |
| **Versión** | v8.2.0 |
| **Base** | Prism Launcher (GPL-3.0), fork extensamente modificado |
| **Autor** | El_PibeCapo — `elpibecapoofficial@gmail.com` |
| **Repo** | https://github.com/ElPibeCapo/BetelineyLauncher |
| **Rama** | `main` |
| **META server** | https://ElPibeCapo.github.io/meta/v1/ |
| **Repo meta** | https://github.com/ElPibeCapo/meta |
| **Código fuente** | `/home/pibe/Descargas/Beteliney Launcher [Minecraft]/BetelineyLauncher/source/` |

---

## STACK

C++20 · Qt 6 (Widgets + QSS) · CMake 3.25+ · Ninja · GitHub Actions
Linux: Ubuntu 24.04 CI · MinGW-w64/MSYS2 para Windows · `-O3 -march=znver1`
Librerías: libarchive · zlib · zstd · lz4 · lzma · bz2 · libcmark · libqrencode · tomlplusplus · libxml2
Auth: MSA Device Code Flow → Xbox Live → XSTS → Minecraft token
Hashing: MD4/MD5/SHA1/SHA256/SHA512/Murmur2 (async) — Murmur2 para CurseForge, SHA512 para Modrinth

---

## QUÉ EXISTE EN EL CÓDIGO (auditoría completa)

### Completo y funcionando

**Visual**
- `BetelineyTheme.cpp` — 810 líneas QSS. Paleta deep-space `#080912` / neón `#39FF14` / cyan `#00D4FF` / fuente JetBrains Mono. 5 iteraciones (backups `.v3bak`, `.v4bak`). Cards con `drawGamerCard()` — bordes neón, hover cyan, QPainter nativo. Todos los widgets cubiertos.

**JVM — BetelineyProfiles.h**
- 7 perfiles con flags documentados: iGPU (384-1536MB), iGPU ZGC Java21+ (pausas <1ms), Ligero Vanilla (512-2048MB), Balanceado (2-4GB), Pesado 100-300 mods (2-6GB), Extremo ≥300 mods (6-12GB).
- Aikar's Flags calibrados con G1HeapRegionSize ajustado por heap. Superan a Prism y GDLauncher Carbon.
- **Estado: struct existe, UI no conectada aún** → Fase 2.

**Updater**
- `BetelineyUpdater.h/cpp` — GitHub Releases API, semver, pre-releases, canal beta, AppImage, backup diferencial.
- `BetelineyExternalUpdater` — timer auto-check. `GitHubRelease.h/cpp` — parser JSON.
- `Launcher_BUILD_ARTIFACT` configurado en CI → updater se compila en builds de CI.

**Instalador Windows**
- `win_install.nsi.in` — NSIS, 516 líneas, 60+ idiomas incluido SpanishInternational.
- URL handlers: `curseforge://`, `beteliney://`, `beteliney-import://`. Asocia `.mrpack` y `.zip`.
- Start Menu + Desktop + Uninstaller completo.

**Auth y cuentas**
- `AccountType { MSA, Offline }` — ambas implementadas y funcionales.
- Cadena MSA completa: MSAStep → MSADeviceCodeStep → XboxUserStep → XboxAuthorizationStep → XboxProfileStep → EntitlementsStep → MinecraftProfileStep → GetSkinStep.

**Plataformas de mods**
- Modrinth: API + CheckUpdate + InstanceCreation + PackExport + PackIndex.
- CurseForge/Flame: API + CheckUpdate + InstanceCreation + FileResolving + PackManifest.
- También: ATLauncher, FTB, Legacy FTB, Technic, PackWiz.

**Logs**
- `LogParser.cpp` — parser XML log4j + texto plano legacy, coloreado por nivel, `anonymizeLog()`.
- `BetelineyLogAnalyzer.h/cpp` — **18 checks implementados** (ver Fase 1 completada abajo).
- `LogPage.ui/h/cpp` — panel de diagnóstico integrado visualmente.

**Java auto-descarga**
- `java/download/`: ArchiveDownloadTask, ManifestDownloadTask, SymlinkTask.
- `java/JavaMetadata.h`: vendor, URL, checksum, versión, OS.
- `Launcher_ENABLE_JAVA_DOWNLOADER`: OFF en Linux por defecto (compatibilidad distros), ON en Windows.

**Otros**
- `McClient.h/cpp` + `McResolver.h/cpp` — ping de servidores Minecraft sobre TCP (MOTD, versión, jugadores).
- `BetelineyCode.h` — easter egg: escribir B-E-T-E en MainWindow.
- Setup wizard: AutoJava, Java, Language, Login, Theme, Paste pages.
- `HashUtils`: Murmur2/SHA512/MD4/MD5/SHA1/SHA256/SHA512 async.

### Incompleto / desconectado

| Qué | Estado | Fix |
|---|---|---|
| Perfiles JVM en UI | Struct existe, sin widget | Fase 2 |
| NewsChecker | Apunta a releases.atom de GitHub | Fase 3: RSS propio |
| Imgur upload | Código existe, key vacía | Decidir si se activa |
| macOS | Código heredado de Prism | Sin CI, sin mantenimiento |

### No existe aún

| Feature | Fase |
|---|---|
| BetelineyPacks (plataforma propia de modpacks) | 3 |
| Detección de conflictos de mods pre-lanzamiento | 4 |
| Verificación hash de mods vs base de malware | 4 |
| Inyección automática Sodium+Lithium+Starlight | 3 |
| Crash reporter del launcher mismo (opt-in) | 4 |
| Importar desde GDLauncher Carbon | 4 |
| Flatpak oficial | 5 |
| Firma de código Windows | 5 |

---

## COMMITS (más recientes primero)

```
44ee765  docs: MASTER_PLAN — Fase 1 completada
4174c4d  feat: BetelineyLogAnalyzer — motor diagnóstico logs v1.0
bb3ba44  docs: MASTER_PLAN actualizado sesion 2
ae1ddd6  fix: Q_INIT_RESOURCE duplicado, BUILD_TESTING OFF, CurseForge key via env, BUILD_ARTIFACT en CI
16bd9bf  fix: Q_INIT_RESOURCE(multimc) -> beteliney_icons
0b4b519  ci: BUILD_TESTING=OFF + verbose ninja
a9a98cb  fix: add_custom_command -> add_custom_target (CMake 4.x)
255dfe5  ci: stub gamemode + quitar java-openjdk-devel
a424fb3  ci: msys2+mingw64 en windows
e894a01  config: META_URL propio, URLs limpias
9828ea0  ci: GitHub Actions Linux + Windows
2915f18  BetelineyLauncher v8.2.0 — commit inicial
```

---

## CÓMO HACER UNA RELEASE

```bash
cd "/home/pibe/Descargas/Beteliney Launcher [Minecraft]/BetelineyLauncher/source"
git add -A
git commit -m "descripción"
git tag v8.3.0
git push && git push --tags
# CI compila Linux + Windows y publica la Release en ~15 min
```

---

## API KEYS Y SERVICIOS

| Servicio | Estado | Valor / Notas |
|---|---|---|
| CurseForge | ✅ Key propia | En CI via secret `CURSEFORGE_API_KEY`. Localmente: `export CURSEFORGE_API_KEY="$2a$10$wIJoe..."` |
| Microsoft Azure | ✅ App registrada | App ID: `4b945c78-d30b-489e-915f-b361bf9c933b` |
| Imgur | ⚠️ Vacío | Registrar en api.imgur.com/oauth2/addclient si se activa |
| META server | ✅ Contenido generado | GitHub Actions corre cada 6h, último OK: 2026-06-16 05:05 UTC |

---

## ACCIONES MANUALES PENDIENTES (requieren browser con tu sesión GitHub)

**#1** — Secret CurseForge en CI:
`https://github.com/ElPibeCapo/BetelineyLauncher/settings/secrets/actions`
→ New repository secret → Nombre: `CURSEFORGE_API_KEY` → Valor: `***CURSEFORGE_KEY_ROTADA_PURGADA***`

**#2** — Activar GitHub Pages del META server:
`https://github.com/ElPibeCapo/meta/settings/pages`
→ Source: "Deploy from a branch" → Branch: `gh-pages` → `/ (root)` → Save
(El contenido ya está generado en la rama `gh-pages`. Solo falta activarlo.)

---

## VERSIONES

| Tipo | Criterio |
|---|---|
| Patch (x.x.+1) | Bugfixes, cambios menores |
| Minor (x.+1.0) | Feature nuevo completo |
| Major (+1.0.0) | Cambio arquitectural o fase completa |

Actual: v8.2.0 · Próxima minor: v8.3.0 (LogAnalyzer ✅ + perfiles JVM UI)

---

## FASES DE DESARROLLO

### ✅ FASE 0 — Estabilización (COMPLETADA)

- [x] `Q_INIT_RESOURCE(beteliney_icons)` duplicado eliminado de `main.cpp`
- [x] `BUILD_TESTING=OFF` como default en CMakeLists
- [x] CurseForge API key sacada del código → lee `$ENV{CURSEFORGE_API_KEY}`
- [x] CI inyecta key desde `secrets.CURSEFORGE_API_KEY`
- [x] `Launcher_BUILD_ARTIFACT` configurado en CI (activa auto-updater)
- [x] META server verificado: `gh-pages` tiene net.minecraft, Forge, NeoForge, Fabric, Quilt, Java Adoptium/Azul/IBM
- [ ] Acción manual #1: secret en GitHub (ver arriba)
- [ ] Acción manual #2: GitHub Pages en repo meta (ver arriba)

---

### ✅ FASE 1 — Motor de diagnóstico de logs (COMPLETADA · commit 4174c4d)

**Archivos creados/modificados:**
- `launcher/logs/BetelineyLogAnalyzer.h` — interfaz + struct `Diagnosis` con severity, title, explanation, solution, detail, affectedMod, actionLabel, actionTarget
- `launcher/logs/BetelineyLogAnalyzer.cpp` — 639 líneas, 18 checks implementados
- `launcher/ui/pages/instance/LogPage.ui` — panel `diagnosisPanel` insertado entre controles y log (row 1), search bar movida a row 3
- `launcher/ui/pages/instance/LogPage.h` — include + slots + miembros `m_diagnoses`, `m_diagnosisIndex`
- `launcher/ui/pages/instance/LogPage.cpp` — integración completa: connect a `Task::finished`, `showDiagnosis()`, `hideDiagnosis()`, slots de acción
- `launcher/CMakeLists.txt` — BetelineyLogAnalyzer registrado

**Los 18 checks:**

| Check | Detecta |
|---|---|
| `checkOutOfMemory` | OOM: Java heap space / GC overhead / Metaspace |
| `checkHeapReservation` | "Could not reserve enough space" / "Unable to create native thread" |
| `checkDuplicateMod` | Mod duplicado en Fabric y Forge/NeoForge, extrae nombre del mod |
| `checkMissingDependency` | "requires mod X to be loaded" / "Missing Mods:" |
| `checkIncompatibleMods` | "Incompatible mods found" / "conflicts with mod" |
| `checkMixinConflict` | "Mixin transformation failed" / "MixinApplyError" |
| `checkFabricIncompatible` | Versiones incompatibles de Fabric loader |
| `checkJavaNotFound` | "Failed to start the minecraft runtime" / "No such file" en JVM |
| `checkUnsupportedJavaVersion` | "UnsupportedClassVersionError" — Java muy viejo |
| `checkForgeJavaRequirement` | "Forge requires Java" — versión incorrecta para Forge |
| `checkOpenGLNotAccelerated` | "Pixel format not accelerated" — GPU/driver |
| `checkOpenGLError` | OpenGL errors genéricos — iGPU / driver issue |
| `checkNativesCrash` | Exit code -1073741819 (Windows AV) / SIGSEGV |
| `checkWindowsLoadLibrary` | "LoadLibrary failed" — DLL faltante |
| `checkNetworkError` | Timeout / SSL / connection refused durante auth |
| `checkFractureiser` | Hashes y firmas conocidos del malware Fractureiser |
| `checkForgeEarlyWindow` | "Failed to create early progress window" |
| `checkForgeCoremods` | Coremod error durante init de Forge |

**Comportamiento del panel:**
- Se activa solo cuando `gameExitCode != 0`
- Severidad visual: Critical=rojo, Error=naranja, Warning=amarillo, Info=cyan
- Múltiples diagnósticos navegables con contador "1/3", "2/3"...
- Botón de acción: abre Settings Java, abre carpeta mods, busca en Modrinth
- Botón X para descartar
- Se oculta y reinicia al lanzar una nueva sesión de juego

---

### 🔄 FASE 2 — Selector de perfiles JVM en UI (SIGUIENTE)

**Objetivo:** exponer `BetelineyProfiles.h` en la interfaz. El usuario selecciona perfil → se auto-rellenan Xmx, Xms y los JVM args.

**Archivos a modificar:**
- `launcher/ui/pages/instance/InstanceSettingsPage.ui/.cpp` — ComboBox de perfil en la sección Java
- `launcher/ui/pages/global/JavaPage.ui/.cpp` — mismo ComboBox para configuración global
- `launcher/BetelineyProfiles.h` — añadir método helper `suggestProfile(int ramMB)` que devuelve el índice del perfil más adecuado según RAM del sistema

**Implementación detallada:**

En ambas páginas, añadir debajo de los campos de memoria:
```
[ComboBox: Perfil JVM ▼]  ← lista los 7 perfiles de BETELINEY_PROFILES
[Label: descripción del perfil seleccionado]
[Label: ⚠ advertencia si Xmx > 60% RAM]   ← solo si aplica
```

Al seleccionar perfil → `onProfileChanged(int index)`:
1. Leer `BETELINEY_PROFILES[index]`
2. Setear `ui->maxMemSpinBox->setValue(profile.defaultXmx)`
3. Setear `ui->minMemSpinBox->setValue(profile.defaultXms)`
4. Append los JVM args del perfil al campo de argumentos (reemplazar los anteriores del perfil si ya estaban)
5. Mostrar descripción debajo del combo

En el constructor, llamar a `suggestProfile(HardwareInfo::getRAMTotalMB())` para pre-seleccionar el perfil adecuado si el usuario no eligió uno aún.

**Verificar antes de empezar:**
- Leer `InstanceSettingsPage.ui` para ver la estructura actual del layout Java
- Leer `JavaPage.ui` para ver la estructura actual
- Verificar nombres de los spin boxes de memoria (Xmx, Xms)
- Verificar cómo se guarda/carga la config de JVM args (INI, settings key)

---

### FASE 3 — Ecosistema Beteliney

**3.1 BetelineyPacks** — plataforma propia de modpacks servida desde GitHub Pages
- Formato JSON: `{id, name, description, version, minecraft, loader, loader_version, mods[{provider, id, version}]}`
- Nueva pestaña en diálogo "Nueva instancia": lista, descripción, screenshot, instalar
- Backend: JSON estático en rama `gh-pages` del repo meta, o repo separado

**3.2 RSS propio** — reemplazar `releases.atom` con un feed de noticias real
- JSON estático generado con GitHub Actions
- Mostrado en MainWindow con imagen, categoría, texto

**3.3 Presets de instancia por tipo de juego**
- `Vanilla Optimizado`: instala Sodium + Lithium + Starlight automáticamente en instancias Fabric
- `PvP Competitivo`: JVM flags agresivos
- `Modpack Pesado`: perfil JVM "Pesado" + aviso RAM

---

### FASE 4 — Features avanzados

- Verificación hash de mods vs API Modrinth/CurseForge + lista negra Fractureiser
- Detección conflictos de mods pre-lanzamiento (leer fabric.mod.json / mods.toml)
- Crash reporter del launcher opt-in (captura stack trace, envía a GitHub Issues)
- Importar instancias desde GDLauncher Carbon

---

### FASE 5 — Distribución profesional

- Flatpak oficial → Flathub
- Firma de código Windows (EV cert o workaround documentado en SmartScreen)
- AppImage
- macOS: job CI + notarización Apple (bajo prioridad)

---

## DECISIONES TÉCNICAS FIJAS

| Decisión | Por qué |
|---|---|
| Fork de Prism, no reescritura en Rust/Tauri | Auth MSA + Forge processors + plataformas de mods = meses de reimplementación. El 90% ya existe y funciona. |
| Qt Widgets + QSS, no migrar a QML | Migrar 100+ vistas = reescribir la UI entera. El QSS actual es production-quality. |
| GitHub Pages para META server | Gratis, auto-deploy, suficiente. Migrar a Cloudflare Pages si se superan los 100GB/mes. |
| MSYS2/MinGW64 para Windows CI | aqtinstall falló en todos los mirrors. MSYS2 es confiable y mantenido. |
| INI files para settings | SQLite cuando se necesite persistir historial de mods / caché de búsquedas. Por ahora no. |
