# Auditoría de Módulos — Registro de Estado de Revisión

> Documento vivo. Se crea el **2026-07-19** (chat separado, fuera de la numeración de sesiones de
> Claude Code que usa `ESTADO.md` — ver nota en ese documento) a partir de un fallo real: durante 48
> sesiones,
> ninguna documentación distinguía "módulo verificado" de "módulo heredado de Prism y nunca mirado" —
> lo que llevó a asumir por defecto que todo estaba en orden simplemente porque nadie había reportado
> un problema. `minecraft/auth/` (login Microsoft, manejo de tokens de sesión) llevaba **cero commits
> de revisión desde el fork**, y nadie lo había documentado como hueco hasta que se preguntó explícitamente.
> Este documento existe para que esa ambigüedad no vuelva a pasar desapercibida.

## Regla de mantenimiento — leer antes de tocar cualquier carpeta de `launcher/`

Si una carpeta no aparece acá con evidencia de revisión (commit real + fecha + qué se encontró), se
trata como **NO REVISADA**, nunca como "probablemente bien". Cualquier sesión que audite, toque o
descarte una carpeta debe actualizar su fila. No se permite marcar "revisado" sin un commit real o una
verificación reproducible (comando corrido, resultado observado) que lo respalde — ninguna entrada de
esta tabla se basa en memoria ni en suposición. Mismo estándar que ya rige el resto de `ESTADO.md`.

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
| `net/` | Heredado | 2 | `f9acc39a4` (2026-07-18) | ✅ Auditado — límite de redirects, tope de memoria, crash fix |
| `meta/` | Heredado | 3 | `f9acc39a4` (2026-07-18) | ✅ Auditado — 2 path traversal + tope de memoria |
| `migration/` | Mixto | 3 | `c149bb9a0` (2026-07-08) | ✅ Auditado — path traversal en importador GDLauncher |
| `updater/` | Propio | 6 | `067aaa01b` (2026-07-15) | ✅ Auditado — mayormente código propio (BetelineyUpdater) |
| `minecraft/mod/` | Heredado | 5 | `efe33a69e` (2026-07-07) | ⚠️ Parcial — 1 use-after-free real corregido, resto sin revisar a fondo |
| `minecraft/launch/` | Mixto | 3 | `da70d0e6b` (2026-07-11) | ⚠️ Parcial — trabajo fue feature propia (sandboxing), no auditoría de lo heredado |
| `ui/` | Mixto | 11 | `b5dfbd239` (2026-07-18) | ⚠️ Parcial — mayoría features/temas propios, no auditoría de seguridad de lo heredado |
| `launch/` (steps) | Heredado | 1 | `f33bf6191` (2026-06-17) | ⚠️ Parcial — solo se agregó una feature encima (CheckModConflicts), resto sin tocar |
| `crash/` | Propio | 2 | `4c6596960` (2026-06-19) | N/A — código propio (CrashReporter) |
| `logs/` | Propio | 1 | `43708b311` (2026-06-16) | N/A — código propio (LogAnalyzer) |
| `modplatform/` | Heredado (salvo `beteliney/`) | 3, ninguno de fondo | `a3eb3e767` (2026-07-06) | ❌ **Nunca auditado** — ver hallazgo abajo |
| `minecraft/auth/` | 100% heredado | 2 (`AccountList.cpp` off-by-one, JSON sin escapar en 3 `steps/`) | `34a101ec6` (2026-07-22) | ✅ **Auditado completo — 2026-07-22.** Los 21 archivos de implementación (raíz + `steps/`) revisados línea por línea. 2 bugs reales corregidos. 1 hallazgo abierto de severidad baja no explotable (base64 sin validar en `AccountData.cpp`, decisión consciente de no tocar). Resto limpio. |
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

1. **`0c2b3b384`** — *fix atl path traversal* (`modplatform/`). Mismo tipo de vulnerabilidad ya cerrada
   en el importador GDLauncher y en el feed de meta, pero en un rincón de `modplatform/` nunca tocado.
   **APLICADO 2026-07-20** (commit local `73e640b1c`), limpio sin conflictos. Build+ctest verificados
   2026-07-21 (31/31). Ver `ESTADO.md` → `### Sesión 49 cont.`.
2. **`56936cf48`** — *fix zip path traversal* (`minecraft/launch/`, `archive/`).
   **APLICADO 2026-07-20** (commit local `f31924b6c`), limpio sin conflictos. Build+ctest verificados
   2026-07-21 (31/31). Ver `ESTADO.md` → `### Sesión 49 cont.`.
3. **`5a0931d3c`** — *fix heap overflow with unstable version comparation* (`modplatform/`).
   **APLICADO 2026-07-20** (commit local `71e275b9e`), reescribe `Version.cpp` completo a FlexVer con
   `operator<=>`. Conflicto real en `Packwiz.cpp` (sort lexicográfico sin dedup del fork vs. sort
   semántico + dedup de upstream) resuelto a favor de upstream. Build+ctest verificados 2026-07-21
   (31/31, incluido `Packwiz` y `Version`). Ver `ESTADO.md` → `### Sesión 49 cont.` para el detalle
   completo de la resolución de conflicto y la verificación de que la API externa no se rompe.
4. ~~**`3967fde40`** — *fix heap buffer overflow*~~ — **verificado 2026-07-21, NO aplica.** Corrige un
   comparador `<=` (no estrictamente débil) en el sort de `Packwiz.cpp` — el mismo bloque que
   `5a0931d3c` ya reescribió con `sortMCVersions()` (`operator<=>` + desempate por string). Sin bug que
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
aplicados** (`0c2b3b384`, `56936cf48`, `5a0931d3c`, sesión 49 cont.), **7 descartados por ya estar
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
esquema y campos, escapado automático. Compilado + `ctest` 31/31, commit `34a101ec6`. No se hizo login
real de punta a punta con cuenta de Microsoft para confirmar en producción — recomendado antes de la
próxima release.

## Próximos pasos recomendados, en orden de prioridad real (actualizado 2026-07-22)

1. **[Cerrado — 2026-07-21, sesión 50]** Los 9 cherry-picks restantes ya se intentaron uno por uno:
   8 no aplican (ya resueltos por otra vía), 1 (`710789b70`, macOS) queda en backlog de baja prioridad
   sin intentar. Ver tabla de cherry-picks arriba.
2. **[Parcial — 2026-07-22, sesión 52]** De los 42 fixes de severidad menor originales, `tools/dev/audit_upstream.sh` corrido en vivo hoy encontró en realidad **53** (el número había quedado
   desactualizado). 6 standalone intentados de verdad: 1 aplicado real (`d958a91ce`, commit `e031c5df7`,
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
