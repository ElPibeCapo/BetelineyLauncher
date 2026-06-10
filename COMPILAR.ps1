# COMPILAR.ps1 - Beteliney Launcher v7
# Ejecutar: clic derecho -> "Ejecutar con PowerShell"
# O desde PowerShell: cd D:\BetelineyLauncher_v7\BetelineyLauncher\source; .\COMPILAR.ps1

Set-Location $PSScriptRoot
$ErrorActionPreference = "Stop"

Write-Host ""
Write-Host "  ==========================================" -ForegroundColor Cyan
Write-Host "   Beteliney Launcher  |  Build Script v7" -ForegroundColor Cyan
Write-Host "  ==========================================" -ForegroundColor Cyan
Write-Host ""

# ============================================================
# RUTAS — detección dinámica, sin hardcodear versiones ni usuarios
# ============================================================

# vcpkg_local incluido en el proyecto (relativo al script)
$SCRIPT_DIR = $PSScriptRoot
$REPO_ROOT  = Split-Path (Split-Path $SCRIPT_DIR -Parent) -Parent
$VCPKG      = Join-Path $REPO_ROOT "vcpkg_local"
$BUILD      = Join-Path $SCRIPT_DIR "build"
$VCPKG_INS  = Join-Path $BUILD "vcpkg_installed"

# ── Detectar Qt (busca la instalación más reciente de MinGW 64-bit) ────────
function Find-Qt {
    $roots = @("C:\Qt", "D:\Qt\Qt", "D:\Qt", "$env:LOCALAPPDATA\Qt", "$env:PROGRAMFILES\Qt")
    foreach ($root in $roots) {
        if (-not (Test-Path $root)) { continue }
        # Ordena versiones descendente → toma la más nueva
        $vers = Get-ChildItem $root -Directory -ErrorAction SilentlyContinue |
                Where-Object { $_.Name -match '^\d+\.\d+' } |
                Sort-Object Name -Descending
        foreach ($ver in $vers) {
            $kit = Join-Path $ver.FullName "mingw_64"
            if (Test-Path (Join-Path $kit "bin\qmake.exe")) {
                return $kit
            }
        }
    }
    return $null
}

# ── Detectar MinGW (busca mingw*_64 en las Tools del Qt encontrado) ────────
function Find-MinGW($qtRoot) {
    $toolsDir = Join-Path (Split-Path (Split-Path $qtRoot -Parent) -Parent) "Tools"
    if (Test-Path $toolsDir) {
        $mg = Get-ChildItem $toolsDir -Directory -ErrorAction SilentlyContinue |
              Where-Object { $_.Name -match '^mingw\d+_64$' } |
              Sort-Object Name -Descending | Select-Object -First 1
        if ($mg -and (Test-Path (Join-Path $mg.FullName "bin\gcc.exe"))) {
            return $mg.FullName
        }
    }
    # Fallback: PATH
    $gcc = Get-Command gcc -ErrorAction SilentlyContinue
    if ($gcc) { return Split-Path (Split-Path $gcc.Source -Parent) -Parent }
    return $null
}

# ── Detectar Ninja ─────────────────────────────────────────────────────────
function Find-Ninja($qtRoot) {
    $toolsDir = Join-Path (Split-Path (Split-Path $qtRoot -Parent) -Parent) "Tools"
    $n = Get-ChildItem $toolsDir -Recurse -Filter "ninja.exe" -ErrorAction SilentlyContinue |
         Select-Object -First 1
    if ($n) { return $n.FullName }
    $nc = Get-Command ninja -ErrorAction SilentlyContinue
    if ($nc) { return $nc.Source }
    return $null
}

