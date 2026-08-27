# ⬡ BetelineyLauncher — Changelog

> Historial de versiones desde v7.0.0. Antes hubo 6 versiones sin documentar (1 a 6, sin ceros ni puntos) de las que no quedó registro de contenido — esas versiones ya eran del Beteliney actual (base Prism), de antes de adoptar el punto decimal en la numeración; no son una etapa previa basada en Battly. El primer Beteliney (inspirado en Battly, nombre y código) fue reescrito desde cero sobre Prism Launcher por consumir demasiada RAM y no ser como se quería — de esa etapa no quedó ningún registro, ni de código ni de versión (ver `docs/FILOSOFIA_Y_METODOLOGIA.md`).
>
> **Convención de versionado** (nunca estuvo escrita formalmente hasta ahora — queda establecida acá): `1, 2, 3, 4, 5, 6` (sin ceros ni puntos) → `7.0, 7.1, 7.2, 7.3, 7.4, 7.5, 7.6, 7.7, 7.8, 7.9, 8.0, 8.1, 8.2, 8.3` (un solo decimal) → `8.4.0` en adelante (dos decimales, patch siempre en `0`). Así es como se nombran y se dicen las versiones de acá en más. Los tags de git y los headers de este documento usan el formato de tres números completo (`7.0.0`, `8.3.0`) por consistencia técnica interna y para que cada versión tenga un tag único — no es una forma alternativa de la convención, es solo cómo se representa en git/markdown lo mismo que arriba.
>
> Numerado real: `7.0.0 → 7.1.0 → ... → 7.9.0 → 8.0.0 → 8.1.0 → 8.2.0 → 8.3.0 → 8.4.0`. El patch (tercer número) siempre es `0` — nunca se publicó una versión con patch distinto de cero. Entradas que aparecían antes como `7.3.2`, `7.6.1`, `7.8.1`, `7.9.1` y `7.9.2` no fueron releases reales; eran trabajo del mismo ciclo que quedó mal fragmentado en una reconstrucción previa de este documento. Ya están fusionadas en su versión base correspondiente. `v8.3.0` y `v8.4.0` tampoco son dos releases independientes en la práctica: `v8.4.0` se usó para refinar/pulir el contenido ya publicado en `v8.3.0`, no para features nuevas — se mantienen como tags separados en este documento porque así quedaron publicados, pero deben entenderse como el mismo ciclo de trabajo.
> Actualizado: **2026-08-03** · Versión actual en código: **v8.4.0** (tag publicado, pero contiene solo el bump de versión — todo lo de abajo son commits reales en `main` posteriores al tag, sin release nueva todavía). Autor: **El_PibeCapo**

---

## Sin publicar aún — commits en `main` desde `v8.4.0` (2026-07-06 a 2026-08-03)

> Nota: la 8.4.0 no trajo features nuevas, pero tampoco fue solo un bump de versión: entre la 8.3 y la 8.4.0 hay 39 commits con arreglos reales — el crash `free(): invalid pointer` en 6 sitios, cuatro fixes del build de Windows, la condición de carrera de `CopyJars` que rompía el CI en ambas plataformas, tres backports de Prism 11.0.2, los presets con hashes SHA-512 verificados y dos redacciones de la API key de CurseForge. Todo lo real de esta sección vive en `main` sin tag propio todavía — la próxima release debería llevar estos cambios y bumpear a `v8.5.0` (nunca reusar `v8.3.0`/`v8.4.0`).

### Seguridad

