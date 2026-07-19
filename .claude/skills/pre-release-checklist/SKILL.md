---
name: pre-release-checklist
description: Usar cuando el usuario pida preparar, verificar, o cortar una release nueva de BetelineyLauncher (bump de versión, build de verificación pre-release, tag, GitHub Release). No usar para builds de desarrollo normales — eso es build-dev/ y no necesita este checklist.
---

Este skill es el procedimiento de pre-release, no una automatización — cada paso requiere juicio (sobre todo el de compilar con `build/`, que ya causó un cuelgue de 9 horas una vez por mal uso). Verificá cada punto contra el estado real del repo, no asumas que el paso anterior salió bien.

## Antes de tocar nada

1. Confirmá con el usuario que efectivamente quiere cortar una release ahora — este checklist termina en un tag público y binarios firmados, no es para "probar cómo queda".
2. `git log --oneline` y `docs/CHANGELOG.md` — confirmá que todo lo que va a entrar en esta release ya está commiteado (local está bien, no hace falta estar pusheado todavía) y documentado en el changelog. Si hay commits sin categorizar en el changelog, es acá donde se completan, no después.

## Versionado

3. `CMakeLists.txt` (líneas ~179-181 al momento de escribir esto, verificá el número real) — el versionado real del proyecto es estrictamente secuencial con patch siempre en 0 (`7.0.0→7.9.0`, `8.0.0→8.4.0`...). No inventes un patch nonzero — eso ya pasó una vez en una reescritura de CHANGELOG y tuvo que corregirse a mano.
4. Bump del número de versión en `CMakeLists.txt` acorde a lo anterior.

## Build de verificación (acá está el riesgo real)

5. Usá `build/` (Release+LTO real) **solo ahora**, nunca para iterar. Si vas a compilar desde cero, avisá antes de arrancar — un build completo con LTO en este hardware puede tardar horas si algo de la configuración de job pools se tocó sin querer (ver la causa raíz documentada en `performance-optimizer.md` y `ESTADO.md` sesión 46: `JOB_POOLS lto_link_pool=2` es la configuración que evita la sobre-suscripción 2x).
6. Build limpio, sin errores ni warnings nuevos.
7. `ctest` completo — todo en verde salvo skips ya conocidos y documentados (ej. `sys`). Si aparece un fallo nuevo, esto no sigue hasta resolverlo — nunca se firma ni se tagea con tests rotos.

## Firma

8. Verificá que la clave privada Ed25519 esté disponible como GitHub Actions secret (no local, no en texto plano en ningún archivo del repo) antes de confiar en que el workflow de CI la va a usar.
9. Si el binario se firma localmente en vez de vía CI, confirmá con `libsodium`/el mecanismo ya usado por `BetelineyUpdater` que la verificación fail-closed sigue intacta (si falta la firma o no valida, el updater borra el archivo y aborta — no debería existir un "instalar de todos modos" en ningún punto del flujo).

## Tag y release

10. `docs/CHANGELOG.md` con la entrada final de esta versión, categorizada (Seguridad / Confiabilidad / Correcciones / Nuevas funciones).
11. Tag `vX.Y.Z` — solo después de que 5-9 estén confirmados, no antes.
12. `git push` del tag y del commit de versión — esto pasa por el hook de confirmación de `git push` igual que cualquier otro push; no lo evites ni lo automatices por fuera de ese mecanismo.
13. GitHub Release con el binario firmado adjunto y el changelog de esa versión como descripción.

## Después

14. Usá el skill `actualizar-estado-md` para documentar la sesión que cerró esta release, citando el tag y el link real a GitHub Releases (mismo formato que se usó para v8.4.0 en `## IDENTIDAD DEL PROYECTO`).
