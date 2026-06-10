<p align="center">
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="/program_info/com.beteliney.BetelineyLauncher.logo-darkmode.svg">
  <source media="(prefers-color-scheme: light)" srcset="/program_info/com.beteliney.BetelineyLauncher.logo.svg">
  <img alt="Beteliney Launcher" src="/program_info/com.beteliney.BetelineyLauncher.logo.svg" width="40%">
</picture>
</p>

<p align="center">
  <b>Beteliney Launcher</b> v8.2.0 — launcher personalizado para Minecraft.<br />
  Gestiona múltiples instalaciones con perfiles JVM optimizados, detección automática de iGPU,<br />
  soporte No-Premium (offline), tema gamer neón y build optimizado para AMD Ryzen / Vega 10.<br /><br />
  Basado en <a href="https://github.com/PrismLauncher/PrismLauncher">Prism Launcher</a> (GPL-3.0).
</p>

---

## Compilar

### Linux (Nobara / Fedora / Ubuntu / Arch)

```bash
cd BetelineyLauncher/source
bash COMPILAR_LINUX.sh   # → [4] instalar deps  (solo primera vez)
bash COMPILAR_LINUX.sh   # → [1] build completo (~10-30 min primera vez)
# Después siempre:
bash lanzar.sh           # lanza directamente, compila si falta el exe
```

### Windows

```
Doble clic en COMPILAR_BETELINEY.bat
  → [1] build completo  (primera vez)
  → [2] recompilar      (siempre después)
```

**Requisitos Windows:** Qt 6.x con MinGW 64-bit · Java 21 · vcpkg incluido en `vcpkg_local/`

Guía detallada: [`docs/COMPILAR_WINDOWS.md`](docs/COMPILAR_WINDOWS.md)

---

## Distribuir / Empaquetar

### Linux

```bash
bash EMPAQUETAR_LINUX.sh
  # [1] tar.gz portable  — funciona sin FUSE, cualquier Linux x86_64
  # [2] AppImage         — requiere FUSE
  # [3] .deb             — solo sistemas con apt (Debian/Ubuntu)
```

### Windows

```
EMPAQUETAR_WINDOWS.bat
  → [1] ZIP portable   (sin instalador)
  → [2] Installer .exe (requiere NSIS)
```

---

## Perfiles JVM

Selección automática al abrir Configuración → Java:

| Perfil | RAM mín | RAM máx | Uso recomendado |
|--------|---------|---------|----------------|
| iGPU / RAM compartida | 384 MB | 1536 MB | Vega 10 / Intel UHD — solo Vanilla |
| Ligero | 512 MB | 2 GB | Vanilla / <10 mods |
| Balanceado | 2 GB | 4 GB | Fabric / Forge ligero / 10–100 mods |
| Pesado | 2 GB | 6 GB | Modpacks 100–300 mods |
| Extremo | 6 GB | 12 GB | Servidores locales / ≥300 mods |
| Personalizado | — | — | Configuración manual |

El launcher detecta automáticamente si hay GPU integrada (lspci en Linux, wmic en Windows) y sugiere el perfil iGPU si corresponde. Documentación técnica: [`docs/PERFILES_JVM.md`](docs/PERFILES_JVM.md)

---

## Diferencias con Prism Launcher

- **Logo propio**: hexágono ⬡ con B neón `#39FF14`, no el prisma de Prism
- **Tema BetelineyTheme v3**: UI gamer completa (deep-space navy + neón)
- **Perfiles JVM Aikar** preconfigurados y ajustados (`BetelineyProfiles.h`)
- **Detección automática de iGPU** para sugerir perfil JVM (lspci / wmic)
- **Cuentas No-Premium** (offline sin Microsoft) soportadas directamente
- **`suitableMaxMem`** conservador para sistemas con iGPU (Vega 10)
- **Build optimizado**: `-O3 -march=znver1`, LTO, `--gc-sections`, fix GCC 15 libnbtplusplus
- **DPI PerMonitorV2** en el manifest de Windows
- **Scripts propios**: `lanzar.sh`, `beteliney-updater.sh`, `EMPAQUETAR_LINUX.sh`
- **Branding completo**: AppID `com.beteliney.BetelineyLauncher`, domain `beteliney.github.io`

---

## Estructura

```
source/
├── launcher/               Código fuente C++/Qt
│   ├── BetelineyProfiles.h Perfiles JVM (6 perfiles, header-only)
│   ├── Application.cpp     Init, settings, capabilities
│   ├── SysInfo.cpp         Detección RAM (Win/Linux/macOS/BSD)
│   └── ui/
│       ├── themes/         BetelineyTheme.cpp — stylesheet neón completo
│       ├── dialogs/        AboutDialog personalizado
│       └── widgets/        JavaSettingsWidget — perfiles + detección iGPU
├── program_info/           SVG/PNG/ICO/manifest/desktop/metainfo
├── docs/
│   ├── CHANGELOG.md        Historial completo de cambios
│   ├── ARQUITECTURA.md     Documento técnico completo
│   ├── PERFILES_JVM.md     Documentación técnica flags G1GC
│   ├── TROUBLESHOOTING.md  Guía de problemas frecuentes
│   └── COMPILAR_WINDOWS.md Guía de compilación detallada
├── tests/                  Tests unitarios (26 suites, Qt Test)
└── tools/pc/               Scripts de utilidad del sistema
```

---

## Documentación

| Documento | Contenido |
|-----------|-----------|
| [`docs/ARQUITECTURA.md`](docs/ARQUITECTURA.md) | Stack técnico, módulos C++, branding, cross-platform, build |
| [`docs/PERFILES_JVM.md`](docs/PERFILES_JVM.md) | Cada flag G1GC explicado, tabla RAM, perfil custom |
| [`docs/TROUBLESHOOTING.md`](docs/TROUBLESHOOTING.md) | Problemas frecuentes Linux y Windows |
| [`docs/CHANGELOG.md`](docs/CHANGELOG.md) | Historial de cambios desde v7.0.0 |
| [`LEEME.txt`](LEEME.txt) | Guía rápida: compilar, lanzar, actualizar, distribuir |

---

## Licencia

Código del launcher: **GPL-3.0-only** — mismo que Prism Launcher.

Logo y assets visuales: **CC BY-SA 4.0**.

No afiliado con el proyecto Prism Launcher (https://prismlauncher.org).