| Commit | Cambio |
|---|---|
| `0e0ca32a8` | **`GDLauncherMigrator`** — path traversal: `shortpath`/`id` de `data.sqlite` se usaban sin sanitizar para construir la ruta de origen, permitiendo lectura arbitraria de archivos del sistema disfrazada de importación de instancia. Fix: `safeChildPath()` valida con `QDir::cleanPath()` que la ruta resultante no escape de `instances/`; nombre de directorio destino rechaza también `..` literal (antes solo bloqueaba caracteres sueltos). |
| `288ceeb3a` | **Feed de meta remoto** (`meta/JsonFormat.cpp`) — `uid`/`version` del feed (`ElPibeCapo/meta`) se usaban sin sanitizar para construir rutas de cache local; un feed comprometido podía inyectar `../` y escapar del directorio esperado. Fix: `isSafePathComponent()`/`requireSafePathComponent()`, rechazan cualquier valor con `..`, `/`, `\`, byte nulo, vacío o `>256` caracteres. |
| `174dca213` | **`mmc-pack.json` local + `Require` compartido** (`PackProfile.cpp`, `JsonFormat.cpp`) — el fix anterior solo cubría el feed remoto. `Component::m_uid` de una instancia local (puede venir de un modpack de terceros importado) permitía borrar/escribir archivos arbitrarios vía `customize()`/`revert()`. Peor: `Meta::Require::uid` (compartido por feed remoto, `mmc-pack.json` y `patches/*.json`) se inyecta automáticamente como componente nuevo en cada resolve/launch de instancia sin interacción del usuario — vector más grave que los otros dos por ser automático. Ambos cerrados con la misma validación. |
| `53d092728` | **Firma Ed25519 fail-closed para actualizaciones** — el updater (`BetelineyUpdater`) ahora descarga y verifica una firma `.sig` con clave pública embebida antes de instalar cualquier update; si falta la firma o no valida, borra el archivo y aborta (no hay "instalar de todos modos"). Usa `libsodium` (nueva dependencia vcpkg). CI firma el release con un secret (`RELEASE_SIGNING_KEY`) antes de publicar. |
| `22dd23d0c` | **Límite de 10 redirects en `NetRequest`** — `handleRedirect()` recursaba en `executeTask()` sin límite de profundidad ni detección de bucle; un servidor con redirects circulares (meta comprometido, `MetaURLOverride` hostil, CDN roto) causaba stack overflow y crash. Afecta toda descarga del launcher, no solo meta. Fix: contador con tope duro, reseteado al arrancar una request nueva. |
| `8d6dfe6bc` | **Tope de tamaño en respuestas HTTP acumuladas en memoria** — `ParsingValidator::write()` (parseo de meta, `BaseEntity.cpp`) y `ByteArraySink::write()` (auth, búsquedas de mods, manifests, uploads) no tenían límite superior. Fix: cap de 32 MB y 64 MB respectivamente; no afecta descargas de mods, esas van por `FileSink` a disco. |
| `c4e99152d` | **Path traversal en el importador ATLauncher** (`modplatform/`, cherry-pick de upstream `c4e99152d`) — mismo patrón ya cerrado en el importador GDLauncher y en el feed de meta, pero presente en un rincón de `modplatform/` nunca auditado. Aplicado limpio, sin conflictos. |
| `3cce733b1` | **Path traversal al extraer zips vía symlink/hardlink** (`minecraft/launch/`, `archive/`, cherry-pick de upstream `3cce733b1`) — libarchive podía seguir un symlink/hardlink hostil dentro del zip y escribir fuera del directorio de destino esperado. Aplicado limpio, sin conflictos. |
| `94d705d68` | **Heap overflow real por comparación de versión inestable** (`modplatform/`, cherry-pick de upstream `94d705d68`) — reescribe `Version.cpp`/`.h` completos a un comparador FlexVer con `operator<=>` (C++20). Conflicto real resuelto en `Packwiz.cpp`: el fork ordenaba versiones de mods con `sort()` lexicográfico plano sin dedup (bug real, "1.10" ordenaba antes que "1.9"), reemplazado por el sort semántico + dedup de upstream. Build limpio y `ctest` 31/31 verificados post-aplicación. |
| `8f9b179a4` | **Lectura fuera de rango en `AccountList::data()`** (`minecraft/auth/`) — chequeo de límites `if (index.row() > count())` debía ser `>=`; con `index.row() == count()` pasaba la validación y llamaba `at(index.row())` dos líneas después, un acceso OOB sobre `m_accounts`. Heredado del fork tal cual (`git blame` contra `09eb67f74` confirma que ya estaba en el commit original, y en upstream). Un carácter de fix. Auditoría línea por línea de `minecraft/auth/` completa (21 archivos), primera vez que se audita esa carpeta desde el fork. Build limpio, `ctest` 31/31. |
| `899b7eca4` | **Self-check de deriva de clave en la firma de CI** (`.github/workflows/build.yml`) — antes de firmar los binarios de un release, el workflow ahora extrae la clave pública embebida en `UpdateVerify.cpp` y la compara contra la derivada del secret `RELEASE_SIGNING_KEY`; aborta el release si no coinciden (evita publicar binarios que el updater rechazaría en silencio por clave desincronizada), más una verificación independiente de cada firma generada antes de subirla. Rescatado de un `git stash` de sesión 38 que sobrevivió una purga de historial sin volver a aplicarse — verificado que seguía compatible con el código actual (la clave se rotó después) antes de aplicarlo. |
| `fdd4cee33` | **JSON de login armado por interpolación de string sin escapar** (`minecraft/auth/steps/LauncherLoginStep.cpp`, `XboxAuthorizationStep.cpp`, `XboxUserStep.cpp`) — los 3 construían el body de sus requests con `QString::arg()` sobre un template crudo, sin escapar comillas/backslashes en los tokens interpolados. Heredado del fork. Reemplazado por `QJsonObject`/`QJsonDocument` en los 3, mismo esquema y campos, escapado automático. Build limpio, `ctest` 31/31. Sin login real de punta a punta con cuenta de Microsoft todavía — recomendado antes de la próxima release. |

### Confiabilidad

| Commit | Cambio |
|---|---|
| `290e01071` | **Use-after-free en `BackgroundModUpdateCheckTask`** — `m_instance` era un puntero crudo a `MinecraftInstance`; si el usuario borraba la instancia mientras el chequeo de mods en background seguía en red, el callback final escribía sobre memoria liberada. Fix: `QPointer<MinecraftInstance>` (Qt lo pone en `nullptr` solo al destruirse el objeto) + guard defensivo adicional. |
| `c56a83e55` | **Cuelgue histórico del build local con LTO, causa raíz resuelta (parcial)** — cada link con `-flto=auto` paralelizaba internamente hasta `nproc()` hilos propios; sin límite, Ninja corría varios links en simultáneo multiplicando el paralelismo muy por encima de cores/RAM disponibles, generando swap thrashing (síntoma arrastrado desde sesiones 20/27/29/31/32/33/36). Fix de ese momento: `JOB_POOLS lto_link_pool=2`, limita a 2 links simultáneos — mitigó el cuelgue pero no la causa de fondo, ver `4de61fe13`. |
| `4de61fe13` | **`build-dev/` sin LTO, causa de fondo del punto anterior resuelta** — `lto_link_pool=2` seguía sobre-suscribiendo el CPU al doble (2 links simultáneos × `-flto=auto` = hasta `2×nproc` procesos `lto1` compitiendo por `nproc` núcleos reales), porque `build/` quedaba cacheado como `Release`+`ENABLE_LTO=ON` para cada iteración de desarrollo, no solo para releases. Medido: build completo desde cero en `build/` llegó a 9 horas sin terminar (31/51 targets, load average ~15); el mismo build en `build-dev/` nuevo (`RelWithDebInfo`, LTO off, linker `lld`) tardó 19m51s, 0 errores, 31/31 tests. `tools/dev/build_fast.sh`/`check_build.sh` usan `build-dev/` por defecto ahora; `build/` (Release+LTO real) queda para verificación final pre-release (`build_fast.sh release`). |
| `a753988e0` | **Crash al actualizar datapacks vía Modrinth** (`ModrinthCheckUpdate.cpp`, cherry-pick de upstream `a753988e0`) — `modLoaderTypesToList(*loader).first()` sobre una lista vacía: los datapacks se tratan como `ModLoaderType` internamente pero no son un modloader real, así que para loader=64 (datapack) la lista podía salir vacía y `.first()` sobre `QList` vacío es UB. Fix: chequear `isEmpty()` antes. Build limpio, `ctest` 31/31. |

### Correcciones

| Commit | Cambio |
|---|---|
| `77a199f3a` | **Badge de detección de GraalVM mostraba una cifra de rendimiento sin fuente verificable** (`JavaSettingsWidget.cpp`) — "+10-20% FPS en Minecraft", la misma cifra que ya se había marcado internamente como no verificada, cita mal puesta en un documento externo apuntando a un artículo sin relación con JVM. La documentación se había corregido pero el texto de UI (visible al usuario real) había quedado sin tocar. Se saca el número; queda solo la afirmación técnica real (JIT más agresivo que OpenJDK), sin comprometerse con ninguna cifra hasta medirla en hardware propio. |
| `87b18c060` | **`Launcher_Git` apuntaba a la org vieja** (`program_info/CMakeLists.txt`) — el valor se compila directo al diálogo "Acerca de" real (`AboutDialog.cpp`) y apuntaba a `github.com/beteliney/BetelineyLauncher`, un remoto que no existe. Corregido al remoto real, `ElPibeCapo/BetelineyLauncher`. |

### Nuevas funciones

| Commit | Cambio |
|---|---|
| `7112549d7` | **Backup manual de mundos** — botón "Backup" en la toolbar de `WorldListPage`. Comprime el mundo seleccionado a `backups/worlds/{nombre}_{timestamp}.zip` dentro de la instancia, reusando `BetelineyZip::ExportToZipTask` (mismo mecanismo que `ExportInstanceDialog`). El backup automático antes de actualizar un pack queda pendiente como trabajo aparte. |
| `07be639ab` | **Badge de actualizaciones de mods** — chequeo silencioso en background (`ModrinthCheckUpdate`/`FlameCheckUpdate`) al seleccionar una instancia, una vez por sesión del launcher por instancia. Activa `MinecraftInstance::setUpdateAvailable()`, que ya existía en el código pero no tenía disparador (feature fantasma detectada en sesión 26: la UI ya pintaba el badge, solo faltaba quién lo activara). |
| `484402e48` | **Command palette (Ctrl+K) + servidores favoritos con quick-join** — paleta estilo VSCode/Sublime que recorre el menú principal en vivo con filtro de texto; diálogo de alta/edición/baja de servidores favoritos persistidos en `SettingsObject`; acceso directo a unirse a un favorito sin abrir la instancia primero. |
| `8af24f71a` | **Sistema de logros de marca por tiempo jugado** — 5 logros (1h/10h/50h/100h/500h de juego acumulado en una instancia), notificación visual tipo toast (esquina inferior derecha, cola FIFO si se desbloquean varios a la vez), persistencia en `SettingsObject`. |
| `977a1f83e` | **Sandboxing opcional con Bubblewrap (`bwrap`) para el proceso de Minecraft en Linux** — opción configurable, no forzada por defecto (no todos los sistemas tienen `bwrap`, ni todas las configuraciones de instancia son compatibles con un sandbox estricto). |

### Pendiente — auditoría del 20-21 ago (Manus + verificación propia), sin commit todavía

> No son ideas de backlog (eso vive en `documentacion-proyecto/BIBLIA_MASTER_V2.md`, sección 16). Son hallazgos verificados contra código y feeds reales, con evidencia línea por línea, sin arreglar aún. Mismo patrón que ya se vio y se corrigió antes en esta misma tabla (badge de GraalVM con cifra sin fuente, link a la org vieja) — documentación/feed desincronizado de lo que el código o el servidor realmente hacen, ahora en más lugares.

| Área | Hallazgo | Evidencia |
|---|---|---|
| Malware scanner | Feed público (`meta/v1/malware/known-hashes.json`) vacío en vivo (`sha256: []`, `sha512: []`, `updated: 2026-06-21`) pese a que `README.md` (línea 214, checkbox `[x]`) dice que se sembró con datos reales el 2026-07-18. La siembra nunca llegó a producción o se revirtió. `MalwareScanner` sigue fail-open si el feed falla o está vacío. | Fetch en vivo al feed + grep del README, 21 ago |
| BetelineyPacks | Pack remoto "Vanilla Optimizado" (`meta/v1/beteliney-packs/vanilla-optimizado.json`) tiene 8/8 mods sin `url` ni `sha512`. `BetelineyPackInstallTask` hace `continue` silencioso por cada mod sin url — el pack se instala "exitoso" con 0 mods. Coexiste sin distinción visual, mismo nombre, con el preset built-in bueno de `BetelineyPresets.h` (4 mods, hash real) — el usuario no puede distinguir cuál elige. | Fetch en vivo al feed + lectura de `BetelineyPackInstallTask.cpp` y `BetelineyPackPage.cpp::openedImpl()`, 21 ago |
| Updater | Ruta AppImage (`BetelineyUpdaterApp::run()`, rama `m_isAppimage`) delega a `AppImageUpdate.AppImage` vía `callAppImageUpdate()` sin pasar nunca por `performUpdate()` — se salta enteramente la verificación de firma Ed25519 que sí protege la ruta normal (commit `53d092728`). Aplica igual con canal beta o estable. | Lectura de `BetelineyUpdaterApp.cpp`, 21 ago |
| Telemetría | `TelemetryEnabled` registrado en `true` (opt-out). En `MainWindow.cpp`, `Telemetry::instance().start()` se llama antes de `shouldShowFirstRunNotice()` — el timer del primer heartbeat (5s) queda programado antes de que el usuario vea el aviso, que además es informativo (`QMessageBox::information`, un botón OK), no un consentimiento real. Endpoint sigue siendo el placeholder `REEMPLAZAR.workers.dev`, sin desplegar — hoy no manda nada, pero el orden quedaría igual el día que se despliegue. | Lectura de `BetelineyTelemetry.cpp` + `MainWindow.cpp`, 21 ago |
| UX | `Ctrl+K` colisiona en 3 sitios: `actionKillInstance` (`MainWindow.ui`), Command Palette (`MainWindow.cpp`, commit `484402e48`), y otra vez Kill en `InstanceWindow.cpp`. No hay sistema de atajos configurables por el usuario para resolverlo de raíz. | Búsqueda de texto en los 3 archivos, 21 ago |
| CI | `build.yml` compila con `-DBUILD_TESTING=OFF` en Linux y Windows — los tests nunca corren en CI pese al historial de "ctest 31/31" citado commit por commit en esta misma tabla (esos 31/31 son corridas locales, no de CI). | Lectura de `.github/workflows/build.yml`, 21 ago |
| Documentación | **CORREGIDO 21 ago — este hallazgo estaba mal.** Se había afirmado que `ESTADO.md` y `docs/GESTORES_Y_SKILLS.md` no existían, basado en una búsqueda apuntada a la carpeta padre (`BetelineyLauncher/`) en vez de `BetelineyLauncher/source/`. Ambos existen: `source/ESTADO.md` (358 KB, 3242 líneas, modificado hoy) y `source/docs/GESTORES_Y_SKILLS.md`. Verificado de nuevo con ruta correcta, 21 ago. |

### Verificado 24 ago — path traversal en Flame/ATLauncher, ya resuelto

Una auditoría paralela (`docs/AUDITORIA_DEUDA_TECNICA_20260820.md`, no vista hasta ahora) mencionaba path traversal sin corregir en Flame/ATLauncher. Verificado contra el código actual: **ya está resuelto en ambos** — `FlameInstanceCreationTask.cpp` usa `FS::safeChildPath(m_stagingPath, m_pack.overrides)` (con comentario propio documentando el riesgo, cambio sin commitear todavía, parte del working tree ya conocido en `PENDIENTES.md`), y `ATLPackInstallTask.cpp` tiene `isPathTraversal()` con bloqueo real (`qWarning` + `return false`), ya commiteado de antes. No es pendiente.

**Otros hallazgos de esa auditoría paralela, ya resueltos o confirmados sin acción, no duplico la tabla completa acá — ver el documento original**: `BetelineyCode.h` confirmado huérfano (coincide con lo ya anotado en la Biblia 16.7/16.8), con un dato nuevo — el easter egg que SÍ funciona es el `KonamiCode.h/.cpp` heredado de Prism sin rebrandear (conectado en `MainWindow.cpp:131/315/571`); `telemetry-worker/` ya tiene el código del backend escrito (Cloudflare Worker + esquema SQL), solo falta el deploy manual — el pendiente es más chico de lo que sugería la fila de Telemetría de arriba; `CommandPaletteDialog`/`FavoriteServers` confirmados limpios, sin TODO/FIXME.

### Confirmado en vivo, 24 ago — el asset roto de Windows (pendiente #2 de PENDIENTES.md) afectó a un usuario real

Capturas de WhatsApp del dueño muestran `beteliney.exe` fallando al arrancar en Windows con 4 errores de "DLL no encontrada" en cadena: `libtomlplusplus-3.dll`, `libb2-1.dll`, `Qt6Sql.dll`, `libexpat-1.dll` — el usuario ni siquiera llega a ver la ventana principal, tiene que darle "Aceptar" a cuatro diálogos de error del sistema operativo uno detrás del otro.

**No es un bug nuevo, ni sin arreglo técnico.** El comentario de `EMPAQUETAR_WINDOWS.bat` (agregado en Sesión 61) ya documentaba estas mismas 4 DLLs exactas como el problema que rompió la release v8.4.0 publicada por el CI. La causa: `windeployqt` resuelve las DLL de Qt (incluido `Qt6Sql`, que usa `GDLauncherMigrator` para leer `data.sqlite`), pero no resuelve dependencias de terceros vía MinGW/vcpkg (`tomlplusplus`, `libb2`, `expat`) — para eso existe `tools/dev/win_collect_dlls.sh`, que recorre la tabla de importaciones de forma recursiva y **aborta el build si queda algo sin resolver**.

Verificado contra el workflow real: **`build.yml` ya invoca `win_collect_dlls.sh` en el paso de empaquetado de Windows del CI**, con la verificación de "falla si queda alguna DLL sin resolver" activa. O sea, cualquier build que el CI genere desde que se agregó ese paso debería salir completo. El problema real es el que ya tenías anotado en `PENDIENTES.md` §2: **el asset viejo y roto sigue publicado en la release v8.4.0 de GitHub, nadie lo reemplazó por uno nuevo generado con el CI ya arreglado** — el token del agente no puede borrarlo, es acción tuya. Estas capturas son la primera confirmación real, con un usuario de carne y hueso, de que ese asset roto sigue activo y la gente lo sigue bajando.

**Aparte, sin resolver todavía:** `EMPAQUETAR_WINDOWS.bat` (el script que corre alguien local, sin CI) tiene el mismo agujero sin cubrir del todo — `win_collect_dlls.sh` necesita bash+objdump (MSYS2), que un `.bat` de cmd.exe nativo no tiene disponible, así que solo cubre la mitad (la parte de Qt vía `windeployqt`). El propio comentario lo marca como "sin verificar, no hay máquina Windows disponible para probar este .bat".

### Bugs reales de la comunidad de PrismLauncher (2026), no los 157 TODO/FIXME - 24 ago

Aclaración importante: los 157 TODO/FIXME de 16.9 **no son bugs confirmados**, son deuda de diseño/cosas sin terminar marcadas por los propios devs. Esto de acá sí son bugs reales, reportados y arreglados por la comunidad oficial de PrismLauncher entre v11.0.0 y v11.1.0 (en desarrollo), relevantes por ser el proyecto del que Beteliney es fork.

**Críticos (memoria/freeze/crash), prioridad alta de verificar contra el código propio:**
- `#5551` Memory leak y crash con el modal de data packs — Beteliney tiene `DataPackPage.cpp` (dos, en `ui/pages/modplatform/` e `ui/pages/instance/`) y `ResourceDownloadDialog.cpp`. Sin verificar en detalle si el fix ya está incluido (el diff exacto de Prism no se pudo obtener en esta sesión).
- `#5388` Infinite loop en `SkinManageDialog` — Beteliney tiene su propio `SkinManageDialog.cpp`. **Verificado parcialmente**: los 3 caminos de la espera síncrona (`Task::aborted`, `Task::failed`, `Task::succeeded`) tienen su `quit()` correspondiente sobre `uuidLoop`/`profileLoop` — no se ve el patrón clásico de loop sin salida. No se pudo confirmar al 100% contra el diff real del fix de Prism.
- `#5289` Crash con paquete de red inválido de un servidor — relevante para estabilidad/seguridad de red, sin verificar contra el código de Beteliney todavía.

**Relevantes para el foco de Beteliney (cuentas no-premium, updater propio):**
- `#5542` Cuentas offline no se refrescan al lanzar el juego.
- `#5620` No borrar cuentas Microsoft viejas hasta reautenticar exitosamente.
- `#5492` String de fallo de Xbox incorrecto.
- `#5486` "Fixes to PrismExternalUpdater" — **muy relevante**, Beteliney tiene su propia clase `BetelineyExternalUpdater` heredada de esa misma base. Sin verificar el diff exacto de qué se arregló.
- `#5370` Updater no reseteaba la tarea actual en la señal de "terminado" (v11.0.2, parche de emergencia sobre un bug que la propia v11.0.1 había introducido).
- `#5334` No contar mods .jar al chequear librerías offline.

**En desarrollo ahora mismo en Prism (v11.1.0, todavía no publicado), ideas que ya veníamos proponiendo de forma independiente:**
- `#5886` "Add warning for untrusted mods in imported modpacks" — coincide casi exacto con la idea de "Modo Confianza" propuesta en 16.5. Prism está construyendo algo similar en paralelo, ahora mismo.
- "Avoid excessive config writes (less freezing especially on HDD!)" — optimización de rendimiento real.
- `#5860` "Cache resource hard link count" — relacionado con la deduplicación de mods entre instancias que había quedado como "verificar si ya existe" en 16.2.

**Otros, menor prioridad o no aplicables:** `#5387` (botones de copiar/subir en ScreenshotsPage), `#5717` (dependencias no habilitan/deshabilitan otras), `#5415` (lógica de abortar tareas), `#5485` ("usar idioma del sistema"), `#5204` (sacar caracteres de marca registrada de nombres de carpeta de instancia), `#5332` (build en 32 bits, no aplica), `#5427` (Cmd+Q en macOS, no aplica - Beteliney es Linux/Windows).

**Pendiente real**: ninguno de estos se verificó al 100% contra el código de Beteliney salvo el de SkinManageDialog (parcial). Antes de decidir cuáles portar, habría que conseguir el diff real de cada PR de Prism (no se pudo traer completo en esta sesión, GitHub devolvía solo metadata) y compararlo línea por línea contra el archivo equivalente de Beteliney.

### Ampliación 24 ago — changelogs completos de v11.0.0 y v11.0.3 traídos íntegros (no solo snippets de búsqueda)

Se trajeron las páginas oficiales completas de prismlauncher.org (v11.0.0, 7 min de lectura, ~55 PRs; v11.0.3, ~35 PRs). Además de lo ya listado arriba:

**Críticos nuevos, sin verificar contra Beteliney:**
- `#4822` "Do not delete LaunchController before we're done using it" — use-after-free real, confirmado por el propio título del PR. Beteliney tiene `LaunchController.h/.cpp` (heredado). **No se pudo confirmar si el fix está incluido** — sin diff exacto disponible en esta sesión. Prioridad alta de verificar manualmente, es del mismo tipo de bug que ya se corrigió antes en Beteliney (ver commit `53d092728`/fixes previos documentados más arriba en este changelog) y del mismo tipo que el segfault sin resolver (pendiente #1, `ESTADO.md` privado).
- `#5301` "LaunchController: fix double task finish" — condición de carrera de doble finalización, mismo archivo que arriba.
- `#5187` "Fix infinite update" — loop infinito en el flujo de actualización (distinto del updater con Ed25519 que ya se auditó, pero relacionado).
- `#5000` "fix crash with skin format" — otro crash de skins, además del de `SkinManageDialog` ya listado.
- `#5418` "fix world size uninitialized memory and UI refresh signal" — lectura de memoria sin inicializar, riesgo real de comportamiento indefinido.
- `#5050` "Use an owning QByteArray in ByteArraySink" — bug de lifetime de buffer.
- `#5043` "fix(flame): avoid double-fail assert in file resolving" — assert duplicado en el resolver de archivos de CurseForge, misma área que se investigó con la conversación sobre Shiwa.
- `#5034` "fix zip path traversal" (genérico, el reportado por el investigador externo Thinkseal) — **distinto** del `#5511 "fix Atl path traversal"` ya confirmado presente en Beteliney. Este es el fix base de extracción de ZIP; Beteliney usa `FS::safeChildPath` en Flame, coherente con este fix, pero no se confirmó línea por línea que sea exactamente este PR.

**Relevante y ya verificado, contradice parcialmente un hallazgo propio anterior:**
- `#4622` "Ask before deleting saves as part of a Modpack update" (Prism 11.0.0) — Prism ya pregunta antes de borrar mundos guardados al actualizar un pack. **Verificado contra el TODO real de Beteliney en `FlameInstanceCreationTask.cpp:170-171`**: ese TODO sigue intacto y trata de un problema relacionado pero distinto — "overrides" (configs/resourcepacks del modpack) y mods deshabilitados por el usuario, no específicamente los `saves/`. No se pudo confirmar si Beteliney heredó el fix de `#4622` en otro punto del flujo de actualización sin revisar más archivos.

**Contexto adicional de por qué existe el path traversal fix en primer lugar**: `#5034`/el fix original fue reportado responsablemente por un investigador de seguridad externo (Thinkseal), no encontrado internamente — dato de color, refuerza que la comunidad de Prism sí recibe auditorías de seguridad externas reales.

**Otros de menor prioridad traídos en esta ampliación**: `#5513` (no borrar directorios base al vaciar el metacache — riesgo de pérdida de datos), `#5442`/`#5444` (rutas vacías y espacios en blanco mal manejados en URLs de packs), `#5216` (tarea de actualización de componente no se sobreescribe bien con modo de red distinto), `#5645` (mostrar error cuando falla la autorización por código de dispositivo), `#5602`/`#5487` (uso de APIs nativas para detectar GPU — referencia técnica real para la idea de auto-perfil JVM por hardware ya en el backlog).

### CONFIRMADO con solución concreta, 24 ago — use-after-free real en `LaunchController` (`#4822` de Prism)

No es conjetura, es lectura directa del código con razonamiento técnico verificado paso a paso:

1. `Application.cpp`: `connect(controller.get(), &LaunchController::finished, this, &Application::controllerFinished);` — sin `Qt::QueuedConnection` explícito. Emisor (`controller`) y receptor (`Application`) están en el mismo hilo (el hilo principal de la GUI) → Qt resuelve esto como conexión **directa/síncrona**.
2. Cuando `LaunchController` hace `emit finished()` desde dentro de alguna de sus propias funciones, Qt llama **inmediatamente**, todavía en el mismo call stack, a `Application::controllerFinished()`.
3. Esa función hace `extras.controller.reset()` (línea 1708) — **destruye el objeto `LaunchController` mientras el código que hizo el `emit` original todavía sigue en el stack, sin terminar de ejecutarse.**
4. Cuando el control vuelve a la función de `LaunchController` que hizo el `emit`, corre sobre un objeto ya destruido — use-after-free clásico de Qt/C++, exactamente el patrón que describe el título del PR de Prism `#4822` ("Do not delete LaunchController before we're done using it"). Beteliney tiene el código idéntico, sin el fix.

**Solución concreta, patrón estándar de Qt para este caso exacto** (cambio de una línea, bajo riesgo):
```cpp
// Application.cpp:1708, en vez de:
extras.controller.reset();
// usar:
extras.controller.release()->deleteLater();
```
`release()` desengancha el objeto del `unique_ptr` sin destruirlo; `deleteLater()` le pide a Qt que lo borre recién en el próximo ciclo del event loop, después de que el `emit finished()` original termine de desenrollar su stack por completo. Sin efectos secundarios esperados sobre el resto de la función (el resto de `controllerFinished()` no vuelve a tocar `extras.controller` después de esa línea).

**Relevancia real**: este es exactamente el tipo de bug que puede producir crashes intermitentes y difíciles de reproducir — coherente en naturaleza (aunque no confirmado como la misma causa) con el segfault sin resolver documentado como pendiente #1 en `ESTADO.md` privado. Vale la pena probarlo como primer candidato a investigar ese segfault, dado que es una causa raíz real ya identificada con solución lista, en vez de seguir a ciegas.

### Cronología completa + v11.0.1/11.0.2/11.1.0 traídas - 24 ago

Cronología real (fechas 2026): 10.0.5 (14 feb) → **11.0.0** (8 abr, "our fastest major release yet") → **11.0.1** (12 abr, parche crítico "para lanzar versiones viejas de Minecraft y actualizar Modpacks") → **11.0.2** (13 abr, parche de emergencia de un día para después — el propio anuncio dice "hicimos suficientes releases como para romper sutilmente nuestro updater con la última" — el sistema de actualización automática de Prism se rompió a sí mismo y tocó pedirle a la gente de Windows/AppImage/Linux Portable que actualizara a mano) → **11.0.3** (11 jul, ~35 fixes) → **11.1.0** en testing por Discord, sin publicar todavía. Dato de color: v10.0.3 y v10.0.4 nunca llegaron a publicarse, ni Prism se salva de releases fallidas.

**v11.0.1 (parche crítico, 4 días después de 11.0.0):**
- `#5333` Permitir desactivar el aviso de RAM baja.
- `#5334` No contar mods .jar al chequear librerías offline (ya listado).
- `#5332` Build roto en arquitecturas de 32 bits (no aplica a Beteliney).

**v11.0.2 (parche de un solo cambio, emergencia del updater):**
- `#5370` "Updater: Do not reset current task in finished signal" — el único cambio de todo el release, sobre el propio sistema de actualización. Máxima relevancia para el `BetelineyExternalUpdater`/`BetelineyUpdater` ya auditado — confirma que el patrón "updater que se rompe a sí mismo entre versiones" es un riesgo real y ya materializado en el proyecto del que Beteliney es fork.

**v11.1.0 (en testing, lista completa):**
- `#5886` Warning para mods no confiables (ya listado, coincide con "Modo Confianza" propuesto en 16.5).
- `#5662` "Added missing tab stops and/or fixed their order" — accesibilidad de navegación por teclado (orden de Tab), distinto de la idea de "atajos configurables" ya en el backlog — vale sumarlo como idea de accesibilidad adicional en 16.6.
- `#5858`/`#5945`/`#5913` "Avoid excessive config writes" — 3 PRs para el mismo problema, sugiere que costó varios intentos resolverlo bien. Relevante en general para el patrón de guardado de settings de Beteliney.
- `#5860` Cache resource hard link count (ya listado).
- `#5818` "Fix existing instance shortcuts not being detected", `#5816` "Fix instance shortcuts breaking when the instance is renamed", `#5822` "Prevent renaming instance folder while instance is running" — 3 bugs relacionados a accesos directos de instancia (shortcuts de escritorio) y renombrado, área no explorada todavía en Beteliney.
- **`#5779` "fix: crash when trying to install a disabled modloader"** — crítico, sin verificar contra Beteliney. Relevante porque conecta con el TODO ya encontrado en `FlameInstanceCreationTask.cpp:93` sobre manejo pobre de instalaciones duplicadas/en conflicto.
- `#4997` Colores del menú difíciles de leer en macOS (no aplica).

### Investigado y descartado, 24 ago — falso positivo en `World::m_gameType`

Por inspección propia (no de un PR de Prism) sospeché memoria sin inicializar en `World.h`: el campo `GameType m_gameType;` no tiene `=` explícito, a diferencia de los demás campos de la clase (`m_size = 0`, `m_randomSeed = 0`, `m_isValid = false`), y confirmé que `World::readFromFS()`/`readFromZip()` retornan antes de asignarlo en caminos de error (`level.dat` ilegible o ausente). **Seguí la cadena hasta el final y no es un bug**: `struct GameType` (definido en el mismo header) tiene `enum { Unknown = -1, ... } type = Unknown;` con inicializador in-class, y `GameType() = default;` respeta ese inicializador — `m_gameType` siempre arranca en `Unknown` de forma segura, aunque no se vea a simple vista en el punto de declaración. Descartado con evidencia, no solo por sospecha.

### Cierre de esta ronda de "bugs, errores, fallas, cómo solucionar" - 24 ago

**Con solución concreta lista para aplicar (1)**: el use-after-free de `LaunchController` de arriba — cambio de una línea, `extras.controller.release()->deleteLater();` en vez de `.reset()`.

**Confirmados sin bug tras investigar a fondo (3)**: path traversal en Flame/ATLauncher (ya resuelto), memoria de `World::m_gameType` (falso positivo, recién descartado), SkinManageDialog (los 3 caminos de error cubiertos, aunque sin diff exacto de Prism para comparar).

**Sin verificar todavía, quedan como pendiente explícito para la próxima ronda si se retoma**: crash con modloader deshabilitado (`#5779`), doble finalización de tarea en LaunchController (`#5301` — relacionado al use-after-free ya confirmado, mismo archivo, vale la pena revisar junto), loop infinito de actualización (`#5187`), bug de `ByteArraySink` (`#5050`), assert doble en Flame file resolving (`#5043`).

---


### Herramientas, pruebas y mantenimiento

| Commit | Cambio |
|---|---|
| `3a4153f4f` | **Permisos de mínimo privilegio en el CI** (`.github/workflows/build.yml`) — el workflow declaraba permisos amplios por defecto; ahora cada job pide solo lo que necesita, y el de release es el único con `contents: write`. |
| `db87d13d2` | **Sistema de telemetría opt-in** — latido anónimo con UUID aleatorio (`QUuid::createUuid()`, no derivado del hardware), versión, sistema operativo y dos contadores de tiempo. No envía IP, cuenta de Minecraft ni nombres de instancias. Documentado en `docs/TELEMETRIA.md`, junto con `SECURITY.md`, `CODE_OF_CONDUCT.md` y plantillas de issue/PR. |
| `636b485ae` | **Verificaciones previas a push y auditoría de repo** (`tools/dev/`) — checklist que corre antes de cada push y `secret_scan` con modo por-archivos, para no depender de acordarse. |
| `74fb83c6b` | **Segunda tanda de cobertura de `minecraft/auth/`** — `AccountData_test.cpp` y `MinecraftAccount_test.cpp`, la carpeta que en la auditoría anterior había resultado ser la menos cubierta. |
| `a2585e6ec` | **Autoría y copyright** a nombre de El_PibeCapo, con el año de fundación corregido. |
| `afa509097` | **`tools/dev/` acotado a los scripts de uso público**, con README propio. |
| `bea5c2333` | **Imagen de social preview** (1280×640) para las tarjetas de enlace del repositorio. |

### Documentación

| Commit | Cambio |
|---|---|
| `b030054ee` | **Convención de versionado escrita formalmente** por primera vez — nunca había quedado documentada. |
| `f2092e6ad` | **Contexto de versionado y origen del proyecto** — cierra el hueco de por qué la numeración es la que es. |
| `abcaddea9` | **Reorganización completa de la documentación** — `ESTADO ACTUAL` estaba desactualizado 4 sesiones, el README tenía 2 items obsoletos y `AUDITORIA_MODULOS` no reflejaba la cobertura de tests. |
| `0d1d5ad1c` | **Índice de documentación** (`docs/README.md`) y limpieza de archivos sueltos. |
| `d1e6887a6` | **Cambio de método de adopción de upstream** — diff contra la punta reemplaza al cherry-pick por commit histórico. |
| `2e576a008` | **Corrección de un hallazgo previo** — el cluster de ~46 commits pendientes de upstream son al menos 2 features sin adoptar, no un solo refactor. |
| `9e9656a3b` | **Investigación de FjordLauncher y ShatteredPrism** — alt-auth, DRM/Client ID y FTB.


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
