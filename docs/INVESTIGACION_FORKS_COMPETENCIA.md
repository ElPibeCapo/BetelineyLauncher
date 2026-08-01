# Investigación — Forks de Prism Launcher (FjordLauncher, ShatteredPrism)

> Origen: el dueño pidió analizar dos forks (`FjordLauncher-develop.zip`, `ShatteredPrism-develop.zip`)
> subidos como código real, más un chat previo con GitHub Copilot que ya había comparado los tres
> proyectos. Este documento verifica ese análisis contra el código fuente real de ambos zips
> (no contra lo que Copilot afirmó) y corrige lo que no se sostenía. Es investigación pura —
> nada de lo de abajo se implementó en esta sesión, es insumo para decidir roadmap.

## Aviso sobre el análisis previo de Copilot

El chat de Copilot adjuntado tiene un problema de fondo antes de entrar en detalles técnicos:
está estructurado como halago, no como análisis. Usa estrellas de GitHub como proxy de "calidad"
(comparar 4 ⭐ de un proyecto de 51 días contra 376 ⭐ de uno con 1+ año no dice nada de la
calidad del código), inventa un puntaje "Beteliney = 95/100, +alt-auth = 100/100 UNDISPUTED KING"
sin ninguna métrica detrás, y la tabla "11/12 características únicas" cuenta cosas que ninguno de
los forks comparados intenta tener (comparar si Fjord tiene "Command Palette" es como evaluar un
auto por si tiene aire acondicionado en un test de frenos — Fjord no compite en esa categoría, no
es que "pierda" ahí). Nada de eso es mentira exactamente, pero el efecto neto es decirte lo que
un asistente cree que querés escuchar. Abajo va lo que sí verifiqué contra el código.

## FjordLauncher — qué es realmente

Fork legítimo y activo, mantenido por `@evan-goode` (`unmojang/FjordLauncher`), basado en PollyMC
(discontinuado). Su único objetivo declarado es soporte de servidores de autenticación alternativos
(Ely.by, LittleSkin, Drasl, cualquier servidor compatible con `authlib-injector`/Yggdrasil). Todo lo
demás que tiene (multiplataforma, empaquetado en Scoop/Homebrew/AUR/Nix/Flatpak) es exactamente lo
mismo que ya tiene Prism Launcher upstream — no es diferencial de Fjord, Copilot lo contó como si
lo fuera.

**"Loki" existe de verdad** (a diferencia de lo que sospeché al principio) — es un agente Java propio
de unmojang (`org.unmojang:Loki`), gestionado junto a `authlib-injector` para parchar auth/skins en
versiones donde el injector normal no aplica limpio. Confirmado en `launcher/minecraft/Agent.h`.

**Tamaño real del alt-auth** (no "copiar el módulo" como sugirió Copilot):

| Archivo nuevo | Líneas |
|---|---|
| `GetAuthlibInjectorApiLocation.cpp` | 83 |
| `minecraft/auth/steps/AuthlibInjectorMetadataStep.cpp` | 50 |
| `minecraft/auth/steps/YggdrasilMinecraftProfileStep.cpp` | 73 |
| `minecraft/auth/steps/YggdrasilStep.cpp` | 327 |
| `minecraft/skins/AuthlibInjectorTextureDelete.cpp` | 62 |
| `minecraft/skins/AuthlibInjectorTextureUpload.cpp` | 83 |
| `ui/dialogs/AuthlibInjectorLoginDialog.cpp` | 206 |
| **Total código nuevo** | **884** |

Además de esos archivos nuevos, Fjord modifica **6 archivos core compartidos**:
`minecraft/auth/AccountData.cpp/.h`, `minecraft/auth/AuthFlow.cpp`, `minecraft/auth/MinecraftAccount.cpp/.h`,
`minecraft/auth/Parsers.cpp`. Estos son los mismos archivos donde Beteliney ya hizo trabajo propio real
(el off-by-one de `AccountList::data()`, sesión 50 cont., y el reemplazo de interpolación de string por
`QJsonObject`/`QJsonDocument` en los 3 pasos de login OAuth/XSTS, sesión 53 — ver `ESTADO.md`). Portar
alt-auth no es copiar 7 archivos y listo: es reconciliar a mano contra fixes de seguridad propios ya
hechos ahí. Estimación honesta: trabajo real de una sesión completa dedicada, no un cherry-pick trivial.

