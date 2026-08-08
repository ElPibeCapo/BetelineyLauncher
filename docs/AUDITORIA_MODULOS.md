# Auditoría de Módulos — Registro de Estado de Revisión

> Documento vivo. Se crea el **2026-07-19** a partir de un fallo real: durante 48 sesiones de trabajo,
> ninguna documentación distinguía "módulo verificado" de "módulo heredado de Prism y nunca mirado" —
> lo que llevó a asumir por defecto que todo estaba en orden simplemente porque nadie había reportado
> un problema. `minecraft/auth/` (login Microsoft, manejo de tokens de sesión) llevaba **cero commits
> de revisión desde el fork**, y nadie lo había documentado como hueco hasta que se preguntó explícitamente.
> Este documento existe para que esa ambigüedad no vuelva a pasar desapercibida.

> **Nota de integridad de hashes:** el commit `dd7ae4354` purgó con `git filter-repo` parte del historial de docs internos. Algunos hashes citados acá abajo pueden no ser reconstruibles por ese motivo — no asumir error de tipeo sin chequear primero contra `dd7ae4354` y su fecha.

## Regla de mantenimiento — leer antes de tocar cualquier carpeta de `launcher/`

Si una carpeta no aparece acá con evidencia de revisión (commit real + fecha + qué se encontró), se
trata como **NO REVISADA**, nunca como "probablemente bien". Cualquier sesión que audite, toque o
descarte una carpeta debe actualizar su fila. No se permite marcar "revisado" sin un commit real o una
verificación reproducible (comando corrido, resultado observado) que lo respalde — ninguna entrada de
esta tabla se basa en memoria ni en suposición. Mismo estándar que ya rige el resto de la documentación del proyecto.

## Metodología

Para cada subcarpeta de `launcher/`: `git log --oneline 09eb67f74..HEAD -- launcher/<carpeta>/`
(`09eb67f74` es el commit de fork real, confirmado contra el tag `11.0.0` de upstream — diff limpio en
archivos no tocados por branding), filtrando commits de branding/docs para aislar trabajo real de
revisión de lógica. Esa parte (revisión propia contra la propia historia) es correcta tal cual y no
cambió.

Para comparar contra lo que upstream arregló y este fork **no trajo**, la base correcta es el tag de
upstream `11.0.0` (no `09eb67f74` — ver nota de auto-corrección más abajo), contra el tag más reciente
(`11.0.3`). Ese proceso ya no se corre a mano: `tools/dev/audit_upstream.sh` lo automatiza completo
(agrega/actualiza el remote `upstream`, detecta el último tag real, filtra por palabras clave de
seguridad, y chequea con `git merge-base --is-ancestor` cuáles ya están aplicados en `main`). Correrlo
de nuevo cada vez que Prism saque un tag nuevo, o periódicamente — no solo cuando surge la duda.

## Tabla de estado por módulo (`launcher/`)

| Carpeta | Origen | Commits de revisión propia* | Último commit real | Estado |
|---|---|---|---|---|
| `net/` | Heredado | 2 | `8d6dfe6bc` (2026-07-18) | ✅ Auditado — límite de redirects, tope de memoria, crash fix |
| `meta/` | Heredado | 3 | `8d6dfe6bc` (2026-07-18) | ✅ Auditado — 2 path traversal + tope de memoria |
| `migration/` | Mixto | 3 | `0e0ca32a8` (2026-07-08) | ✅ Auditado — path traversal en importador GDLauncher |
| `updater/` | Propio | 6 | `977ebb010` (2026-07-15) | ✅ Auditado — mayormente código propio (BetelineyUpdater) |
| `minecraft/mod/` | Heredado | 5 | `290e01071` (2026-07-07) | ⚠️ Parcial — 1 use-after-free real corregido, resto sin revisar a fondo |
| `minecraft/launch/` | Mixto | 3 | `977a1f83e` (2026-07-11) | ⚠️ Parcial — trabajo fue feature propia (sandboxing), no auditoría de lo heredado |
| `ui/` | Mixto | 11 | `77a199f3a` (2026-07-18) | ⚠️ Parcial — mayoría features/temas propios, no auditoría de seguridad de lo heredado |
| `launch/` (steps) | Heredado | 1 | `0bf83eae3` (2026-06-17) | ⚠️ Parcial — solo se agregó una feature encima (CheckModConflicts), resto sin tocar |
| `crash/` | Propio | 2 | `0d3577558` (2026-06-19) | N/A — código propio (CrashReporter) |
| `logs/` | Propio | 1 | `43708b311` (2026-06-16) | N/A — código propio (LogAnalyzer) |
| `modplatform/` | Heredado (salvo `beteliney/`) | 4, 1 de fondo | `34a101ec6` (2026-07-22) | ⚠️ **Parcial, recién arrancada — 2026-07-22, CORREGIDA 2026-08-05.** `flame/FlameInstanceCreationTask.cpp` (734L) — auditoría de sesión 54 se declaró "sin hallazgos" y **estaba incompleta**: verificó `targetFolder` (correctamente descartado) pero se salteó `m_pack.overrides`, un path traversal real vía `manifest.json` sin validar, mismo patrón que `migration/GDLauncherMigrator.cpp`. Encontrado y en corrección por otra sesión (diff sin commitear al 2026-08-05, agrega `FS::safeChildPath()`) — ver `ESTADO.md` nota del 2026-08-05. Todo lo demás (`FileResolvingTask.cpp`, `FlameAPI.cpp`, `PackManifest.cpp`, y las carpetas `atlauncher/`, `ftb/`, `import_ftb/`, `legacy_ftb/`, `technic/`, `packwiz/`, `modrinth/`, `helpers/` completas) sin auditar. |
| `minecraft/auth/` | 100% heredado | 2 (`AccountList.cpp` off-by-one, JSON sin escapar en 3 `steps/`) | `74fb83c6b` (2026-07-23) | ✅ **Auditado completo — 2026-07-22.** Los 21 archivos de implementación (raíz + `steps/`) revisados línea por línea. 2 bugs reales corregidos. 1 hallazgo abierto de severidad baja no explotable (base64 sin validar en `AccountData.cpp`, decisión consciente de no tocar). Test unitario propio: 3/12 archivos (`AccountList`, `AccountData`, `MinecraftAccount`), verificado con prueba de control real. Falta `AuthFlow.cpp` + 9 de `steps/` (requiere mock de `Task`/`NetworkTask`). |
| `minecraft/skins/` | 100% heredado | 0 | nunca | ❌ Nunca auditado |
| `minecraft/update/` | 100% heredado | 0 | nunca | ❌ Nunca auditado |
| `tasks/` | 100% heredado | 0 | nunca | ❌ Nunca auditado (infraestructura transversal — riesgo medio-alto por eso mismo) |
| `java/` | 100% heredado | 0 | nunca | ❌ Nunca auditado (detección/lanzamiento de binarios Java — riesgo medio) |
| `settings/` | 100% heredado | 0 | nunca | ❌ Nunca auditado |
| `archive/` | 100% heredado | 0 | nunca | ❌ Nunca auditado |
| `console/` | 100% heredado | 0 | nunca | ❌ Nunca auditado |
| `filelink/` | 100% heredado | 0 | nunca | ❌ Nunca auditado |
| `news/` | 100% heredado | 0 | nunca | ❌ Nunca auditado |
| `tools/` | 100% heredado | 0 | nunca | ❌ Nunca auditado |
| `icons/` | 100% heredado | 0 | nunca | ❌ Nunca auditado (riesgo bajo — solo íconos) |
| `macsandbox/` | 100% heredado | 0 | nunca | ❌ Nunca auditado (no compila en Linux — riesgo bajo para el hardware objetivo) |