# ── Detectar Java 21+ ──────────────────────────────────────────────────────
function Find-Java {
    $roots = @(
        "$env:PROGRAMFILES\Eclipse Adoptium",
        "$env:PROGRAMFILES\Microsoft",
        "$env:PROGRAMFILES\Java",
        "$env:PROGRAMFILES\BellSoft",
        "$env:PROGRAMFILES\Azul Systems\Zulu",
        "C:\Program Files\Eclipse Adoptium",
        "C:\Program Files\Java"
    )
    foreach ($root in $roots) {
        if (-not (Test-Path $root)) { continue }
        $jdks = Get-ChildItem $root -Directory -ErrorAction SilentlyContinue |
                Where-Object { $_.Name -match 'jdk-?(2[1-9]|[3-9]\d)' } |
                Sort-Object Name -Descending
        foreach ($jdk in $jdks) {
            if (Test-Path (Join-Path $jdk.FullName "bin\java.exe")) {
                return $jdk.FullName
            }
        }
    }
    # Fallback: JAVA_HOME o PATH
    if ($env:JAVA_HOME -and (Test-Path "$env:JAVA_HOME\bin\java.exe")) {
        return $env:JAVA_HOME
    }
    $jc = Get-Command java -ErrorAction SilentlyContinue
    if ($jc) { return Split-Path (Split-Path $jc.Source -Parent) -Parent }
    return $null
}

Write-Host "  [AUTO] Detectando herramientas..." -ForegroundColor Yellow
$QT_ROOT = Find-Qt
if (-not $QT_ROOT) {
    Write-Host "  [ERROR] Qt MinGW 64-bit no encontrado." -ForegroundColor Red
    Write-Host "          Instala Qt desde https://www.qt.io/download-open-source" -ForegroundColor Red
    Read-Host "  Enter para salir"; exit 1
}
$QT_VER  = Split-Path (Split-Path $QT_ROOT -Parent) -Leaf
$MINGW   = Find-MinGW $QT_ROOT
$NINJA   = Find-Ninja $QT_ROOT
$JAVA    = Find-Java
$ECM     = Join-Path $VCPKG "packages\ecm_x64-windows\share\ECM"


# ============================================================
# DETECTAR CMAKE (orden: sistema -> PATH -> Qt Tools)
# cmake de Qt Tools puede estar bloqueado por AppLocker en equipos corporativos.
# ============================================================
$CMAKE = $null
$cmakeCandidates = @(
    "C:\Program Files\CMake\bin\cmake.exe",
    "D:\Qt\Qt\Tools\CMake_64\bin\cmake.exe"
)
foreach ($c in $cmakeCandidates) {
    if (Test-Path $c) {
        try {
            & $c --version 2>&1 | Out-Null
            if ($LASTEXITCODE -eq 0) { $CMAKE = $c; break }
        } catch {}
    }
}
if (-not $CMAKE) {
    $found = Get-Command cmake -ErrorAction SilentlyContinue
    if ($found) {
        try {
            cmake --version 2>&1 | Out-Null
            if ($LASTEXITCODE -eq 0) { $CMAKE = "cmake" }
        } catch {}
    }
}
if (-not $CMAKE) {
    Write-Host "  [ERROR] CMake no encontrado o bloqueado (AppLocker)." -ForegroundColor Red
    Write-Host "          Instala CMake desde https://cmake.org/download/ y agrega al PATH." -ForegroundColor Red
    Read-Host "  Enter para salir"; exit 1
}


# ============================================================
# VERIFICAR HERRAMIENTAS
# ============================================================
Write-Host "  --- Verificando herramientas ---" -ForegroundColor Yellow
Write-Host ""

$checks = @(
    @{ Name="Qt $QT_VER MinGW"; Path="$QT_ROOT\bin\qmake.exe" },
    @{ Name="MinGW gcc";        Path="$MINGW\bin\gcc.exe" },
    @{ Name="Ninja";            Path=$NINJA },
    @{ Name="vcpkg";            Path="$VCPKG\vcpkg.exe" },
    @{ Name="ECM";              Path="$ECM\ECMConfig.cmake" },
    @{ Name="Java 21+";         Path="$JAVA\bin\java.exe" }
)

$ok = $true
foreach ($c in $checks) {
    if (Test-Path $c.Path) {
        Write-Host "  [OK] $($c.Name)" -ForegroundColor Green
    } else {
        Write-Host "  [ERROR] $($c.Name) no encontrado en: $($c.Path)" -ForegroundColor Red
        $ok = $false
    }
}
Write-Host "  [OK] CMake: $CMAKE" -ForegroundColor Green

if (-not $ok) {
    Write-Host ""; Write-Host "  Faltan herramientas. Revisa las rutas." -ForegroundColor Red
    Read-Host "  Enter para salir"; exit 1
}


