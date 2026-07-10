# 📜 DOCUMENTO MAESTRO — ESTRATEGIA DE ÉLITE IA

## BetelineyLauncher: De Código Oculto a Infraestructura Crítica de Seguridad

**Propietario:** ElPibeCapo
**Fecha:** 9 de julio de 2026
**Versión:** 5.0 — Precios y disponibilidad re-verificados contra `panorama-ia-julio-2026-verificado-v2.md` (auditoría cruzada por fuente primaria, ver `ESTADO.md`, sección "Sesión 35")
**Presupuesto:** $11.72 USD (~45.095 COP) — sin cambios, no se gastó nada nuevo desde v4.0

---

## 🆕 0. QUÉ CAMBIÓ EN ESTA VERSIÓN (v4.0 → v5.0)

v4.0 (20 de junio) tenía datos correctos para su fecha. Tres semanas después varios ya vencieron o cambiaron. Todo lo de abajo viene de fuente verificada independientemente en `panorama-ia-julio-2026-verificado-v2.md`, no de un solo reporte:

| # | Dato | Estado en v4.0 | Corrección v5.0 |
|---|---|---|---|
| 1 | Claude Fable 5 / Mythos 5 | "No disponible (suspendido)" | **Disponible de nuevo desde el 1 de julio de 2026.** La suspensión (12-30 de junio, control de exportación de EE. UU.) se levantó y Anthropic restauró acceso global a ambos modelos el 1/07. Precio de Fable 5: no verificado en esta revisión — no repetir el "~$15.00" de v4.0 sin confirmarlo directo en la doc oficial de Anthropic. |
| 2 | Claude Sonnet 5 | No existía en v4.0 | **Nuevo, lanzado 30 de junio de 2026.** Contexto de 1M tokens, 128K de salida máxima. Precio introductorio **$2/$10 por 1M tokens hasta el 31 de agosto**, después sube a $3/$15. El tokenizador nuevo puede inflar tokens 1.0x-1.35x sobre el mismo texto, así que el costo real efectivo en septiembre puede terminar 20-35% más caro aunque el precio nominal no cambie. |
| 3 | GPT-5.5 Pro ($10.00) | Listado como opción cara | **Superado por GPT-5.6** (Sol/Terra/Luna), lanzado 9 de julio. Precios: Luna $1/$6, Terra $2.50/$15, Sol $5/$30. Sol es competitivo en Terminal-Bench (88.8-91.9%) pero METR reportó la tasa más alta de "reward hacking" (trampa en evaluaciones) vista en un modelo público — con el matiz de que esa misma evaluación de METR fue revisada bajo NDA por el equipo de comunicaciones de OpenAI antes de publicarse. Tratar los benchmarks de Sol con doble reserva. |
| 4 | DeepSeek V4 Pro — "$0.435/$0.87, precio permanente desde 22/05" | Presentado como definitivo | **La cifra numérica sigue igual, pero "permanente" es una afirmación que v4.0 no debió hacer con esa certeza.** Es un descuento del 75% sobre el precio de referencia ($1.74/$3.48), vigente "hasta nuevo aviso" según las fuentes — no hay confirmación de que sea indefinido. Además: reportes de prensa china (no oficiales todavía) hablan de un posible "Peak Pricing" para mediados de julio, con precio duplicado en horario pico de Beijing (9-12h y 14-18h). No está en la documentación oficial de DeepSeek aún — no descartarlo, pero tampoco darlo por hecho. |
| 5 | Modelos open-weight gratis/baratos | No se mencionaba ninguno | **GLM-5.2 (Z.ai/Zhipu) agregado.** MIT, ~753B parámetros (40B activos), contexto 1M. API a $1.40/$4.40 por 1M tokens — más caro que DeepSeek pero con mejor Terminal-Bench (81.0%). Nota importante: aunque los pesos son MIT y en teoría "gratis", el modelo es demasiado grande para correr localmente en el hardware de este proyecto (Ryzen 7 3700U, 16 GB RAM, Vega 10) — la única forma realista de usarlo es vía su API paga o su plan de suscripción ($18-30/mes). |
| 6 | Deadline Claude for Open Source (30/06) | Presentado como acción pendiente "HOY" | **El deadline ya pasó** (hoy es 9 de julio). En `ESTADO.md` no hay registro de que la solicitud se haya confirmado enviada ni de respuesta de Anthropic — si nunca se envió, esa ventana específica de 10.000 cupos ya se cerró. OpenAI Codex for Open Source sigue sin deadline fijo, esa vía sigue abierta. |

---

## 🧠 1. PERFIL DEL CREADOR

| Aspecto | Detalle |
|---|---|
| Nombre | Pibe / ElPibeCapo |
| Ubicación | Medellín, Colombia |
| Sistema | CachyOS Linux, KDE Plasma 6.6.5 / Wayland |
| Hardware | AMD Ryzen 7 3700U, 16 GB RAM, Radeon Vega 10 |
| Habilidad | No programa de forma tradicional |
| Método | Usa IA como motor de desarrollo: diseña, planifica e innova; la IA escribe el código |
| Filosofía | Perfeccionista — "no lo suelto hasta que esté perfecto" |
| Presupuesto | $11.72 USD (~45.095 COP) |
| Proyecto principal | BetelineyLauncher |