\* Excluye commits de branding/traducción/docs. Un número >0 no implica auditoría de seguridad completa
— ver columna Estado para el detalle real de qué se hizo.

## Hallazgo concreto — corregido: no son 3, son 53 (ver nota de auto-corrección abajo)

**Nota de auto-corrección (2026-07-19, mismo día):** la primera versión de este documento reportaba
"3 fixes reales de upstream sin aplicar", encontrados corriendo el diff a mano. Al convertir ese
proceso en un script repetible (`tools/dev/audit_upstream.sh`, ver más abajo) salieron dos bugs en el
método manual, no en los 3 hallazgos en sí (esos siguen siendo reales y siguen sin aplicar):

- El rango de comparación usaba `09eb67f74` (el primer commit de *este* repo) como base contra tags
  de *upstream* — pero ese commit no vive en la misma línea de historia que upstream, así que un rango
  `A..B` con esa base no refleja lo que se buscaba comparar. La base correcta es el tag de upstream con
  el que ese commit tiene diff limpio: `11.0.0`.
- Una vez corregida la base, el filtro de palabras clave (`git log --grep -iE ...`) tenía un typo de
  sintaxis (`-iE` no es una flag válida de git, se necesita `-i -E` por separado) que fallaba con
  `fatal: argumento no reconocido`, error que quedaba silenciado por un `2>/dev/null` — el script
  reportaba "0 encontrados" cuando en realidad estaba fallando por completo, no encontrando cero por
  auditoría limpia. Ese es exactamente el patrón de "ausencia de evidencia interpretada como evidencia
  de ausencia" contra el que existe este documento — corregido en el propio script para que un error
  de `git` aborte con mensaje explícito en vez de imprimir un resultado vacío.

Con la base y el filtro corregidos, `tools/dev/audit_upstream.sh` (rango `11.0.0..11.0.3`, mismas
palabras clave que la búsqueda manual original) encuentra **53 commits** de upstream con mensajes de
seguridad/crash/leak que tocan carpetas heredadas y **ninguno de los 53 está aplicado en `main`**
(confirmado por el script con `git merge-base --is-ancestor` contra cada hash). Los 3 originales siguen
adentro de esos 53. De los 53, el siguiente subconjunto es el de mayor severidad real —
path traversal, overflow, use-after-free, o etiquetado `security(...)` explícitamente por upstream —
priorizado sobre el resto, que son en su mayoría crashes puntuales y memory leaks reales pero de menor
severidad:

1. **`c4e99152d`** — *fix atl path traversal* (`modplatform/`). Mismo tipo de vulnerabilidad ya cerrada
   en el importador GDLauncher y en el feed de meta, pero en un rincón de `modplatform/` nunca tocado.
   **APLICADO 2026-07-20** (commit local `c4e99152d`), limpio sin conflictos. Build+ctest verificados
   2026-07-21 (31/31). Verificado internamente, sesión 49.
2. **`3cce733b1`** — *fix zip path traversal* (`minecraft/launch/`, `archive/`).
   **APLICADO 2026-07-20** (commit local `3cce733b1`), limpio sin conflictos. Build+ctest verificados
   2026-07-21 (31/31). Verificado internamente, sesión 49.
3. **`94d705d68`** — *fix heap overflow with unstable version comparation* (`modplatform/`).
   **APLICADO 2026-07-20** (commit local `94d705d68`), reescribe `Version.cpp` completo a FlexVer con
   `operator<=>`. Conflicto real en `Packwiz.cpp` (sort lexicográfico sin dedup del fork vs. sort
   semántico + dedup de upstream) resuelto a favor de upstream. Build+ctest verificados 2026-07-21
   (31/31, incluido `Packwiz` y `Version`). Verificado internamente, sesión 49, para el detalle
   completo de la resolución de conflicto y la verificación de que la API externa no se rompe.
4. ~~**`3967fde40`** — *fix heap buffer overflow*~~ — **verificado 2026-07-21, NO aplica.** Corrige un
   comparador `<=` (no estrictamente débil) en el sort de `Packwiz.cpp` — el mismo bloque que
   `94d705d68` ya reescribió con `sortMCVersions()` (`operator<=>` + desempate por string). Sin bug que
   corregir. Conflicto real al intentarlo, abortado.
5. ~~**`5f874330d`** — *security(modrinth) reorder hash algo priority*~~ — **verificado 2026-07-21, NO
   aplica.** El fix reordena una cadena de fallback sha1→sha512→sha256. Verificado con `git blame`
   hasta el commit de fork: nuestra línea nunca tuvo fallback, exige sha512 y falla si falta — más
   estricta que la versión ya corregida de upstream. Conflicto real, abortado.
6. ~~**`ac13579b9`** — *fix heap-use-after-free in modrinth creation task*~~ — **verificado 2026-07-21,
   NO aplica.** El fix agrega un guard `if (!ended_well) { liberar, retornar temprano }` tras el loop.
   Nuestro código ya tiene el equivalente (variables renombradas). Conflicto real, abortado.
7. ~~**`ded77e618`** — *Fix NetJob use-after-free*~~ — **verificado 2026-07-21, NO aplica.** El archivo
   que toca (`NetworkResourceAPI.cpp`) ya no existe, refactorizado a `ResourceAPI.cpp`. Ahí el patrón
   real usa `netJob.toWeakRef()` + `.lock()` dentro del lambda — más seguro que capturar por valor
   (lo que hace el fix de upstream). Conflicto de modificar/eliminar, abortado.
8. ~~**`9cd199a49`** — *fix use-after-free crash caused by QtConcurrent*~~ — **verificado 2026-07-21,
   NO aplica.** Único que mergeó sin conflicto — y dio diff vacío. Ya está aplicado palabra por palabra.
   `git cherry-pick --skip`, sin commit vacío.
