<p align="center">
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="/program_info/com.beteliney.BetelineyLauncher.logo-darkmode.svg">
  <source media="(prefers-color-scheme: light)" srcset="/program_info/com.beteliney.BetelineyLauncher.logo.svg">
  <img alt="Beteliney Launcher" src="/program_info/com.beteliney.BetelineyLauncher.logo.svg" width="40%">
</picture>
</p>

<p align="center">
  <b>BetelineyLauncher</b> v8.4.0 — launcher personalizado para Minecraft.<br />
  Gestión de instancias, diagnóstico inteligente de crashes, perfiles JVM optimizados,<br />
  BetelineyPacks, protección contra malware, importar desde GDLauncher y más.<br /><br />
  Basado en <a href="https://github.com/PrismLauncher/PrismLauncher">Prism Launcher</a> (GPL-3.0).
</p>

<p align="center">
  <a href="https://discord.gg/fMbSkEd85r"><img alt="Discord" src="https://img.shields.io/badge/Discord-Únete-5865F2?style=for-the-badge&logo=discord&logoColor=white"></a>
</p>

<p align="center">
  <img src="screenshots/ventana-principal.png" alt="Ventana principal de Beteliney Launcher" width="85%">
</p>
<p align="center"><i>Ventana principal — instancias con tema neón.</i></p>

<p align="center">
  <img src="screenshots/betelineypacks.png" alt="BetelineyPacks" width="49%">
  <img src="screenshots/perfiles-jvm.png" alt="Perfiles JVM" width="49%">
</p>
<p align="center"><i>BetelineyPacks (izquierda) — Perfiles JVM calibrados (derecha).</i></p>

---

## Features Beteliney

### Diagnóstico inteligente de logs
Cuando Minecraft crashea, el launcher analiza el log automáticamente y muestra en español exactamente qué pasó y cómo arreglarlo. 18 tipos de error detectados: falta de memoria, mods duplicados, dependencias faltantes, conflictos de Mixin, Java incorrecto, OpenGL sin aceleración, malware Fractureiser, y más.

### Perfiles JVM calibrados
7 perfiles preconfigurados con flags Aikar's G1GC calibrados para cada caso de uso:

| Perfil | RAM | Uso |
|--------|-----|-----|
| iGPU / RAM compartida | 384–1536 MB | Vega 10 / Intel UHD — Vanilla |
| iGPU ZGC (Java 21+) | 384–1536 MB | Pausas <1ms con ZGC generacional |
| Ligero Vanilla | 512–2048 MB | Sin mods o muy pocos |
| Balanceado | 2–4 GB | Fabric / Forge con mods moderados |
| Pesado 100–300 mods | 2–6 GB | Modpacks medianos |
| Extremo ≥300 mods | 6–12 GB | Modpacks enormes / servers locales |
| Personalizado | manual | Configuración manual completa |

Auto-detección de iGPU (AMD y Intel) con sugerencia automática del perfil. Badge GraalVM si se detecta.

### BetelineyPacks
Plataforma propia de modpacks curados, integrada como primera pestaña en "Nueva instancia". Incluye presets built-in sin internet: Vanilla Optimizado (Sodium + Lithium + Iris + ModernFix), PvP Competitivo, y Modpack Pesado NeoForge.

### Protección contra malware
El launcher descarga una lista negra de hashes de malware conocido (Fractureiser y variantes) y verifica cada mod antes de instalarlo. Detección también en el análisis de logs.

### Verificación de conflictos de mods
Antes de lanzar Minecraft, el launcher escanea la carpeta de mods, detecta mod IDs duplicados y avisa en el log de lanzamiento con el nombre exacto del conflicto.

### Crash reporter
Si el launcher mismo crashea, captura el backtrace automáticamente (Linux: sigaction, Windows: MiniDump) y al siguiente inicio ofrece reportarlo en GitHub con un clic.

### Importar desde GDLauncher Carbon
Detecta automáticamente la instalación de GDLauncher Carbon, lee sus instancias y las convierte al formato Prism. Acceso en **Archivo → Importar desde GDLauncher Carbon**.

### Botón "Optimizar" en instancias Fabric
En la página de versión de cualquier instancia Fabric/Quilt, un botón "Optimizar (rendimiento)" instala automáticamente Sodium, Lithium, Iris y ModernFix si no están presentes.

### Cuentas sin Microsoft (offline/No-Premium)
Soporte completo de cuentas offline sin necesidad de Microsoft.

