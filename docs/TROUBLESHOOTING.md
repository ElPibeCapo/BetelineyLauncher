# ⬡ Beteliney Launcher — Troubleshooting

> Guía de resolución de problemas frecuentes.

---

## 🐧 Linux

### El launcher no inicia / crash al abrir

**Síntoma:** `bash lanzar.sh` muestra error o la ventana se cierra inmediatamente.

```bash
# Lanzar con logging verbose para ver el error exacto:
bash lanzar.sh --debug 2>&1 | head -60
```

Si el problema es un plugin Qt que no carga:
```bash
QT_DEBUG_PLUGINS=1 ./build/beteliney
# Buscar líneas con "FAILED" o "Cannot load library"
```

---

### `StackOverflowError` con mods

**Síntoma:** El juego crashea con `java.lang.StackOverflowError` al cargar mods.

**Causa:** El valor por defecto de `-Xss` (stack por hilo) es insuficiente para mods con ASM/mixins que generan frames profundos.

**Solución:** En la configuración de la instancia → Java → Argumentos JVM, subir el stack:
```
-Xss1M    # mínimo recomendado para Forge/Fabric
-Xss2M    # si sigue fallando con muchos mixins
```
O usar el **Perfil Balanceado** que ya incluye `-Xss1M`.

---

### Minecraft lagea / stutters de GC

**Síntoma:** Caídas de FPS periódicas cada 5–15 segundos.

**Causa:** El GC (G1GC) hace colecciones mientras el juego corre.

**Solución:**
1. Usar el perfil JVM adecuado (iGPU → Ligero → Balanceado según RAM disponible)
2. Verificar que no estás asignando más del 60% de la RAM total del sistema
3. En sistemas con Vega 10: el perfil **iGPU** limita a 1.5 GB para no privar de RAM a la GPU

---

### `CMake Error: Qt6 not found`

```
CMake Error at CMakeLists.txt:XX:
  find_package called with invalid argument "Qt6"
```

**Solución:**
```bash
# En Nobara/Fedora:
sudo dnf install qt6-qtbase-devel qt6-qtnetworkauth-devel qt6-qtsvg-devel

# O usar el script:
bash COMPILAR_LINUX.sh   # → [4] instalar deps
```

---

### `tomlplusplus not found` durante configure

**Síntoma:**
```
CMake Error: Could not find a package configuration file provided by "tomlplusplus"
```

**Causa:** En Fedora, `tomlplusplus` registra su CMake en `/usr/lib64/cmake/tomlplusplus`, que CMake no encuentra automáticamente.

**El script `COMPILAR_LINUX.sh` ya pasa `-Dtomlplusplus_DIR=/usr/lib64/cmake/tomlplusplus`.**

Si compilás manualmente:
```bash
cmake -B build -Dtomlplusplus_DIR=/usr/lib64/cmake/tomlplusplus ...
```

---

### La partición Windows monta en read-only

**Síntoma:** `MONTAR_WINDOWS_NOBARA.sh` monta pero no podés escribir.

**Causa:** Windows dejó el disco en estado "sucio" (no apagó correctamente, Hibernación activa, Fast Startup).

**Solución:**
```bash
# Desactivar hibernación en Windows (ejecutar como admin en Windows):
powercfg /h off

# O forzar montaje en Linux (solo si sabés lo que hacés):
sudo ntfs-3g -o remove_hiberfile /dev/sdXY /mnt/windows
```

---

### Partición NTFS monta con `ntfs3` pero no funciona bien

**Síntoma:** Archivos corruptos o errores al copiar.

**Solución:** Usar `ntfs-3g` (más estable para escritura):
```bash
sudo umount /mnt/windows
sudo mount -t ntfs-3g /dev/sdXY /mnt/windows
```

---

## 🪟 Windows

### `ECM not found` al configurar CMake

```
CMake Error: Could not find a package configuration file provided by "ECM"
```

**Solución:**
```batch
:: En la carpeta del proyecto:
vcpkg_local\vcpkg.exe install ecm:x64-windows
```

---

### CMake bloqueado por AppLocker (equipos SENA / corporativos)

**Síntoma:**
```
[ERROR] CMake no encontrado o bloqueado (AppLocker)
```

**Causa:** En algunos equipos corporativos, AppLocker bloquea ejecutables fuera de `C:\Program Files`.

**Solución:**
1. Descargar CMake del instalador oficial: https://cmake.org/download/
2. Instalarlo en `C:\Program Files\CMake` (carpeta de sistema, usualmente no bloqueada)
3. Agregar al PATH durante la instalación
4. El script detecta `C:\Program Files\CMake\bin\cmake.exe` con prioridad máxima

---

### `vcpkg install` falla / timeout

**Síntoma:** `vcpkg.exe install` falla con error de red o timeout.

**Solución:**
```batch
:: Actualizar vcpkg primero:
cd vcpkg_local
git pull
bootstrap-vcpkg.bat

:: Luego volver a intentar:
cd ..\BetelineyLauncher\source
COMPILAR_BETELINEY.bat
```

---

### El `.exe` se abre y se cierra inmediatamente

