---
name: actualizar-estado-md
description: Usar cuando toque documentar el trabajo de una sesión en ESTADO.md — al cerrar una sesión de trabajo, cuando el usuario pida "documentá esto", o cuando cualquier gestor (feature-builder, security-auditor, performance-optimizer, ui-ux-reviewer) termine algo que amerite quedar registrado. Da el formato exacto para no romper la estructura que ya tiene el documento tras 47+ sesiones.
---

Este skill existe porque el formato de `ESTADO.md` no está escrito en ningún lado de forma explícita — se infiere leyendo sesiones anteriores, lo que lleva a inconsistencias (ya pasó dos veces: huecos de sincronización en sesiones 38 y 45, donde el bloque de arriba no reflejaba lo que sí estaba commiteado abajo). Este skill es la referencia explícita para que cualquier gestor o la sesión principal lo hagan igual siempre.

## Estructura del documento (no la reordenes)

1. `## IDENTIDAD DEL PROYECTO` — tabla estática, casi nunca cambia.
2. `## ESTADO ACTUAL — LEER ESTO PRIMERO` — la única sección que un chat nuevo necesita leer. Contiene, de arriba hacia abajo: la fecha/sesión del encabezado, un párrafo en negrita por cada sesión reciente (más nueva arriba), la lista de "Pendientes reales restantes", y cualquier bloque de decisiones abiertas.
3. `## STACK TÉCNICO` — tabla estática.
4. `## HISTORIAL DE COMMITS` / `## HISTORIAL DE SESIONES` — el detalle completo por sesión, con encabezado `### Sesión N`.

## Al cerrar una sesión

1. **Actualizá la fecha y número de sesión** en el encabezado de `## ESTADO ACTUAL` (`actualizado YYYY-MM-DD, sesión N cerrada`).
2. **Agregá un párrafo nuevo en negrita** arriba de los párrafos de sesiones anteriores (orden: más nueva primero), formato `**Sesión N (cerrada):** ...` — denso, en prosa, sin bullets. Incluí: qué se pidió, qué se encontró/hizo, con qué evidencia se verificó (build, ctest, número de tests, hash de commit), y qué quedó pendiente si algo. Citá archivo:línea o commit cuando aplique — nunca "parece que anda" o "debería estar bien".
3. **Si la sesión cierra o modifica un pendiente de la lista "Pendientes reales restantes"**, editá esa entrada específica con `[Cerrado en sesión N]` o `[Parcialmente resuelto en sesión N]` al inicio, sin borrar el texto que explica el pendiente original — el historial de por qué existía importa.
4. **Agregá la entrada espejo en el historial detallado** (`### Sesión N`, más abajo en el documento) con el mismo contenido pero expandido: comandos corridos, código relevante, decisiones descartadas y por qué.
5. **Si hay commits nuevos**, agregá la entrada correspondiente en `docs/CHANGELOG.md` bajo la categoría que corresponda (Seguridad / Confiabilidad / Correcciones / Nuevas funciones) — `ESTADO.md` y `CHANGELOG.md` se actualizan juntos, nunca uno sin el otro cuando hay código nuevo de por medio.
6. **No fabriques números.** Si no corriste `ctest`, no escribas "31/31" — escribí lo que corriste de verdad o decí explícitamente que no se verificó.
7. **No reescribas historial de sesiones anteriores** salvo para corregir un error factual concreto (como la corrección de sincronización de sesión 45) — y si lo hacés, dejá una nota explícita de qué se corrigió y por qué, no lo edites en silencio.

## Antes de terminar

Corré `tools/dev/estado_actual.sh` para confirmar que la sección `## ESTADO ACTUAL` sigue siendo extraíble limpiamente (el script asume que esa sección tiene límites claros) y que lo que escribiste es lo que un chat nuevo va a leer primero.
