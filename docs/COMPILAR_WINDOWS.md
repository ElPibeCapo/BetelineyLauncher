# ⬡ Beteliney Launcher — Guía de Compilación y Distribución

## 🐧 Linux (Nobara / Fedora / Ubuntu / Arch)

### Primera vez

```bash
# Si el proyecto está en el home:
cd ~/BetelineyLauncher_v7/BetelineyLauncher/source

# Si viene de montar la partición Windows:
bash MONTAR_WINDOWS_NOBARA.sh   # detecta usuario dinámicamente y ofrece copiar al home

bash COMPILAR_LINUX.sh          # → [4] instalar deps, luego [1] build completo
```

### Flujo normal (después de la primera vez)

```bash
bash lanzar.sh                  # lanza directamente (compila si falta el exe)
bash beteliney-updater.sh       # git pull + recompila
```

### Opciones de COMPILAR_LINUX.sh

| Opción | Acción |
|--------|--------|
| `[1]` | Build completo (primera vez) |
| `[2]` | Recompilar rápido (incremental) |
| `[3]` | Limpiar + rebuild |
| `[4]` | Solo instalar dependencias |
| `[5]` | Crear AppImage distribuible |

### Crear AppImage (distribuible en cualquier Linux x86_64)

```bash
bash COMPILAR_LINUX.sh   # → [5]
# Genera: build/BetelineyLauncher-YYYYMMDD-x86_64.AppImage
```

### Dependencias (se instalan solas con `[4]`)

**Fedora / Nobara:**
```
cmake  ninja-build  git  gcc-c++
qt6-qtbase-devel  qt6-qtnetworkauth-devel  qt6-qtsvg-devel
qt6-qtbase-private-devel  extra-cmake-modules
libarchive-devel  cmark-devel  qrencode-devel  zlib-devel
tomlplusplus-devel  gamemode-devel
java-21-openjdk-devel  desktop-file-utils  libappstream-glib
```

---

## 🪟 Windows

### Compilar

```
COMPILAR_BETELINEY.bat   ← doble clic
  → [1] build completo  (primera vez, ~10-30 min)
  → [2] recompilar rápido (siempre después)
```

**Requisitos:**
- Qt 6.x con MinGW 64-bit → https://www.qt.io/download-qt-installer
- Java 21 (Temurin) → https://adoptium.net
- vcpkg incluido en `vcpkg_local/` — no requiere instalación extra

### Empaquetar para distribuir

```
EMPAQUETAR_WINDOWS.bat   ← doble clic
  → [1] ZIP portable   (extraer y ejecutar, sin instalador)
  → [2] Installer .exe (requiere NSIS: https://nsis.sourceforge.io)
```

Genera en `source\dist\`:
- `BetelineyLauncher-YYYYMMDD-Windows-x64.zip`
- `BetelineyLauncher-YYYYMMDD-Setup.exe`

---

## 📋 Resumen de scripts

| Script | OS | Función |
|--------|----|---------|
| `lanzar.sh` | Linux | Lanza el launcher; compila automáticamente si falta el exe |
| `beteliney-updater.sh` | Linux | `git pull` + recompila |
| `COMPILAR_LINUX.sh` | Linux | Build manual con menú interactivo |
| `COMPILAR_BETELINEY.bat` | Windows | Build manual con menú interactivo |
| `COMPILAR.ps1` | Windows | Build PowerShell (detección dinámica de Qt/MinGW/Java) |
| `EMPAQUETAR_WINDOWS.bat` | Windows | ZIP portable o installer NSIS |
| `MONTAR_WINDOWS_NOBARA.sh` | Linux | Monta partición NTFS (detecta usuario dinámicamente) |

---

## ⚠️ Notas

- El AppImage corre en cualquier Linux x86_64 sin instalar nada.
- El ZIP de Windows: extraer y ejecutar `beteliney.exe`, no requiere instalación.
- Los flags `-march=znver1` son específicos del **Ryzen 7 3700U**. En otra CPU el script detecta automáticamente y usa `-march=native`.
- **MinGW vs MSVC**: ABI incompatibles. No mezcles DLLs de builds MSVC con este build MinGW → crashes garantizados. El bat copia automáticamente las DLLs correctas.