# ============================================================
# MENU
# ============================================================
Write-Host ""
Write-Host "  Elige modo:" -ForegroundColor Cyan
Write-Host "    [1]  Build completo    (primera vez)"
Write-Host "    [2]  Recompilar        (solo lo que cambio, rapido)"
Write-Host "    [3]  Limpiar + rebuild (borra build\ entero)"
Write-Host "    [4]  Solo configurar   (cmake configure sin compilar)"
Write-Host "    [5]  Debug build       (con simbolos de depuracion)"
Write-Host "    [0]  Salir"
Write-Host ""

$hasBuild = Test-Path "$BUILD\CMakeCache.txt"
if (-not $hasBuild) {
    Write-Host "  Nota: no hay build previo, [2] usara [1]." -ForegroundColor DarkGray
}

$MODE = Read-Host "  Opcion [1]"
if ($MODE -eq "") { $MODE = "1" }
if ($MODE -eq "0") { exit 0 }
if ($MODE -eq "2" -and -not $hasBuild) { $MODE = "1" }

if ($MODE -eq "3") {
    Write-Host "  Borrando build\..." -ForegroundColor Yellow
    if (Test-Path $BUILD) { Remove-Item -Recurse -Force $BUILD }
    Write-Host "  [OK] Limpio." -ForegroundColor Green
    $MODE = "1"
}

$BUILD_TYPE = if ($MODE -eq "5") { "RelWithDebInfo" } else { "Release" }
$LTO       = if ($MODE -eq "5") { "OFF" } else { "ON" }


# ============================================================
# PATH
# ============================================================
$env:PATH = "$MINGW\bin;$QT_ROOT\bin;$JAVA\bin;" + $env:PATH

# ============================================================
# .git MINIMO (para que cmake no falle buscando version git)
# ============================================================
if (-not (Test-Path ".git\HEAD")) {
    New-Item -ItemType Directory -Force ".git\refs\heads" | Out-Null
    Set-Content ".git\HEAD" "ref: refs/heads/master"
    Set-Content ".git\refs\heads\master" "0000000000000000000000000000000000000000"
    New-Item -Force ".git\packed-refs" | Out-Null
}