9. ~~**`345641f7d`** — *sanitize some MSA auth logging*~~ — **verificado 2026-07-19, NO aplica: ver
   nota de re-verificación más abajo.** Su archivo base (`minecraft/auth/flows/AuthContext.cpp`,
   arquitectura vieja) ya no existe — reemplazado por la arquitectura `steps/` actual, que resuelve el
   mismo problema (tokens crudos en logs) de forma mejor: `qCDebug(authCredentials())` con
   `DEFAULT_SEVERITY Warning` (`launcher/CMakeLists.txt`), o sea que esos logs no imprimen nada salvo
   que el usuario habilite el debug de esa categoría a mano. Cherry-pick intentado y descartado — sin
   diff que aplicar.
10. ~~**`f4b22dae9`** — *fix accounts crash*~~ — **verificado 2026-07-19, NO aplica.** `git cherry-pick`
    con `--no-commit` produjo diff vacío: el campo `m_name` que causaba el crash ya no existe en
    `AccountList.cpp`/`.h` actuales — el bug ya no está presente, aparentemente resuelto por una
    refactorización posterior no relacionada a este commit puntual.
11. **`710789b70`** — *security-scoped bookmarks* (`settings/`, macOS) — prioridad baja para este
    proyecto, el hardware/SO objetivo es Linux, no macOS.

**Nota de re-verificación (2026-07-19, misma tarde, al intentar aplicar los cherry-picks):** de los 11
priorizados, se intentaron los 2 de `minecraft/auth/` primero, autorizado por el usuario. Ninguno de
los 2 aplicaba — ver el tachado en los ítems 9 y 10 arriba. Esto revela una limitación real del método
del script: `merge-base --is-ancestor` solo detecta si el commit exacto está en la historia, no si el
problema que soluciona ya fue resuelto por una vía distinta (refactor posterior, arquitectura
reemplazada). Los 9 restantes de la lista de 11 no fueron re-verificados todavía — quedan con el mismo
nivel de confianza que antes (fix real, según mensaje de commit; no garantizado que apliquen limpio
hasta intentarlo uno por uno).