### Tema gamer neón
BetelineyTheme v5: fondo deep-space `#080912`, acento neón `#39FF14`, cyan `#00D4FF`, fuente JetBrains Mono. Cards de instancias con bordes neón y animaciones.

### Command Palette (Ctrl+K) + servidores favoritos
Paleta de comandos estilo editor de código para saltar entre instancias y settings, con quick-join directo a servidores favoritos guardados.

### Sistema de logros por tiempo jugado
Marca hitos de tiempo jugado con toasts de logro en pantalla — gamificación ligera de la marca Beteliney.

### Sandboxing opcional con Bubblewrap (Linux)
Aislamiento a nivel de proceso del juego vía `bwrap`, opt-in y sin interferir con GameMode/MangoHud.

### Backup manual de mundos + badge de actualización de mods
Botón de backup manual de mundos en la página de instancia, y badge silencioso que avisa cuando hay actualizaciones de mods disponibles al seleccionar una instancia.

### Actualizaciones firmadas (Ed25519)
El updater verifica la firma Ed25519 de cada release antes de aplicarla — fail-closed: si la firma no valida, no se instala.

---

## Compilar

### Linux

```bash
cd BetelineyLauncher/source
bash COMPILAR_LINUX.sh   # → [4] instalar deps (solo primera vez)
bash COMPILAR_LINUX.sh   # → [1] build completo
```

### Windows

```
Doble clic en COMPILAR_BETELINEY.bat → [1] build completo
```

Guía detallada: [`docs/COMPILAR_WINDOWS.md`](docs/COMPILAR_WINDOWS.md)

---

## Empaquetar

### Linux

```bash
bash EMPAQUETAR_LINUX.sh      # tar.gz portable
bash EMPAQUETAR_APPIMAGE.sh   # AppImage (auto-descarga herramientas)
```

### Windows

```
EMPAQUETAR_WINDOWS.bat → [1] ZIP portable / [2] Installer .exe (NSIS)
```

---

## CI/CD

GitHub Actions compila automáticamente Linux y Windows en cada push a `main` y publica una Release en cada tag `v*.*.*`.

```bash
git tag --list   # confirmar el último tag antes de elegir el nuevo — v8.3.0 y v8.4.0 ya existen
git tag v8.5.0 && git push --tags   # ejemplo — usar el número siguiente real
# Release publicada en ~15 min
```

---

## Estructura de archivos Beteliney

```
launcher/
├── BetelineyCode.h                    Easter egg (B-E-T-E)
├── BetelineyProfiles.h                7 perfiles JVM calibrados
├── crash/BetelineyPanicHandler.h/cpp  Crash reporter (Linux + Windows)
├── logs/BetelineyLogAnalyzer.h/cpp    Motor diagnóstico (18 checks)
├── migration/GDLauncherMigrator.h/cpp Importador GDLauncher Carbon
├── minecraft/mod/MalwareScanner.h/cpp Lista negra de malware
├── launch/steps/CheckModConflicts.h/cpp Verificador pre-lanzamiento
├── modplatform/beteliney/             BetelineyPacks (backend completo)
├── BetelineyAchievements.h/cpp        Sistema de logros por tiempo jugado
├── BubblewrapSandbox.h/cpp            Sandboxing opcional del proceso Minecraft (Linux)
├── FavoriteServers.h/cpp              Servidores favoritos persistidos
├── updater/betelineyupdater/          Updater con verificación de firma Ed25519
├── ui/themes/BetelineyTheme.cpp       Tema visual (810 líneas QSS)
├── ui/widgets/JavaSettingsWidget.cpp  Selector de perfiles JVM
├── ui/widgets/AchievementToast.h/cpp  Toast visual de logros
├── ui/dialogs/CommandPaletteDialog.h/cpp    Paleta de comandos (Ctrl+K)
├── ui/dialogs/FavoriteServersDialog.h/cpp   UI de gestión de servidores favoritos
└── ui/pages/modplatform/beteliney/    BetelineyPackPage (UI)

tools/dev/                             8 herramientas de desarrollo (build_fast.sh, check_ci.sh, secret_scan.sh, etc. — ver tools/dev/README.md)

packaging/
└── com.beteliney.BetelineyLauncher.json  Manifest Flatpak

docs/
├── CHANGELOG.md       Historial completo de cambios
├── ARQUITECTURA.md    Documento técnico de módulos
├── PERFILES_JVM.md    Flags G1GC explicados
├── TROUBLESHOOTING.md Problemas frecuentes y soluciones
└── COMPILAR_WINDOWS.md Guía de compilación Windows

ESTADO.md              Estado completo del proyecto (fuente de verdad)
EMPAQUETAR_APPIMAGE.sh Script para generar AppImage
```

