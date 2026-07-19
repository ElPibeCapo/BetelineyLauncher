# Instrucciones de proyecto — BetelineyLauncher

`ESTADO.md` es la fuente de verdad del estado del proyecto — leerlo (sección `## ESTADO ACTUAL`) antes de arrancar cualquier trabajo nuevo.

## Gestores especializados (subagentes)

Este proyecto tiene 6 subagentes en `.claude/agents/`, cada uno con contexto propio del historial y los patrones ya establecidos del código — pensados como departamentos de una empresa: cada uno 100% enfocado en su dominio, sin meterse en el trabajo de otro.

- **security-auditor** — seguridad: red, parsing de datos externos, updater/firma, sandboxing. Arregla lo que encuentra.
- **performance-optimizer** — rendimiento: build/compilación/linking, profiling de runtime, LTO/flags. Arregla lo que encuentra.
- **feature-builder** — implementar funciones nuevas de punta a punta. Construye.
- **ui-ux-reviewer** — interfaz, temas, accesibilidad (100% QWidgets, sin Qt Quick/QML). Arregla lo que encuentra.
- **qa-tester** — testea todo (suite automatizada, flujos manuales, regresiones). NO arregla nada — reporta la falla y dice explícitamente a cuál de los otros gestores le corresponde resolverla.
- **maintenance-checker** — chequeo amplio y rápido de salud general (git, build, tests, docs), no un deep-dive. Si encuentra algo serio, deriva al gestor específico en vez de investigarlo a fondo.

**Patrón de handoff:** `qa-tester` y `maintenance-checker` encuentran y derivan, nunca arreglan a fondo. Los otros cuatro arreglan dentro de su dominio. El handoff puede pasar en la misma sesión (invocar al siguiente gestor a continuación) o en una sesión/chat distinto — el usuario decide cuándo retomarlo.

El auto-routing por descripción no siempre delega solo — si el pedido calza claramente con uno de estos dominios, invocalo explícitamente por nombre en vez de manejarlo directo en la sesión principal, salvo que sea un ajuste trivial de una línea.

## Reglas de ejecución (aplican a cualquier subagente y a la sesión principal)

- Compilar (`build-dev/` para iterar, `build/` solo para verificación pre-release) y correr `ctest` antes de dar cualquier fix o feature por terminado — nunca "debería andar".
- `git push` y cualquier acción sobre `origin` (de este repo o del repo `meta`) requieren autorización explícita del usuario en esa misma conversación. Commitear local sí es autónomo.
- No fabricar cifras de rendimiento, seguridad, o compatibilidad sin fuente verificable — el proyecto ya tuvo que corregir al menos una cifra de marketing sin respaldo que se había colado en la UI real.
