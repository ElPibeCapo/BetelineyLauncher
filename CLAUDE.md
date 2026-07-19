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

**Memoria persistente:** `security-auditor` y `qa-tester` tienen `memory: project` habilitado (`.claude/agent-memory/<nombre>/`, versionado en git) — compounding de hallazgos entre sesiones sin tener que reescribir el `.md` del agente cada vez. Los otros 4 no la tienen todavía por decisión explícita, no por olvido: su contexto de dominio es más estable (patrones de código ya cerrados) que el de auditoría/testing, que sí varía sesión a sesión.

## Skills (`.claude/skills/`)

A diferencia de los subagentes (contexto aislado, se invocan para delegar una tarea completa), los skills son procedimientos reusables que cualquier gestor o la sesión principal puede cargar sin cambiar de contexto — para pasos que se repiten igual sin importar quién los ejecuta.

- **actualizar-estado-md** — formato exacto para documentar el cierre de una sesión en `ESTADO.md`. Reemplaza la inferencia por lectura de sesiones anteriores (causa real de los huecos de sincronización de sesiones 38 y 45).
- **pre-release-checklist** — procedimiento completo de release (versionado, build `build/` con LTO, firma, tag, GitHub Release). Solo para cortar una release real, no para builds de desarrollo.

Antes de crear un skill nuevo, evaluá si el conocimiento es realmente reusable entre distintos gestores/contextos (skill) o si es específico de un solo dominio (mejor como parte del `.md` del agente correspondiente) — duplicar la misma prosa en varios agentes en vez de centralizarla en un skill es exactamente el desperdicio que este layer existe para evitar.

## Hooks (`.claude/settings.json`)

- **PreToolUse sobre `Bash`** (`tools/dev/claude_guard_git_push.sh`): intercepta cualquier comando que contenga `git push` (en este repo o en cualquier otro alcanzable desde el comando, ej. `meta`) y fuerza el prompt de confirmación de Claude Code, sin depender de que el modelo recuerde la regla de abajo. Es la versión determinística de la regla "`git push` requiere autorización explícita" — no la reemplaza, la respalda.

No confundir con los git hooks de `tools/dev/install_hooks.sh` (`secret_scan.sh` como `pre-commit`) — son mecanismos distintos: esos corren en `git commit` a nivel del propio git: estos corren en Claude Code, antes de que la herramienta `Bash` se ejecute.

## Reglas de ejecución (aplican a cualquier subagente y a la sesión principal)

- Compilar (`build-dev/` para iterar, `build/` solo para verificación pre-release) y correr `ctest` antes de dar cualquier fix o feature por terminado — nunca "debería andar".
- `git push` y cualquier acción sobre `origin` (de este repo o del repo `meta`) requieren autorización explícita del usuario en esa misma conversación. Commitear local sí es autónomo. Reforzado por el hook de arriba.
- No fabricar cifras de rendimiento, seguridad, o compatibilidad sin fuente verificable — el proyecto ya tuvo que corregir al menos una cifra de marketing sin respaldo que se había colado en la UI real.
- Los 6 agentes de `.claude/agents/` heredan automáticamente este `CLAUDE.md` completo al ser invocados (confirmado contra la documentación oficial de subagentes) — no hace falta, y por lo tanto no corresponde, repetir estas reglas globales dentro de cada `.md` de agente. Cada agente solo debería llevar lo que es específico de su dominio.