## ShatteredPrism — qué es realmente

Fork de Fjord (no de Prism directo), de `Noctilune`. El propio README confirma lo que Copilot reportó
sobre mantenimiento: el autor está mudándose y avisa que el proyecto está semi-parado hasta agosto 2026
("*I am currently in the process of moving... little time to maintain the launcher currently*").
Aporta encima de Fjord solo dos cosas reales: quitar el requisito de Client ID propio de Microsoft
("DRM", ver abajo) y reactivar descarga de modpacks FTB. Todo el alt-auth que tiene es heredado de
Fjord, no es trabajo propio.

## El "DRM" no es DRM — y no aplica a Beteliney

Esto es lo que más se prestaba a confusión en el chat de Copilot, así que lo verifiqué a fondo (búsqueda
web + código). Prism Launcher exige que cada fork configure su **propio Client ID de Microsoft Azure**
para el login MSA — no es protección del contenido de Minecraft, es una medida anti-abuso: si todos los
forks compartieran el mismo Client ID que Prism, Microsoft podría banear ese ID por volumen/abuso y
tumbar el login para todos a la vez. PollyMC fue el primero en quitar ese requisito (permitiendo login
sin configurar nada); Fjord decidió **mantenerlo** a propósito; ShatteredPrism lo volvió a quitar,
siguiendo a PollyMC.

**Verificado en el código de Beteliney: esto no aplica.** Ya tenés tu propio Client ID registrado en
Azure y horneado en el build (`CMakeLists.txt`: `Launcher_MSA_CLIENT_ID = "4b945c78-d30b-489e-915f-b361bf9c933b"`,
propagado a `BuildConfig.MSA_CLIENT_ID` y usado en `MSAStep`/`MSADeviceCodeStep`). El login premium de
Beteliney ya funciona sin que el usuario configure nada — exactamente el estado al que Fjord/ShatteredPrism
étán tratando de llegar de formas distintas. No hay nada que portar ni decidir acá.

## FTB legacy — no es solo código, hay un pedido legal de por medio

Antes de anotar esto como "feature fácil de copiar" encontré el contexto real: en 2023 el equipo de
FTB le pidió formalmente a Prism Launcher que dejara de descargar modpacks vía su API
(`modpacks.ch`), porque bypasea la publicidad de la que depende su financiamiento
(`prismlauncher.org/news/ftb-removal/`). Prism cumplió el pedido y lo sacó. Fjord/ShatteredPrism lo
reactivaron sin permiso explícito de FTB — hay un PR en el repo oficial de Prism (#3559) donde un
mantenedor dice literalmente *"we were specifically asked to remove this and have received no
permission to add it back"*.

El código (`modplatform/legacy_ftb/`, `ui/pages/modplatform/legacy_ftb/`) probablemente ya existe
en el árbol de Beteliney sin tocar — es uno de los 45 archivos con diferencia real contra upstream
que ya tenés catalogados en `docs/AUDITORIA_MODULOS.md` ítem 7, no código nuevo que haya que portar.
Si en algún momento se evalúa reactivar el tab de FTB en la UI, es una decisión de riesgo/reputación
del proyecto (ir en contra de un pedido explícito de un content provider), no una decisión técnica —
señalado acá para que quede registrado, sin recomendación en ningún sentido.

## Conclusión — qué hay realmente para el roadmap

De las tres cosas que Copilot presentó como "agregale esto y sos el rey indiscutido", una no aplica
(DRM/Client ID, ya resuelto), una tiene una traba externa real que no es solo trabajo de programación
(FTB), y la única con valor real y sin trabas es **alt-auth vía authlib-injector/Yggdrasil** — con
costo honesto de ~884 líneas nuevas más reconciliación manual de 6 archivos core ya modificados por
Beteliney. Queda documentado acá como candidato de roadmap; no se tocó código en esta sesión, es
investigación pura a pedido del dueño.

Las features que Copilot listó como "únicas de Beteliney" (diagnóstico de logs en español, perfiles
JVM, BetelineyPacks, tema neón, Command Palette, etc.) sí están confirmadas reales contra `ESTADO.md`
y el propio código — en eso el chat no mintió, solo lo envolvió en un marco de marketing que no
aportaba nada al análisis.