**Síntoma:** `beteliney.exe` abre y cierra sin mostrar nada.

**Causa más común:** Falta una DLL de Qt o MinGW.

**Diagnóstico:**
```batch
:: Abrir una ventana de CMD en la carpeta del exe y ejecutar:
beteliney.exe
:: Ver el mensaje de error de DLL que falta
```

**Solución:** Correr `COMPILAR_BETELINEY.bat` → la opción de recompilar copia las DLLs automáticamente. Si el problema persiste, opción `[3]` (limpiar + rebuild).

---

### MinGW no encontrado / rutas distintas

**Síntoma:**
```
[ERROR] MinGW no encontrado en C:\Qt\Tools ni D:\Qt\Qt\Tools
```

**Causa:** Qt instalado en una ruta no estándar.

**Solución temporal:** Editar `COMPILAR_BETELINEY.bat` y agregar tu ruta en la lista del bloque de detección MinGW. Ejemplo:
```batch
for %%T in ("C:\Qt\Tools" "D:\Qt\Qt\Tools" "E:\MiQt\Tools") do (
```

O usar `COMPILAR.ps1` que tiene detección más flexible.

---

## 🔧 General

### ¿Cómo saber qué perfil JVM usar?

| Tu situación | Perfil recomendado |
|---|---|
| GPU integrada (Vega 10, Intel UHD) | **iGPU / RAM compartida** |
| Vanilla o <10 mods | **Ligero** |
| Fabric, OptiFine, 10–100 mods | **Balanceado** |
| Modpack grande (100–300 mods) | **Pesado** |
| Servidor local o ≥300 mods | **Extremo** |

**Regla de oro:** nunca asignar más del 60% de la RAM total. En iGPU, restar 1–1.5 GB adicional para la VRAM dinámica.

---

### El updater no encuentra actualizaciones

El updater (`beteliney-updater.sh`) hace `git fetch` al repositorio configurado en el `remote.origin.url` del repo local.

```bash
# Verificar remoto configurado:
cd BetelineyLauncher/source
git remote -v

# Verificar conexión:
bash beteliney-updater.sh --check
```

---

### Preguntas frecuentes

**¿Puedo jugar sin cuenta de Microsoft?**
Sí. Usá "Agregar cuenta No-Premium" en la sección de cuentas. El launcher lanza el juego directamente sin verificar con Mojang. Solo funciona en servidores que no requieren autenticación (modo offline).

**¿Los flags `-march=znver1` sirven en mi CPU?**
Solo si tu CPU es un AMD Ryzen 7 3700U (Picasso, Zen+). El script de compilación detecta automáticamente la CPU y usa `-march=native` para cualquier otra. El ejecutable resultante no es portable entre CPUs incompatibles.

**¿El AppImage funciona en cualquier Linux?**
En cualquier Linux x86_64 con glibc ≥ 2.31 (Ubuntu 20.04+, Fedora 32+, Nobara). En sistemas muy viejos puede fallar por incompatibilidad de glibc.


---

## Compilación en CachyOS / Arch Linux

### CMAKE_Java_COMPILER no encontrado

CachyOS instala `jdk-openjdk` (JDK completo con `javac`) y `jre21-openjdk` (solo JRE). El default de Java apunta al JRE 21 que no tiene `javac`.

**Fix**: el script `COMPILAR_LINUX.sh` ya detecta `javac` automáticamente y ajusta `JAVA_HOME` al JDK 26. Si se llama CMake directamente:
```bash
export JAVA_HOME=/usr/lib/jvm/java-26-openjdk
export PATH=$JAVA_HOME/bin:$PATH
```

### jar: El indicador 'c' necesita especificación de archivos

Bug de CMake 4.x + Java 21+/26: `UseJava` genera un `java_class_filelist` vacío cuando el path contiene espacios. Java 26 rechaza crear un JAR desde un argfile vacío.

**Fix aplicado en sesión 21**: `libraries/launcher/CMakeLists.txt` y `libraries/javacheck/CMakeLists.txt` reescritos con `add_custom_command` y `jar --create -C dir .` (sin argfiles).

### -Werror=sfinae-incomplete= en GCC 16

GCC 16 introduce el warning `-Wsfinae-incomplete=` que Qt 6.x dispara en `qmetatype.h`. Con `-Werror` activo bloquea toda la compilación.

**Fix aplicado**: `-Wno-sfinae-incomplete` condicionado a `GCC >= 16.0` en `launcher/CMakeLists.txt`.

### WrapVulkanHeaders not found

Instalar el paquete:
```bash
sudo pacman -S vulkan-headers
```

### tomlplusplus no encontrado por CMake

En Arch el path es `/usr/lib/cmake/tomlplusplus` (no `/usr/lib64` de Fedora). El `COMPILAR_LINUX.sh` ya lo detecta automáticamente. Si se llama CMake directamente:
```bash
cmake -B build -Dtomlplusplus_DIR=/usr/lib/cmake/tomlplusplus ...
```

### zlib no encontrado

CachyOS usa `zlib-ng` como reemplazo de `zlib`. Instalar la capa de compatibilidad:
```bash
sudo pacman -S zlib-ng-compat
```