# ============================================================
# VCPKG INSTALL (solo si no esta hecho)
# ============================================================
if (-not (Test-Path "$VCPKG_INS\vcpkg\status")) {
    Write-Host ""
    Write-Host "  [PRE] Instalando dependencias vcpkg (10-30 min primera vez)..." -ForegroundColor Yellow
    Write-Host ""
    & "$VCPKG\vcpkg.exe" install `
        --triplet x64-mingw-dynamic `
        --x-manifest-root="$PSScriptRoot" `
        --x-install-root="$VCPKG_INS"
    if ($LASTEXITCODE -ne 0) {
        Write-Host "  [ERROR] vcpkg install fallo." -ForegroundColor Red
        Read-Host "  Enter para salir"; exit 1
    }
    Write-Host "  [OK] Dependencias instaladas." -ForegroundColor Green
} else {
    Write-Host "  [OK] Dependencias vcpkg ya instaladas." -ForegroundColor Green
}


# ============================================================
# CMAKE CONFIGURE
# ============================================================
if ($MODE -eq "1" -or $MODE -eq "4" -or $MODE -eq "5") {
    Write-Host ""
    Write-Host "  [1/3] Configurando CMake ($BUILD_TYPE)..." -ForegroundColor Cyan
    Write-Host ""

    # znver1 = Zen+ (Ryzen 3700U Picasso). NO usar znver2 en esta CPU (eso es Zen 2 desktop).
    & $CMAKE -B build `
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" `
        "-DCMAKE_PREFIX_PATH=$QT_ROOT" `
        "-DCMAKE_C_COMPILER=$MINGW/bin/gcc.exe" `
        "-DCMAKE_CXX_COMPILER=$MINGW/bin/g++.exe" `
        "-DCMAKE_MAKE_PROGRAM=$NINJA" `
        "-DCMAKE_TOOLCHAIN_FILE=$VCPKG/scripts/buildsystems/vcpkg.cmake" `
        -DVCPKG_TARGET_TRIPLET=x64-mingw-dynamic `
        -DVCPKG_MANIFEST_INSTALL=OFF `
        "-DVCPKG_INSTALLED_DIR=$VCPKG_INS" `
        "-DECM_DIR=$ECM" `
        "-DJAVA_HOME=$JAVA" `
        -DCMAKE_SUPPRESS_REGENERATION=ON `
        -DENABLE_LTO="$LTO" `
        "-DCMAKE_C_FLAGS_RELEASE=-O3 -march=znver1 -mtune=znver1 -ffunction-sections -fdata-sections -fno-asynchronous-unwind-tables" `
        "-DCMAKE_CXX_FLAGS_RELEASE=-O3 -march=znver1 -mtune=znver1 -ffunction-sections -fdata-sections -fno-asynchronous-unwind-tables" `
        "-DCMAKE_EXE_LINKER_FLAGS_RELEASE=-Wl,--gc-sections" `
        -G Ninja

    if ($LASTEXITCODE -ne 0) {
        Write-Host ""; Write-Host "  [ERROR] CMake configure fallo. Lee el error arriba." -ForegroundColor Red
        Read-Host "  Enter para salir"; exit 1
    }

    if ($MODE -eq "4") {
        Write-Host "  [OK] Configure completado." -ForegroundColor Green
        Read-Host "  Enter para salir"; exit 0
    }
}


# ============================================================
# COMPILAR
# ============================================================
$CORES = $env:NUMBER_OF_PROCESSORS
Write-Host ""
Write-Host "  [2/3] Compilando con $CORES nucleos..." -ForegroundColor Cyan
Write-Host ""

$t0 = Get-Date
& $CMAKE --build build --parallel $CORES
$elapsed = [int]((Get-Date) - $t0).TotalSeconds

if ($LASTEXITCODE -ne 0) {
    Write-Host ""; Write-Host "  [ERROR] Compilacion fallo." -ForegroundColor Red
    Read-Host "  Enter para salir"; exit 1
}

# ============================================================
# COPIAR DLLs Qt + MinGW
# ============================================================
Write-Host ""; Write-Host "  [3/3] Copiando DLLs..." -ForegroundColor Cyan

$EXE_DIR = $null; $EXE_NAME = $null
foreach ($name in @("beteliney.exe", "BetelineyLauncher.exe")) {
    foreach ($dir in @("build", "build\launcher")) {
        if (Test-Path "$dir\$name") { $EXE_DIR = $dir; $EXE_NAME = $name; break }
    }
    if ($EXE_DIR) { break }
}

if (-not $EXE_DIR) {
    Write-Host "  [WARN] Ejecutable no encontrado. Buscando..." -ForegroundColor Yellow
    Get-ChildItem -Recurse build -Filter "*.exe" | Where-Object { $_.Name -notmatch "test" }
    Read-Host "  Enter para salir"; exit 1
}

Push-Location $EXE_DIR
& "$QT_ROOT\bin\windeployqt.exe" --no-translations --no-opengl-sw --skip-plugin-types generic,networkinformation $EXE_NAME
Pop-Location

foreach ($dll in @("libgcc_s_seh-1.dll", "libstdc++-6.dll", "libwinpthread-1.dll")) {
    $src = "$MINGW\bin\$dll"
    if (Test-Path $src) {
        Copy-Item $src "$EXE_DIR\" -Force
        Write-Host "  [OK] $dll" -ForegroundColor Green
    }
}

$sizeMB = [int]((Get-Item "$EXE_DIR\$EXE_NAME").Length / 1MB)
Write-Host ""
Write-Host "  ==========================================" -ForegroundColor Green
Write-Host "   BUILD EXITOSO" -ForegroundColor Green
Write-Host "  ==========================================" -ForegroundColor Green
Write-Host "   Archivo : $EXE_DIR\$EXE_NAME"
Write-Host "   Tamano  : $sizeMB MB"
Write-Host "   Tiempo  : ${elapsed}s"
Write-Host "  ==========================================" -ForegroundColor Green
Write-Host ""
Start-Process explorer.exe $EXE_DIR
Read-Host "  Enter para cerrar"
