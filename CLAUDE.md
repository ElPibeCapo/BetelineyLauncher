# Instrucciones de proyecto — BetelineyLauncher

`ESTADO.md` es la fuente de verdad del estado del proyecto — leerlo (sección `## ESTADO ACTUAL`) antes de arrancar cualquier trabajo nuevo.

## Gestores especializados (subagentes)

Este proyecto tiene 4 subagentes en `.claude/agents/`, cada uno con contexto propio del historial y los patrones ya establecidos del código:

- **security-auditor** — seguridad: red, parsing de datos externos, updater/firma, sandboxing.
- **performance-optimizer** — rendimiento: build/compilación/linking, profiling de runtime, LTO/flags.
- **feature-builder** — implementar funciones nuevas de punta a punta.
- **ui-ux-reviewer** — interfaz, temas, accesibilidad (100% QWidgets, sin Qt Quick/QML).

El auto-routing por descripción no siempre delega solo — si el pedido calza claramente con uno de estos dominios, invocalo explícitamente en vez de manejarlo directo en la sesión principal, salvo que sea un ajuste trivial de una línea.

## Reglas de ejecución (aplican a cualquier subagente y a la sesión principal)

- Compilar (`build-dev/` para iterar, `build/` solo para verificación pre-release) y correr `ctest` antes de dar cualquier fix o feature por terminado — nunca "debería andar".
- `git push` y cualquier acción sobre `origin` (de este repo o del repo `meta`) requieren autorización explícita del usuario en esa misma conversación. Commitear local sí es autónomo.
- No fabricar cifras de rendimiento, seguridad, o compatibilidad sin fuente verificable — el proyecto ya tuvo que corregir al menos una cifra de marketing sin respaldo que se había colado en la UI real.