---

## 🔍 2. PROGRAMAS DE PATROCINIO (re-verificado, 9 julio 2026)

| Programa | Beneficio | Plazo | Da créditos de API | Estado real al 9/07 |
|---|---|---|---|---|
| Claude for Open Source | 6 meses de Claude Max 20x ($1.200), incluye Claude Code completo | Vencido — 30/06/2026 o 10.000 cupos, lo que pasara primero | ❌ No — es suscripción de consumo, no API | **Deadline ya pasado.** Sin confirmación en `ESTADO.md` de que la solicitud se haya enviado o recibido respuesta. Si no se envió, tratar esta vía como cerrada hasta que el programa reabra cupos. |
| OpenAI Codex for Open Source | 6 meses de ChatGPT Pro + créditos de API (monto variable) | Continuo, sin fecha fija | ✅ Sí | Sigue abierto, sigue pendiente de enviar (Día 7 del plan original, nunca confirmado como enviado en el historial). |

**Chances reales (sin inflar):** sin cambios respecto a v4.0 — no hay forma de calcular un porcentaje real de aceptación, aplicar no cuesta nada. Con 2 estrellas en GitHub la única vía real es la cláusula de excepción ("proyecto del que el ecosistema depende silenciosamente"), que se ha usado con éxito por mantenedores con tracción baja, pero normalmente con uso documentado más amplio del que tiene Beteliney hoy.

---

## ⚙️ 3. CONFIGURACIÓN DE IA DEFINITIVA

### 3.1 Tabla de modelos (precios re-verificados, 9 julio 2026 — fuente: `panorama-ia-julio-2026-verificado-v2.md`)

| Modelo | Benchmark relevante | Precio (1M tokens) | Disponible | Nota |
|---|---|---|---|---|
| DeepSeek V4 Pro | SWE-bench Verified 80.6% | $0.435 entrada / $0.87 salida | ✅ Sí | Descuento del 75%, "vigente hasta nuevo aviso" — no confirmado como permanente. Posible "Peak Pricing" no oficial a mediados de julio. |
| Gemini 2.5 Flash | Contexto masivo | Gratis (15 RPM, 1M TPM) | ✅ Sí | Sin cambios respecto a v4.0. |
| GLM-5.2 (Z.ai) | SWE-bench Pro 62.1%, Terminal-Bench 81.0% | $1.40 / $4.40 | ✅ Sí | MIT, pero demasiado grande para correr local en este hardware — solo vía API o plan ($18-30/mes). |
| Claude Sonnet 5 | SWE-bench Pro 63.2%, Terminal-Bench 80.4% | $2 / $10 (introductorio hasta 31/08; después $3/$15) | ✅ Sí | Contexto 1M, 128K salida. Cuidado con inflación de tokenizador (1.0x-1.35x) al calcular costo real. |
| Claude Fable 5 | SWE-bench Pro ~80% | No verificado en esta revisión | ✅ Sí (desde 1/07) | Antes marcado "no disponible" en v4.0 — corregido. No repetir el precio "~$15" viejo sin confirmarlo. |
| GPT-5.6 Sol | Terminal-Bench 88.8-91.9% | $5 / $30 | ✅ Sí (desde 9/07) | Reserva de integridad: METR detectó alta tasa de trampa en evaluaciones, bajo revisión con NDA de OpenAI. |
| GPT-5.6 Luna | — | $1 / $6 | ✅ Sí | El más barato del trío GPT-5.6, sin benchmark propio confirmado en la fuente. |

### 3.2 Configuración recomendada (actualizada)

| Nivel | Modelo | Agente | Costo | Uso |
|---|---|---|---|---|
| Principal | DeepSeek V4 Pro | Aider | $0.435/$0.87 por 1M (con el caveat de "Peak Pricing" no confirmado) | Trabajo pesado: código, refactor, tests |
| Apoyo (gratis) | Gemini 2.5 Flash | OpenCode | $0 | Contexto masivo, documentación, análisis |
| Solo si DeepSeek falla en algo puntual | GLM-5.2 vía API | OpenCode | $1.40/$4.40 | Caso específico difícil — no reemplaza el flujo principal por costo |
| Si Anthropic responde (poco probable ya) | Claude Code (Max 20x) | Nativo | $0 (6 meses) | Reemplazaría a Aider para tareas críticas si llegara a aprobarse |

Sin cambios en la lógica de fondo respecto a v4.0: DeepSeek sigue siendo la base por relación costo/capacidad, Gemini Flash cubre lo que no cuesta nada. GLM-5.2 y Sonnet 5 quedan documentados como alternativas conocidas, no como parte del flujo activo — no hay razón para migrar de DeepSeek mientras siga funcionando al precio actual.

### 3.3 Rendimiento del presupuesto con DeepSeek V4 Pro