**Hallazgo de `MSAStep.cpp`, investigado a fondo y cerrado como no-problema (2026-07-19, misma tarde):**
la primera lectura de este documento decía que `qWarning() << "OAuth2 request failed:" << reply->readAll();`
podía filtrar el `code=` de la redirección OAuth. Verificado contra la documentación oficial de Qt
(`QAbstractOAuthReplyHandler::networkReplyFinished`: *"After the server determines whether the request
is valid this function will be called... to get the data received from the server"*) y confirmado que
ese lectura era incorrecta: `networkReplyFinished` no maneja el callback local del navegador — es el
hook que Qt dispara cuando termina la petición saliente de **intercambio de token** contra
`login.microsoftonline.com/consumers/oauth2/v2.0/token`. `reply` es la respuesta de Microsoft a esa
petición, no la redirección del navegador. Verificado además contra `git show 11.0.3:.../MSAStep.cpp`:
el patrón es idéntico, línea por línea, a upstream actual — no es código de Beteliney. Y verificado
contra la documentación de Microsoft Entra sobre errores de OAuth2 (`reference-error-codes`): el cuerpo
de error de ese endpoint contiene únicamente `error`, `error_description`, `error_codes`, `trace_id`,
`correlation_id`, `timestamp` — nunca el `access_token`, `refresh_token`, ni el código de autorización
(por definición, un intercambio fallido no emite tokens). Conclusión real: **no es una fuga de
credenciales**, es logging de diagnóstico estándar de OAuth2 (útil para depurar "no puedo iniciar
sesión", el reporte de soporte más común de cualquier launcher), heredado sin cambios de upstream, y no
justifica un parche local unilateral contra el propio criterio del proyecto original. Cerrado — no
requiere acción.



Los 42 restantes son crashes puntuales y memory leaks reales de `modplatform/`, `minecraft/mod/`,
`net/`, `tasks/`, `java/` — no re-listados acá uno por uno para no inflar este documento; correr
`tools/dev/audit_upstream.sh` de nuevo para verlos todos con hash y carpeta.

**Pendiente de acción — actualizado 2026-07-21, sesión 50:** de los 11 priorizados originales, **3
aplicados** (`c4e99152d`, `3cce733b1`, `94d705d68`, sesión 49 cont.), **7 descartados por ya estar
resueltos de otra forma o no aplicar** (`345641f7d`, `f4b22dae9`, `3967fde40`, `5f874330d`,
`ac13579b9`, `ded77e618`, `9cd199a49` — ver arriba cada uno), y **1 sin intentar por baja prioridad de
plataforma** (`710789b70`, macOS, hardware/SO objetivo del proyecto es Linux). No queda ningún
cherry-pick accionable en Linux de esta lista de 11. Siguiente paso real: evaluar los 42 fixes de
severidad menor restantes (no en bloque — cherry-pick masivo sin revisar cada diff individual sería
repetir el mismo error de fondo que generó este documento, solo que más rápido).

## Hallazgo documentado, sin acción tomada — almacenamiento de tokens en `minecraft/auth/`

`AccountData.cpp`/`.h` serializa `accessToken`/`refresh_token` (Microsoft/Xbox/Minecraft) como JSON
plano en disco (`accounts.json`), sin cifrar. No es un bug introducido por este proyecto — es el diseño
estándar heredado de PrismLauncher/MultiMC, y el mismo patrón que usan prácticamente todos los
launchers de este tipo (protección delegada al permiso de archivo del sistema operativo, sin capa
propia). Se documenta acá como **pregunta abierta, no como hallazgo cerrado**: nunca se evaluó
explícitamente si vale la pena agregar cifrado a nivel SO (`libsecret`/keyring en Linux, DPAPI en
Windows) dado el modelo de amenaza real de este proyecto. Sin decisión tomada.

## Hallazgo documentado, sin acción tomada — validación de base64 en `AccountData.cpp`

`profileFromJSONV3` (`AccountData.cpp`) decodifica `skin.data`/`cape.data` con
`QByteArray::fromBase64()` sin `AbortOnBase64DecodingErrors` — a diferencia de `Parsers.cpp`, que sí
usa esa opción para el mismo tipo de dato. Dos `// TODO: validate base64` explícitos en el código lo
marcan como sabido. **Verificado 2026-07-22, severidad baja, no explotable:** el dato viene de
`accounts.json` local (no de red directa — si ese archivo ya está comprometido hay problemas peores), y
el consumidor (`MinecraftAccount::getFace()`) chequea el `bool` de retorno de `QPixmap::loadFromData()`
— un base64 corrupto produce como mucho una cara sin cargar, no un crash. Queda como TODO legítimo de
completitud, no como vulnerabilidad activa. Sin decisión tomada sobre si vale la pena cerrarlo.

## Hallazgo — JSON armado por interpolación de string en 3 pasos de `minecraft/auth/steps/` — **resuelto 2026-07-22**

`LauncherLoginStep.cpp`, `XboxAuthorizationStep.cpp` y `XboxUserStep.cpp` armaban el body JSON de sus
requests con `QString::arg()` sobre un template crudo, interpolando valores de tokens (`uhs`, `xToken`,
`userToken.token`, `msaToken.token`) sin escapar comillas/backslashes. Confirmado heredado del fork vía
`git blame` contra `09eb67f74`. Explotabilidad real siempre fue bajísima (valores vienen de Microsoft/
Xbox Live sobre TLS). **Corregido:** los 3 reemplazados por `QJsonObject`/`QJsonDocument`, mismo
esquema y campos, escapado automático. Compilado + `ctest` 31/31, commit `fdd4cee33`. No se hizo login
real de punta a punta con cuenta de Microsoft para confirmar en producción — recomendado antes de la
próxima release.

## Próximos pasos recomendados, en orden de prioridad real (actualizado 2026-07-23)

0. **[Nuevo — cobertura de test real, sesiones 53 cont./54, 2026-07-23]** La auditoría de `minecraft/auth/` (ítem 3 de abajo) fue de lectura de código — no había ni un test unitario propio sobre esa carpeta hasta ahora. Se agregaron 3: `AccountList_test.cpp` (verifica la regresión del off-by-one de `8f9b179a4`), `AccountData_test.cpp` (round-trip de serialización, migración de token legacy, fallbacks) y `MinecraftAccount_test.cpp` (18 tests: `createOffline`/`createBlankMSA`, `uuidFromUsername`, `ownsMinecraft()`, los 5 casos de `shouldRefresh()`, `fillSession()`). Los tres verificados con prueba de control real (romper el bug/umbral a propósito, confirmar que el test lo detecta, revertir). Cobertura de `minecraft/auth/`: **3 de 12 archivos con test propio.** Pendiente real: `AuthFlow.cpp` y los 9 archivos de `steps/` — el flujo que sí habla con la red vía OAuth/XSTS, requiere mockear `Task`/`NetworkTask` para poder testearlo sin depender de un servidor real ni de credenciales de Microsoft. No es trivial, pero es el hueco de cobertura más grande que queda en esta carpeta.

1. **[Cerrado — 2026-07-21, sesión 50]** Los 9 cherry-picks restantes ya se intentaron uno por uno:
   8 no aplican (ya resueltos por otra vía), 1 (`710789b70`, macOS) queda en backlog de baja prioridad
   sin intentar. Ver tabla de cherry-picks arriba.
2. **[Parcial — 2026-07-22, sesión 52]** De los 42 fixes de severidad menor originales, `tools/dev/audit_upstream.sh` corrido en vivo hoy encontró en realidad **53** (el número había quedado
   desactualizado). 6 standalone intentados de verdad: 1 aplicado real (`a753988e0`, commit `a753988e0`,
   compilado+testeado), 5 ya resueltos de otra forma. **Los ~46 restantes no son candidatos sueltos —
   forman un cluster ligado al refactor de ownership de recursos de upstream** (`FolderModel` de
   punteros crudos a modelo con `shared_ptr`, serie larga de commits relacionados en
   `minecraft/mod/` y `modplatform/`). Adoptarlo es una decisión de arquitectura del dueño del proyecto,
   no un TODO técnico — no intentar cherry-pickear archivo por archivo sin decidir eso primero.
3. **[Cerrado — 2026-07-22, sesión 51]** Auditoría línea por línea de `minecraft/auth/`: completa,
   los 21 archivos de implementación revisados. 1 bug corregido (`AccountList.cpp`), 1 hallazgo de
   robustez sin tocar (ver arriba, JSON sin escapar en 3 `steps/`). Ver tabla arriba.
4. Primera pasada sobre `modplatform/` — sigue siendo la carpeta con más superficie de código externo
   nunca mirado (Modrinth, CurseForge, FTB, Technic, Packwiz, ATLauncher).
5. **[Hecho — 2026-07-19]** Repetir el diff contra `upstream` cada vez que Prism saque un tag nuevo:
   convertido en `tools/dev/audit_upstream.sh`, ya no depende de acordarse ni de repetir comandos a
   mano.
6. **[Nuevo — sesión 54 cont., 2026-07-23, hallazgo real, no solo re-lectura]** El dueño del proyecto
   decidió adoptar el refactor de ownership del ítem 2 (respuesta explícita: "adoptarlo completo").
   Antes de tocar código se intentó `git cherry-pick --no-commit` real sobre los 35 candidatos que
   quedaban del ítem 2 (no solo lectura) para separar qué es genuinamente parte del refactor de qué
   no. Resultado, verificado uno por uno:
   - **3 más resultan diff vacío** (`fe5aee261`, `2981d9109`, `21de7a2d9`) — ya resueltos de otra forma
     en este fork, sin relación con el refactor. Sube el total de "ya resuelto de otra forma, detectado
     tarde" a **10 de 53** (7 del ítem 2 original + estos 3). Ninguno se commiteó (diff vacío, nada que
     commitear).
   - **`17ea51ce2`** (leak de Windows en `JavaUtils.cpp`, no tenía nada que ver con el refactor por
     archivo) sí dio conflicto real, pero al revisar el código actual **el leak ya no existe** —
     nuestro `FindJavaFromRegistryKey` fue reescrito distinto (una sola asignación, sin duplicar) en
     algún momento sin quedar documentado como tal. Se suma a la lista de "ya resuelto de otra forma":
     **11 de 53.**
   - **Hallazgo de fondo, el importante:** el cluster de "~46" del ítem 2 **no es un solo refactor,
     son al menos DOS features completos de upstream nunca adoptados**, y `tools/dev/audit_upstream.sh`
     no puede verlos porque solo busca palabras clave de seguridad/crash en el *mensaje* del commit —
     no detecta una dependencia de API. Se confirmó intentando `1b3ff96ff` (leak de `NetJob`, parecía
     independiente por archivo — `ConcurrentTask.cpp`, `FileResolvingTask.cpp`): el conflicto real
     mostró que ese commit llama a `updateStepProgress(progress, Operation::REMOVED/CHANGED)`, un
     método y un enum que **no existen en absoluto** en este fork. Rastreando con
     `git log -S "updateStepProgress"` aparece el origen real: `f997529cd`/`fae250d50`,
     **"feat: better task tracking"** (upstream, 2023-03-30) — una feature de UI completa y separada
     (barra de progreso por sub-tarea en vivo: `SubTaskProgressBar.cpp/.h/.ui` nuevos, cambios en
     `Task.h/.cpp`, `ConcurrentTask.h/.cpp`, `Download.cpp/.h`, `NetAction.h`, `ProgressDialog.*`) que
     nunca se adoptó en este fork y que ningún documento había registrado como pendiente hasta ahora.
     **Es una decisión de adopción de feature aparte, no parte de la decisión ya tomada sobre el
     refactor de ownership de recursos** — el dueño del proyecto todavía no se expidió sobre esta.
     Cherry-pick de `1b3ff96ff` abortado limpio, árbol restaurado a `abcaddea9` sin nada a medio
     resolver. El resto de los ~30 candidatos sin intentar individualmente todavía puede repartirse
     entre estos dos features (ownership de recursos / task tracking) o ser genuinamente independiente
     — no asumido, pendiente de repetir este mismo método (`cherry-pick --no-commit` real, uno por uno)
     antes de escribir una sola línea de más.
7. **[Nuevo — sesión 54 cont., 2026-07-23 — cambio de método, hallazgo mayor]** Investigando `1b3ff96ff`
   más a fondo (¿de dónde sale `updateStepProgress`/`Operation` si no existen en ningún lado?) se
   encontró que **ese mecanismo tampoco existe en la punta actual de upstream (`11.0.3`)** — lo
   introdujo `1b3ff96ff` en 2023 y el propio upstream lo sacó después (`6b918daf7`/`c5592a446`,
   sep-2024, *"Remove all the slowdown code from ConcurrentTask"*, por ser lento). Comparando
   `launcher/tasks/ConcurrentTask.cpp` actual contra `11.0.3` directamente (no contra el commit
   histórico): **son idénticos** — no hay ninguna brecha ahí. Lo mismo con
   `launcher/modplatform/flame/FileResolvingTask.cpp`: el "leak" que `1b3ff96ff` decía arreglar ya no
   está presente en ninguna de las dos puntas; la única diferencia real contra upstream es un feature
   chico y no relacionado (`FallbackMRBlockedMods`, un toggle de settings para reintentar descargas de
   mods bloqueados en Modrinth, que este fork no tiene). **Conclusión metodológica, la importante:**
   comparar contra commits históricos individuales (`merge-base --is-ancestor`, cherry-pick por hash)
   da falsos positivos sistemáticos en archivos que cambiaron mucho — un commit puede quedar "sin
   aplicar" simplemente porque upstream lo revirtió o reescribió después, no porque el problema siga
   presente. **El método correcto es diff directo del archivo actual contra la punta actual de
   upstream (`11.0.3`), no contra commits sueltos.** Corrida esa comparación sobre las 3 carpetas
   completas (`minecraft/mod/`, `modplatform/`, `tasks/`, filtrando branding/copyright/whitespace):
   **45 archivos con diferencia real**, de 3 a 316 líneas de diff cada uno — lista completa por tamaño
   en el registro interno de sesiones (sesión 54 cont.). Esto reemplaza al "cluster de ~46 commits" como
   la unidad de trabajo real: no todo ese diff es un bug sin aplicar — una parte es divergencia propia
   intencional de Beteliney (features/branding propios) y otra parte real puede ser mejoras o fixes de
   upstream sin traer. Separar una cosa de la otra, archivo por archivo, empezando por los más chicos,
   es el trabajo real pendiente — no cherry-pickear hashes sueltos.


## Nota de recuperación — 2026-08-06, este documento fue restaurado tras pérdida de cambios locales

Entre la sesión que generó las secciones de abajo (evaluación por diff de archivo, tandas 1-4) y esta
nota, otra sesión activa en este mismo repo hizo `reset: moving to origin/main` (ver `git reflog`,
`HEAD@{8}`) seguido de varios commits reales propios (purga de historial con `filter-repo`, corrección
de hashes en este documento, `CHANGELOG.md`, cambios de código). Ese reset descartó todo lo que estaba
sin commitear en ese momento — y **toda la auditoría por diff de archivo de abajo nunca había sido
commiteada por nadie, en ninguna sesión anterior** — vivía solo como cambio local acumulado.

Restaurado acá desde el contexto de la conversación que lo escribió originalmente, con una distinción
honesta:
- **Primera tanda** (25 de 45 archivos): texto verbatim, se había leído completo del archivo antes de
  que se perdiera.
- **Segunda tanda** (`ATLPackInstallTask.cpp`) y **Nota de proceso** (activación de herramientas):
  **reconstruidas a partir del resumen conversacional de esa sesión, no del texto original exacto**
  — el archivo se perdió antes de leerlas verbatim en esta conversación. Fieles en contenido y
  conclusión, pero la redacción no es necesariamente palabra por palabra la original.
- Todo lo que sigue después de la nota de proceso (`ModrinthInstanceCreationTask.cpp` en adelante,
  incluida la reconciliación de 217 archivos y `audit_file_divergence.sh`): texto verbatim, escrito en
  esta misma conversación.

**Recomendación aplicada esta vez:** este documento se commitea localmente (sin push) apenas termina de
restaurarse, para que un futuro `reset`/checkout de otra sesión no vuelva a llevárselo. No es una
garantía absoluta (un `reset --hard` a un commit anterior a este también lo tumbaría), pero es la
mitigación real disponible sin coordinar activamente con quien sea que corra la próxima sesión.


## Evaluación real, primera tanda (25 de 45 archivos) — 2026-07-28

*(Sección reconstruida desde lo procesado en esta conversación al leer el archivo antes de que se
perdiera — fiel en contenido y conclusiones, no garantizado palabra por palabra igual al original.)*

Cubre archivos con diff de 3 a 19 líneas totales contra `11.0.3`, filtrando branding a mano. De 25
evaluados, 14 tuvieron algo puntual para anotar:

- `modplatform/technic/TechnicPackProcessor.cpp` — cambio menor de manejo de errores, sin impacto de
  comportamiento real.
- `modplatform/helpers/OverrideUtils.cpp` — ajuste de estilo, sin cambio funcional.
- `modplatform/modrinth/ModrinthPackExportTask.cpp` y `.h` — variación menor en el export de packs,
  no se identificó riesgo.
- `minecraft/mod/tasks/LocalModParseTask.cpp` — cambio acotado en el parseo local de mods, sin
  regresión detectada.
- `modplatform/ftb/FTBPackInstallTask.cpp` — diferencia menor de estilo/flujo, sin efecto de
  comportamiento confirmado.
- `tasks/Task.h` y `tasks/Task.cpp` — ajustes pequeños en la clase base de tareas, revisados sin
  hallazgo de riesgo.
- `modplatform/ResourceAPI.h` y `.cpp` — cambios acotados en la capa base de API de recursos, sin
  regresión identificada en esta pasada (nota: esta es la misma capa donde después, en tandas
  posteriores, se encontraron remociones de metadata de tracking en `ModrinthAPI.h` — vale revisar
  este archivo de nuevo a la luz de eso).
- `minecraft/mod/ResourceFolderModel.cpp`, `minecraft/mod/ShaderPackFolderModel.cpp`,
  `minecraft/mod/DataPack.h`, `minecraft/mod/tasks/DataPackFolderModel.cpp`,
  `minecraft/mod/tasks/ResourcePackFolderModel.cpp` — marcados como "mejoras genuinas": cambios que
  parecían mejoras deliberadas del fork sobre el comportamiento de upstream, no drift accidental.

Los otros 11 de los 25 se evaluaron y no dieron nada para reportar — **no quedó registrado en su momento
cuáles son exactamente**, limitación real de esa sesión. Quedan 20 archivos de ese rango de tamaño (3-19
líneas) sin evaluar de la lista original de 45.


## Segunda tanda — 2026-07-28 cont.

*(Reconstruida desde el resumen conversacional de la sesión que la generó, no desde el texto exacto del
archivo perdido.)*

Recontó los "45 archivos" con `git diff --stat` y `--numstat` (bug encontrado: `--stat` trunca rutas
largas con `...`, rompiendo el diff contra rutas inválidas sin avisar — corregido usando `--numstat`, que
no trunca). También corrigió que `beteliney/*` y `MalwareScanner.*` no son divergencia de heredado, son
código propio nuevo que ni existe en `11.0.3` — filtrarlos dio 46 archivos reales, coherente con el "45"
ya documentado.

### `ATLPackInstallTask.cpp` (296 líneas de diff, el más grande de los 46 en ese momento)

Mayormente desmodernización de estilo (se sacó `std::move`, `std::ranges::any_of`/`find_if`, se pasó de
`const&` a por-valor en varios parámetros) — sin cambio de comportamiento en esa parte.

**Falso positivo descartado explícitamente:** `bool resetDirectory;` sin inicializar parecía riesgoso
mirando solo el diff parcial, pero leyendo la función completa, los dos únicos casos del `switch` la
asignan y el `default:` corta con `return` antes de usarla — no hay camino de ejecución que la lea sin
inicializar. No es bug.

**Hallazgo real, severidad BAJA:** el scope del lock de `SettingsObject::Lock` en `install()` cambió —
perdió sus llaves explícitas, así que ahora se libera después de `emitSucceeded()` en vez de justo antes.
Se investigó qué es `SettingsObject::Lock` antes de calificarlo: no es un mutex de threading, es RAII de
batching de escritura a disco — no hay riesgo de deadlock, solo una ventana despreciable de
desincronización disco-memoria. (Este mismo patrón de "se sacaron las llaves de un bloque con `Lock`" se
volvió a encontrar, ya en esta conversación, en `import_ftb/PackInstallTask.cpp` y
`legacy_ftb/PackInstallTask.cpp` — ver más abajo, tres apariciones en total.)


## Nota de proceso — activación de herramientas al empezar sesión nueva

*(Reconstruida desde el intercambio de esta misma conversación sobre el mismo tema, no desde el texto
exacto del archivo perdido — el contenido es equivalente.)*

Las herramientas (Desktop Commander, filesystem, pibe-mcp) son las mismas en todas las sesiones — no hay
"otro Claude" con acceso distinto. Cada conversación nueva arranca sin nada cargado del set de
herramientas diferidas, tenga o no memoria activada — es un mecanismo técnico de carga por sesión, no
depende de que el usuario active memoria en Settings ni de que la IA "se acuerde" de charlas pasadas.

Lo que sí es un fallo de la IA y no del sistema: no debería esperar a que el usuario diga "activalas". Si
el pedido ya trae la ruta o el contexto necesario, eso debería disparar la búsqueda/carga de herramientas
sola, sin que haga falta pedirlo explícitamente — sea el tema Beteliney o cualquier otro completamente
distinto. La ruta del archivo sí hay que seguir dándola siempre (no hay forma barata de inferirla sin que
se diga, ni con memoria activada), pero "activar herramientas" no debería requerir pedirlo.


## `ModrinthInstanceCreationTask.cpp` (264 líneas de diff, segundo más grande de los 46 originales)

Diff completo contra `11.0.3` leído en una sola pasada (525 líneas de diff unificado, sin cortes).
Confirmación cruzada contra: `InstanceCreationTask.h`/`.cpp` (clase base), `FlameInstanceCreationTask.cpp`
(mismo patrón replicado ahí), `net/ApiDownload.h` (firma real de `makeFile`), y `git show 11.0.3:...`
para comparar contra el comportamiento original línea por línea, no solo el diff.

### Hallazgo real, severidad ALTA — se perdió la confirmación antes de borrar `saves/`

En `11.0.3`, `InstanceCreationTask::scheduleToDelete()` tenía esta protección explícita antes de agregar
cualquier archivo a la lista de borrado:

```cpp
if (path.startsWith("saves/")) {
    if (m_shouldDeleteSaves == ShouldDeleteSaves::NotAsked)
        m_shouldDeleteSaves = askIfShouldDeleteSaves(parent);
    if (m_shouldDeleteSaves == ShouldDeleteSaves::No)
        return;  // no se agrega a la lista, no se borra
}
```

Esa función completa (`scheduleToDelete`), el enum `ShouldDeleteSaves`, el miembro `m_shouldDeleteSaves`
y el diálogo `askIfShouldDeleteSaves` **ya no existen en ningún archivo del repo actual** (verificado con
`grep -rn` sobre todo `launcher/`, cero resultados para los cuatro símbolos). El fork inlineó
directamente `m_files_to_remove.append(...)` en `ModrinthInstanceCreationTask::updateInstance()` — y de
forma idéntica en `FlameInstanceCreationTask.cpp`, mismo patrón, mismas líneas de manejo de `.disabled` —
sin la verificación de prefijo `saves/` en ningún lado (confirmado con `grep -rn "saves/" launcher/
modplatform/ launcher/InstanceCreationTask.cpp`, cero resultados).

`m_files_to_remove` sí se consume de verdad (no es dead code): `InstanceCreationTask.cpp`, dentro del
bloque `if (shouldOverride())`, hace `QFile::remove(path)` sobre cada entrada, sin ningún filtro
adicional.

**Efecto real:** si el `modrinth.index.json` (o el manifest de CurseForge — mismo problema en Flame) de
una versión vieja de un modpack lista algún archivo bajo `saves/` como removido/reemplazado en la
versión nueva, el launcher ahora lo borra en silencio al actualizar el modpack, sin preguntar nada —
mientras que `11.0.3` mostraba un diálogo de confirmación y respetaba un "No". Es baja probabilidad de
que un modpack legítimo liste archivos de `saves/` en su manifest de borrado, pero si pasa, es pérdida
de datos del mundo del usuario sin aviso. No es exclusivo de Modrinth — el mismo patrón se replicó en
`FlameInstanceCreationTask.cpp`, así que el riesgo es sistémico a los dos flujos de actualización de
modpack, no un caso aislado.

**Recomendación:** restaurar la verificación de `saves/` + confirmación del usuario, ya sea reviviendo
`scheduleToDelete()` en la clase base o agregando el chequeo directamente donde se hace
`m_files_to_remove.append()` en ambos archivos (Modrinth y Flame). Es un cambio chico y acotado — el
riesgo es real pero la probabilidad de disparo es baja, así que no es bloqueante para seguir usando el
launcher mientras se agenda, pero sí debería entrar antes del próximo release que toque flujos de
actualización de modpacks. **Este es el hallazgo más serio de toda la auditoría hasta ahora, sin
resolver en código todavía.**

### Hallazgo de diseño, a confirmar intención — se eliminó el metadata de tracking en las descargas

`Net::ApiDownload::makeFile()` (`launcher/net/ApiDownload.h`) ya no acepta el cuarto parámetro `meta`
que existía en `11.0.3` (`Net::ModrinthDownloadMeta{ reason, gameVersion, loader }`, enviado como
headers HTTP en cada descarga de mod hacia la API de Modrinth). La firma actual es solo
`makeFile(QUrl url, QString path, Download::Options options = NoOptions)` — no es que el caller dejó de
pasar un argumento opcional, la funcionalidad fue removida de la API entera (también se sacaron los
includes `net/ApiHeaderProxy.h` y `modplatform/ModIndex.h`, y el cálculo muerto de la variable `loader`
que ya no se usa en ningún lado). Tiene toda la pinta de ser un cambio de diseño deliberado (menos
telemetría/identificación enviada a la API de Modrinth por cada descarga), no un drift accidental — está
limpio en toda la superficie afectada, sin cabos sueltos. **No lo marco como bug**, pero como es una
remoción de funcionalidad real (no solo estilo), vale confirmar que fue intencional y no efecto
secundario de otro refactor.

### Ruido de estilo, sin efecto de comportamiento
Renombrado sistemático de variables locales a `snake_case` (`indexPath`→`index_path`, etc.), remoción de
llaves en `if` de una sola línea, `const auto&`→`auto const&`, `auto*`→`auto` en varios punteros locales.
Ningún cambio de flujo de control real en esta categoría — verificado línea por línea contra el diff
completo.

### Detalle menor — permisos de archivo
El diff marca `old mode 100644 / new mode 100755` — el archivo `.cpp` quedó con bit de ejecución puesto.
No tiene efecto en la compilación, pero es higiene de repo rota (probablemente un editor o `chmod`
accidental). Barato de arreglar: `chmod 644`.


## `DataPack.cpp` (226 líneas de diff, tercero más grande de los 46 originales)

### Hallazgo real, severidad MEDIA — el archivo está desactualizado respecto a `11.0.3`, no "refactorizado"

A diferencia de todos los archivos evaluados hasta ese punto, acá el diff no es un refactor de estilo del
fork sobre una base más simple — es al revés: `11.0.3` tiene una tabla `s_pack_format_versions` mucho
más extensa (formatos de datapack hasta el 100, cubriendo snapshots hasta "26.1-snap11") con soporte de
**rangos** vía `std::pair<int,int>` (mayor, menor) y campos `m_min_format`/`m_max_format`, más los
métodos `packFormatStr()` (string legible con el rango de versiones de Minecraft compatibles) y
`mappings()`. El fork tiene una versión bastante más vieja y corta de esta misma tabla, con clave `int`
simple (sin rango), tabla cortada en el formato 48 (Minecraft 1.21), y sin ninguno de esos métodos:
`setPackFormat()` ahora solo recibe `new_format_id`, `valid()` quedó reducido a `m_pack_format != 0`.

Confirmé que no es una ruptura a medias: `grep -rn "packFormatStr\|mappings()\|m_min_format\|m_max_format"`
sobre todo `launcher/` (cpp/h/ui) da cero resultados — nada llama a lo que se sacó, así que compila
limpio, coherente con lo que dice `ESTADO.md` sobre el estado del build. Pero funcionalmente el fork
perdió: reconocer datapacks con formato 49+ (cualquier data pack para Minecraft más nuevo que 1.21 ahora
tira "Pack format 'N' is not a recognized data pack id!" en vez de mostrar el rango de versiones
compatible), y perdió el soporte de rango completo (un datapack que declara `min_format`/`max_format` en
vez de un formato único ya no se valida contra eso).

**Interpretación:** esto no tiene la firma de un cambio de diseño deliberado (no hay ningún gancho a
`beteliney/`, ninguna lógica propia agregada a cambio de lo que se sacó, y el patrón no calza con el
resto de los refactors sistemáticos vistos en otros archivos). Tiene más pinta de que este archivo
específico quedó desincronizado — forkeado desde un punto de upstream anterior a que se agregara el
soporte de rango y se extendiera la tabla, y nunca se volvió a mergear. **Recomendación:** antes de
decidir cualquier cosa, confirmar con el dueño si esto fue intencional; si no, es candidato directo a
resolverse tomando la versión de `11.0.3` de este archivo casi entera (a diferencia de los otros
archivos evaluados, acá no hay lógica propia del fork que preservar en `setPackFormat`/`valid`/
`compatibleVersions`).

### Resto del diff
Cabecera de copyright (`Prism Launcher` → `Beteliney Launcher` + línea de copyright propia) y el mismo
detalle de permisos `100644`→`100755` que en los archivos anteriores — ambos esperables y sin
implicancia de comportamiento.

## `ModrinthAPI.h` (115 líneas de diff)

Sobre todo ruido de estilo (`const auto&`→`auto`/`const`, `static`→`inline` en varios métodos, formato
de indentación), sin cambios de comportamiento en la mayoría. Dos cosas reales para anotar:

**Se sacó `include_changelog` de las URLs de `getVersionsURL()` y `getDependencyURL()`** — igual que el
caso de `ModrinthDownloadMeta` en `ModrinthInstanceCreationTask.cpp`, es una remoción limpia y completa:
`grep -rn "includeChangelog" launcher/` da cero resultados, el campo ya no existe ni en el struct de
args. No es un bug a medias, es consistente en todos lados. Efecto: las requests a la API de Modrinth
para listar versiones ya no piden el changelog. No lo marco como bug, pero vale confirmar intención.

**`getAuthorURL` pasó de `static` a método de instancia.** Como el proyecto compila limpio, ningún call
site debe estarlo llamando como `ModrinthAPI::getAuthorURL(...)` ya — no hay riesgo real.

## `ResourcePack.cpp` (111 líneas de diff)

Mismo patrón exacto que `DataPack.cpp`: tabla `s_pack_format_versions` de 11.0.3 llega hasta formato 83
("26.1-snap11"), con soporte de rango completo; la del fork corta en formato 34 (Minecraft 1.21) y sin
rango, mismo cambio `mappings()` → `compatibleVersions()`. Confirma que no es un accidente aislado de
`DataPack.cpp` — es el mismo archivo "gemelo" (resource packs en vez de data packs) con la misma
desincronización respecto a `11.0.3`. Refuerza la recomendación anterior: esto pinta a que ambos archivos
se forkearon desde un punto de upstream anterior al agregado de rangos + extensión de tabla, y no se
volvieron a sincronizar — vale resolver los dos juntos, no por separado.

## `import_ftb/PackInstallTask.cpp` (131 líneas) y `legacy_ftb/PackInstallTask.cpp` (168 líneas)

Evaluados juntos, mismo patrón y mismo veredicto en los dos.

**Mismo hallazgo de scope de lock que en `ATLPackInstallTask.cpp` (severidad BAJA), repetido dos veces
más.** En ambos, el bloque `{ SettingsObject::Lock lock(instance.settings()); ... }` perdió sus llaves —
`lock` ahora vive hasta el final de la función en vez de liberarse justo antes de
`emitSucceeded()`/del `return` de fallback. Mismo análisis que en `ATLPackInstallTask.cpp` aplica sin
cambios, no hay ruta de fallo nueva. Van tres archivos con este mismo patrón — tiene pinta de un pase de
reformateo/linter que corrió sobre todo `modplatform/`, no tres decisiones independientes.

**En `legacy_ftb/PackInstallTask.cpp`:** `#include "MMCZip.h"` → `#include "BetelineyZip.h"`, con
`MMCZip::extractDir` → `BetelineyZip::extractDir`. Confirmé que `launcher/BetelineyZip.h` existe — es el
rebranding propio del fork de la utilidad de zip, no una regresión. Sin más cambios reales en ninguno de
los dos archivos: el resto es copyright header y el mismo `100644`→`100755` de siempre.

## `FlameCheckUpdate.cpp` (196 líneas) y `ModrinthCheckUpdate.cpp` (225 líneas)

Evaluados juntos, mismo patrón en los dos: la clase de API (`FlameAPI`/`ModrinthAPI`) pasó de construirse
al vuelo en cada llamada a una instancia `static` a nivel de archivo. Confirmé que ninguna de las dos
clases tiene variables miembro (`FlameAPI.h` es puro métodos `static`/`inline`, sin estado) — así que
compartir una instancia estática entre llamadas no introduce ningún riesgo de concurrencia. Es
optimización pura, sin efecto de comportamiento.

**Confirma el patrón sistémico de remoción del parámetro "reason" en `ResourceDownloadTask`.** Ambos
archivos llaman `makeShared<ResourceDownloadTask>(pack, version, m_resourceModel)` (3 args) donde
`11.0.3` pasaba `(pack, version, m_resourceModel, true, "update")` (5 args). Revisé
`ResourceDownloadTask.h` del fork: el constructor actual es
`ResourceDownloadTask(pack, version, packs, bool is_indexed = true)` — solo 4 parámetros, sin ningún
string de "reason"/motivo. No es un caller desactualizado, es la firma real del header, coherente en los
dos call sites. Con esto van tres apariciones del mismo patrón (`ModrinthDownloadMeta.reason`,
`include_changelog`, y este) — ya no lo trato como hallazgo aislado por archivo: es una remoción de
funcionalidad deliberada y consistente en toda la capa de descarga/actualización de mods. No es un bug,
pero como es un patrón repetido, vale confirmarlo una sola vez con el dueño para los tres juntos.


## Estado consolidado y definitivo — reconciliación de los tres conteos (45 → 106 → 1320 → 217)

**Por qué había tres números distintos:** cada tanda usó un criterio de filtrado distinto para lo mismo
("¿este archivo tiene divergencia real, o es solo branding?"). Tanda 1 filtró branding leyendo cada diff
a mano. Una corrección posterior usó un piso arbitrario de 20 líneas totales de `--numstat` — eso dejó
afuera archivos grandes fuera de `modplatform/` (`PrismExternalUpdater.cpp`, etc.) y adentro archivos
chicos que eran puro cambio de copyright, dando 106. Un recuento sin piso dio 1320, dominado casi
enteramente por el cambio de copyright "Prism Launcher"→"Beteliney Launcher" que toca casi todo el árbol
con 2-4 líneas cada uno.

**Herramienta que resuelve esto de raíz:** `tools/dev/audit_file_divergence.sh [tag] [min_real_lines]`.
Para cada archivo, toma el diff (`-U0`), descarta las líneas que matchean un regex de
branding/copyright/mode-only conocido, y cuenta cuántas líneas de cambio real sobreviven — un archivo
entra a la lista solo si sobrevive contenido real, no por superar un piso de líneas totales elegido a
ojo. Verificado contra los 23 archivos ya nombrados individualmente en este documento hasta ese momento:
los 23 aparecen en la salida del script, con conteos coherentes. Corrida con `tag=11.0.3,
min_real_lines=3` (mismo piso que usó tanda 1 originalmente): **217 archivos con divergencia real**, ese
es el número definitivo.

**Cuántos quedan pendientes, con la honestidad de un hueco real:** de los 217, hay 24 confirmados como
evaluados por nombre exacto: los 9 archivos grandes de tandas 2-4 (`ATLPackInstallTask.cpp`,
`ModrinthInstanceCreationTask.cpp`, `DataPack.cpp`, `ModrinthAPI.h`, `ResourcePack.cpp`,
`import_ftb/PackInstallTask.cpp`, `legacy_ftb/PackInstallTask.cpp`, `FlameCheckUpdate.cpp`,
`ModrinthCheckUpdate.cpp`), los 14 nombrados de tanda 1 (`TechnicPackProcessor.cpp`,
`OverrideUtils.cpp`, `ModrinthPackExportTask.cpp`/`.h`, `LocalModParseTask.cpp`,
`FTBPackInstallTask.cpp`, `tasks/Task.h`/`.cpp`, `ResourceAPI.h`/`.cpp`, `ResourceFolderModel.cpp`,
`ShaderPackFolderModel.cpp`, `DataPack.h`, `DataPackFolderModel.cpp`, `ResourcePackFolderModel.cpp`), y
`PrismExternalUpdater.cpp` (ver abajo, resultó falso positivo de metodología). Tanda 1 dice haber
evaluado 25 en total pero solo nombró 14 hallazgos — los otros ~11 se revisaron y no dieron nada para
reportar, pero no quedó registrado cuáles son. Así que el pendiente real está entre **182 y 193**, no un
número exacto.

## `PrismExternalUpdater.cpp` (367 líneas reales según el script) — falso positivo, no archivo pendiente

El diff contra `11.0.3` lo muestra como `deleted file mode` — el archivo entero fue borrado, no
modificado. No es una regresión: fue renombrado/reemplazado por `launcher/updater/BetelineyExternalUpdater.cpp`
(+ `.h`), confirmado que existe. **Caveat real para `audit_file_divergence.sh`:** un archivo que aparece
como `deleted file mode` puro no es automáticamente "código heredado perdido sin reemplazo" — puede ser
un rename. Antes de auditar cualquier entrada grande de la lista de 217, confirmar primero si es
`deleted file mode` puro y si hay un archivo con nombre nuevo que cumple el mismo rol, antes de invertir
tiempo de más.
