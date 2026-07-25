# Índice de documentación — BetelineyLauncher

Mapa de todos los documentos del proyecto: qué es cada uno, para qué sirve, y en qué orden conviene leerlos según lo que necesites. Objetivo: que nadie (ni una sesión nueva de Claude, ni un colaborador humano) tenga que adivinar dónde está algo.

## Si querés retomar el trabajo del proyecto (empezar acá)

**[`ESTADO.md`](../ESTADO.md)** (raíz) — fuente de verdad viva del proyecto. Lee SOLO la sección `## ESTADO ACTUAL` al principio: resume qué está hecho, qué falta, y qué decisiones esperan al dueño del proyecto. El resto del archivo (`## HISTORIAL DE SESIONES`) es el detalle sesión por sesión, útil para auditar una decisión pasada, no necesario para arrancar. Es **append-only** por convención propia: nunca se borra ni reescribe contenido de una sesión ya cerrada, solo se puede reorganizar/consolidar el resumen de arriba (ya se hizo en sesiones 33 y 54).

**[`ESTADO_HISTORICO.md`](../ESTADO_HISTORICO.md)** (raíz) — sesiones 1-29 (arco fundacional: fork inicial, estabilización de CI, meta server), movidas acá en sesión de mantenimiento para no dejar `ESTADO.md` sin poda. **Solo consulta, no se edita.**

## Si querés entender cómo está armado el launcher

**[`ARQUITECTURA.md`](ARQUITECTURA.md)** — documento técnico de módulos: qué hace cada pieza Beteliney-específica y cómo se engancha en el código heredado de Prism Launcher.

**[`AUDITORIA_MODULOS.md`](AUDITORIA_MODULOS.md)** — tabla persistente de qué carpetas de `launcher/` heredadas de Prism ya fueron auditadas línea por línea (seguridad, tests) y cuáles siguen sin revisión. Regla explícita: toda carpeta sin evidencia documentada de revisión se trata como NO REVISADA por defecto.

**[`PERFILES_JVM.md`](PERFILES_JVM.md)** — los 7 perfiles JVM del launcher explicados: qué flags G1GC/ZGC usa cada uno y por qué.

**[`CHANGELOG.md`](CHANGELOG.md)** — historial de cambios técnico completo, commit por commit. Es la fuente de verdad de versiones publicadas (`ESTADO.md` remite acá para no duplicar).

## Si querés compilar, empaquetar o resolver un problema

**[`COMPILAR_WINDOWS.md`](COMPILAR_WINDOWS.md)** — guía detallada de compilación en Windows (requisitos, pasos, errores comunes de ese entorno).

**[`TROUBLESHOOTING.md`](TROUBLESHOOTING.md)** — problemas frecuentes (StackOverflow con mods, lag, crashes en Vega 10, CMake sin encontrar Qt6) y su solución.

Para Linux, la chuleta rápida es [`LEEME.txt`](../LEEME.txt) (raíz) — comandos listos para copiar/pegar, no una guía narrada.

## Si querés contribuir código

**[`CONTRIBUTING.md`](CONTRIBUTING.md)** — cómo proponer cambios al proyecto.

## Sobre el uso de IA en este proyecto

**[`FILOSOFIA_Y_METODOLOGIA.md`](FILOSOFIA_Y_METODOLOGIA.md)** — el *por qué* detrás de cómo se construye Beteliney: metodología de trabajo con IA, filosofía de documentación, visión de largo plazo, origen del nombre. No es fuente de estado (para eso, `ESTADO.md`).

**[`GESTORES_Y_SKILLS.md`](GESTORES_Y_SKILLS.md)** — investigación y diseño de los subagentes, skills y hooks de Claude Code configurados en `.claude/` (qué es cada capa, por qué se decidió así, qué se descartó y por qué). Leer antes de tocar `.claude/agents/`, `.claude/skills/` o `.claude/settings.json`.

**[`ESTRATEGIA_IA.md`](../ESTRATEGIA_IA.md)** (raíz) — comparación de modelos de IA (costos, capacidades) evaluados para el flujo de desarrollo del proyecto.

**[`CLAUDE.md`](../CLAUDE.md)** (raíz) — instrucciones raíz que Claude Code carga automáticamente en cada sesión sobre este repo.

## Licencia

**[`COPYING.md`](COPYING.md)** — texto completo de la licencia GPL-3.0. Esta copia (`docs/COPYING.md`, no la de la raíz) es la que se embebe como recurso Qt en el binario y se muestra en el diálogo "Acerca de" del launcher — no la muevas ni la borres pensando que es un duplicado del `COPYING.md` de la raíz (ese es el que GitHub detecta como licencia del repo; cumplen roles distintos, ambos reales).