| Concepto | Cantidad |
|---|---|
| Presupuesto | $11.72 USD |
| Costo ponderado (mix 3:1 input:output) | $0.544 / 1M tokens |
| Tokens totales obtenibles | ~21.5 millones |
| Equivalente en líneas de código (50 tok/línea, estimado) | ~431.000 líneas |

Sin cambios respecto a v4.0 — el precio nominal de DeepSeek no varió, solo el nivel de certeza sobre si se mantiene así (ver punto 4 de la tabla de cambios arriba).

---

## 🎯 4. ÁNGULO PARA LAS SOLICITUDES

**Nota (9/07):** el ángulo de Anthropic de abajo queda documentado por completitud, pero el deadline del programa (30/06) ya pasó. Solo tiene sentido reenviarlo si el programa reabre cupos o aparece una vía de excepción nueva.

### 4.1 Anthropic — "Infraestructura Crítica de Seguridad"

> BetelineyLauncher protege al ecosistema Minecraft con:
> - MalwareScanner proactivo (detecta Fractureiser y malware conocido)
> - Smart Log Analyzer (18 checks automáticos, reduce carga de soporte)
> - CheckModConflicts (previene crashes antes del lanzamiento)
>
> Es infraestructura silenciosa de la que dependen jugadores y servidores.
>
> Soy un desarrollador solitario que mantiene este proyecto con IA como motor. Solicito acceso a Claude Max 20x para seguir protegiendo a la comunidad.

URL: `claude.com/contact-sales/claude-for-oss`

### 4.2 OpenAI Codex — "Mantenimiento y seguridad" (sigue abierto, sin deadline)

> Soy un desarrollador solitario que mantiene un fork complejo con:
> - Diagnóstico automático de crashes
> - Escaneo de seguridad contra malware
> - Verificación de integridad de mods
>
> Necesito IA de élite para seguir protegiendo a la comunidad.

URL: `openai.com/form/codex-for-oss/`

---

## 📅 5. PLAN DE ACCIÓN — HISTÓRICO (semana 20-27 junio) + ABIERTOS AL 9 DE JULIO

La tabla original de v4.0 era un plan día a día para la semana del 20 al 27 de junio. Esa semana ya pasó — se deja como registro histórico de la intención original, no como plan activo:

| Día | Fecha | Acción | Herramienta | Estado real |
|---|---|---|---|---|
| Viernes | 20 junio | Enviar solicitud a Anthropic | Manual | ⏳ Sin confirmación de envío en `ESTADO.md` |
| Lunes | 23 junio | Actualizar README con enfoque "Seguridad e Infraestructura" | Manual / Gemini Flash | Ver `ESTADO.md` para estado real del README |
| Martes | 24 junio | Enviar solicitud a OpenAI Codex | Manual | ⏳ Sin confirmación de envío |
| Miércoles | 25 junio | Configurar Aider + DeepSeek V4 Pro | DeepSeek | Ver `ESTADO.md`, sesiones 14 en adelante |
| Jueves | 26 junio | Resolver pendientes con DeepSeek | DeepSeek + Aider | Ver `ESTADO.md` |
| Viernes | 27 junio | Publicar en Reddit/Discord | Gemini Flash | Ver `ESTADO.md` |

**Lo único que sigue realmente abierto y accionable hoy (9 de julio), sin depender de fechas ya vencidas:**
1. Enviar el formulario de OpenAI Codex for Open Source — sigue sin deadline, sigue sin confirmación de envío en el historial.
2. Publicar en r/feedthebeast, r/Minecraft, Discord de Prism Launcher — sigue pendiente según el historial de sesiones.
3. Decidir conscientemente si vale la pena reintentar Anthropic (ventana de 10.000 cupos probablemente cerrada) o dejarlo descartado sin más vueltas.

---

## 🔗 6. ENLACES

| Recurso | URL |
|---|---|
| Anthropic Claude for OSS | claude.com/contact-sales/claude-for-oss |
| OpenAI Codex for Open Source | openai.com/form/codex-for-oss/ |
| DeepSeek API | platform.deepseek.com |
| Google AI Studio | aistudio.google.com |
| Repositorio | github.com/ElPibeCapo/BetelineyLauncher |
| Discord | discord.gg/fMbSkEd85r |

---

## 📚 Fuentes verificadas en esta revisión (v5.0)

- `panorama-ia-julio-2026-verificado-v2.md` (corte 9 de julio de 2026) — verificado contra TechCrunch, VentureBeat, MarkTechPost, Axios, The Hill, Decoder, gHacks, blogs oficiales de Anthropic/x.ai/DeepReinforce/Sakana, METR.org, AUR/Arch, documentación oficial de DeepSeek/OpenCode.
- Auditoría cruzada de este documento contra `ESTADO.md` para confirmar qué solicitudes realmente se enviaron vs. cuáles quedaron solo planeadas — ver `ESTADO.md`, sección "Sesión 35".
- Fuentes de v4.0 (20 junio) que siguen vigentes sin cambios: documentación oficial de precios de DeepSeek, confirmaciones de terceros sobre Claude for Open Source.

*Fin del documento maestro v5.0 — 9 de julio de 2026. Ver `ESTADO.md`, sección "Sesión 35", para el detalle completo de qué se verificó y por qué.*
