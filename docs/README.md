# Índice de documentación — BetelineyLauncher

Mapa de todos los documentos del proyecto: qué es cada uno, para qué sirve, y en qué orden conviene leerlos según lo que necesites. Objetivo: que nadie tenga que adivinar dónde está algo.

## Si querés entender cómo está armado el launcher

**[`ARQUITECTURA.md`](ARQUITECTURA.md)** — documento técnico de módulos: qué hace cada pieza Beteliney-específica y cómo se engancha en el código heredado de Prism Launcher.

**[`AUDITORIA_MODULOS.md`](AUDITORIA_MODULOS.md)** — tabla persistente de qué carpetas de `launcher/` heredadas de Prism ya fueron auditadas línea por línea (seguridad, tests) y cuáles siguen sin revisión. Regla explícita: toda carpeta sin evidencia documentada de revisión se trata como NO REVISADA por defecto.

**[`PERFILES_JVM.md`](PERFILES_JVM.md)** — los 7 perfiles JVM del launcher explicados: qué flags G1GC/ZGC usa cada uno y por qué.

**[`CHANGELOG.md`](CHANGELOG.md)** — historial de cambios técnico completo, commit por commit. Es la fuente de verdad de las versiones publicadas.

**[`TELEMETRIA.md`](TELEMETRIA.md)** — qué datos de telemetría manda el launcher, qué no, y por qué (privacidad primero).

**[`INVESTIGACION_FORKS_COMPETENCIA.md`](INVESTIGACION_FORKS_COMPETENCIA.md)** — investigación de código real (no marketing) sobre FjordLauncher y ShatteredPrism: qué tienen, cuánto costaría portar su alt-auth, y por qué el "DRM" que ambos tocan no le aplica a Beteliney (ya tiene su propio Client ID de Microsoft). Insumo de roadmap, sin código implementado.

## Si querés compilar, empaquetar o resolver un problema

**[`COMPILAR_WINDOWS.md`](COMPILAR_WINDOWS.md)** — guía detallada de compilación en Windows (requisitos, pasos, errores comunes de ese entorno).

**[`TROUBLESHOOTING.md`](TROUBLESHOOTING.md)** — problemas frecuentes (StackOverflow con mods, lag, crashes en Vega 10, CMake sin encontrar Qt6) y su solución.

Para Linux, la chuleta rápida es [`LEEME.txt`](../LEEME.txt) (raíz) — comandos listos para copiar/pegar, no una guía narrada.

## Si querés contribuir código

**[`CONTRIBUTING.md`](CONTRIBUTING.md)** — cómo proponer cambios al proyecto.

## Licencia

**[`COPYING.md`](COPYING.md)** — texto completo de la licencia GPL-3.0. Esta copia (`docs/COPYING.md`, no la de la raíz) es la que se embebe como recurso Qt en el binario y se muestra en el diálogo "Acerca de" del launcher — no la muevas ni la borres pensando que es un duplicado del `COPYING.md` de la raíz (ese es el que GitHub detecta como licencia del repo; cumplen roles distintos, ambos reales).