---

## Diferencias con Prism Launcher

- Diagnóstico inteligente de crashes con 18 patrones de error
- BetelineyPacks — plataforma propia de modpacks curados
- Protección contra malware (hash blacklist desde META server)
- Verificación de conflictos de mods antes de lanzar
- Crash reporter con backtrace automático
- Importador de instancias desde GDLauncher Carbon
- Botón "Optimizar" para instancias Fabric con mods de rendimiento
- 7 perfiles JVM calibrados con auto-detección de iGPU
- Badge GraalVM CE en JavaSettings
- BetelineyTheme v5 — UI gamer completa (deep-space + neón)
- Cuentas No-Premium (offline) sin restricciones
- SmartScreen bypass documentado en el instalador Windows
- Manifest Flatpak y script AppImage incluidos
- Build optimizado: `-O3 -march=znver1`, LTO, sin Prism en ningún string

---

## Roadmap

- [x] Activar GitHub Pages en el repo `meta` (causa raíz era `build_type` mal configurado, no falta de activación)
- [x] 3 BetelineyPacks publicados (Vanilla Optimizado, PvP Competitivo, Pesado NeoForge — mods verificados contra la API de Modrinth)
- [x] `known-hashes.json` en el meta server — publicado vacío por diseño honesto (no había fuente pública de hashes reales de Fractureiser al momento de crearlo). **Actualización:** sí existe una fuente real (MalwareBazaar/abuse.ch, API pública) — sembrar la lista con datos reales queda en el plan priorizado (ver checklist de abajo)
- [x] Rotación de la CurseForge API key expuesta + secret cargado en CI
- [x] Capturas de BetelineyPacks y perfiles JVM
- [ ] Captura del panel de diagnóstico de logs (requiere forzar un crash de lanzamiento para activarlo)
- [ ] Flathub — submitear el manifest Flatpak para revisión oficial
- [ ] macOS — activar CI (el código heredado de Prism ya soporta Sparkle updater + entitlements)
- [ ] Certificado de firma de código Windows (elimina el aviso de SmartScreen)
- [ ] i18n propio conectado a Weblate o similar
- [ ] Publicar en r/feedthebeast, r/Minecraft, Discord de Prism Launcher

**Ideas de mejora ya implementadas** (corregido en esta revisión — estaban listadas abajo como "no confirmadas aún" pero ya están en `main`, ver `ESTADO.md` y `docs/CHANGELOG.md` sección "Sin publicar aún"):

- [x] Backup manual de mundos antes de actualizar una instancia (botón en página de instancia)
- [x] Badge de mods con actualización disponible en la card de instancia
- [x] Servidores favoritos + quick-join desde el dashboard
- [x] Búsqueda universal tipo Ctrl+K (Command Palette — instancias, settings)
- [x] Sistema de logros ligado a la marca (gamificación por tiempo jugado)
- [x] Sandboxing opcional del proceso Minecraft con Bubblewrap (Linux)
- [x] Firma Ed25519 fail-closed en el updater

**Ideas de mejora futuras (investigadas y verificadas, no confirmadas aún — ver bloque "ESTADO CONSOLIDADO" al final de `ESTADO.md` para el plan completo y priorizado):**

- [ ] Sembrar `known-hashes.json` con datos reales de MalwareBazaar (ver corrección arriba — bloqueado por API key de abuse.ch, requiere que el usuario la consiga)
- [ ] Discord Rich Presence a nivel de proceso launcher/juego
- [ ] Preset de BetelineyPacks con fuente CurseForge (esfuerzo medio — limitado por el "Project Distribution Toggle" de CurseForge)
- [ ] Tema de alto contraste (accesibilidad — ningún launcher competidor lo tiene)
- [ ] Búsqueda combinada Modrinth+CurseForge en una sola pestaña (mediano/largo plazo)

---

## Licencia

Código del launcher: **GPL-3.0-only** — mismo que Prism Launcher.
Logo y assets visuales: **CC BY-SA 4.0**.
No afiliado con el proyecto Prism Launcher.
