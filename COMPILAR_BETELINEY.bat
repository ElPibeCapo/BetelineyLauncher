@echo off
setlocal EnableDelayedExpansion
title Beteliney Launcher - Compilador v7
chcp 437 >nul 2>&1
cls

echo.
echo  ==========================================
echo    Beteliney Launcher  ^|  Build Script v7
echo  ==========================================
echo.

:: ========== Verificar que estemos en source\ ==========
if not exist "CMakeLists.txt" (
    echo  [ERROR] Ejecuta este bat desde dentro de la carpeta source\
    pause & exit /b 1
)

:: ========== Detectar si hay build previo para ofrecer modo incremental ==========
set HAS_CACHE=0
if exist "build\CMakeCache.txt" set HAS_CACHE=1

:: ========== Menu principal ==========
echo  Elige modo de compilacion:
echo.
echo    [1]  Build completo        - configura + compila  (primera vez)
if %HAS_CACHE%==1 (
    echo    [2]  Recompilar            - solo compila, usa cache  ^(rapido^)
    echo    [3]  Limpiar y rebuild     - borra build\ y empieza de cero
) else (
    echo    [2]  Recompilar            - no disponible, aun no hay build
    echo    [3]  Limpiar y rebuild     - equivalente a [1]
)
echo    [4]  Solo configurar       - cmake configure sin compilar
echo    [5]  Debug build           - build con simbolos de depuracion
echo    [0]  Salir
echo.
set /p MODE="  Opcion [1]: "
if "%MODE%"=="" set MODE=1
if "%MODE%"=="0" exit /b 0
if "%MODE%"=="2" if %HAS_CACHE%==0 set MODE=1

:: Validar opcion
if "%MODE%"=="1" goto :do_detect
if "%MODE%"=="2" goto :do_detect
if "%MODE%"=="3" goto :do_clean
if "%MODE%"=="4" goto :do_detect
if "%MODE%"=="5" goto :do_detect
echo  [ERROR] Opcion invalida.
pause & exit /b 1

:do_clean
echo.
echo  [INFO] Borrando carpeta build\...
if exist "build" rd /s /q "build"
echo  [OK] build\ eliminado. Iniciando build completo.
set MODE=1

:do_detect
echo.
echo  --- Detectando herramientas ---
echo.

:: ========== Detectar Git ==========
set GIT_EXE=
for %%P in (
    "C:\Program Files\Git\bin\git.exe"
    "C:\Program Files (x86)\Git\bin\git.exe"
) do if exist %%~P if not defined GIT_EXE set GIT_EXE=%%~P

if defined GIT_EXE (
    for %%G in ("%GIT_EXE%") do set "PATH=%%~dpG;!PATH!"
    echo  [OK] Git: %GIT_EXE%
) else (
    where git >nul 2>&1
    if errorlevel 1 (
        echo  [ERROR] Git no encontrado. https://git-scm.com/download/win
        pause & exit /b 1
    )
    echo  [OK] Git: en PATH
)

:: ========== Detectar Qt (6.8 a 6.10) ==========
:: Orden de búsqueda: C:\Qt (instalación por defecto del Qt Installer en Windows)
::                   D:\Qt\Qt, D:\Qt (instalaciones en segunda unidad)
::                   %USERPROFILE%\Qt (instalación por usuario)
::                   %LOCALAPPDATA%\Qt (instalación portable)
set QT_ROOT=
set QT_VER=
for %%V in (6.10.2 6.10.1 6.10.0 6.9.2 6.9.1 6.9.0 6.8.3 6.8.2 6.8.1 6.8.0) do (
    for %%B in ("C:\Qt\%%V" "D:\Qt\Qt\%%V" "D:\Qt\%%V" "%USERPROFILE%\Qt\%%V" "%LOCALAPPDATA%\Qt\%%V") do (
        if exist "%%~B\mingw_64\bin\qmake.exe" (
            if not defined QT_ROOT (
                set QT_ROOT=%%~B
                set QT_VER=%%V
            )
        )
    )
)
if not defined QT_ROOT (
    echo  [ERROR] Qt 6.8+ con MinGW no encontrado. Rutas buscadas:
    echo          D:\Qt\Qt\{version}\   C:\Qt\{version}\   D:\Qt\{version}\
    echo          Instala desde https://www.qt.io/download-qt-installer
    echo          Marcar: Qt 6.x -^> MinGW 64-bit + Qt Tools -^> Ninja + CMake
    pause & exit /b 1
)
echo  [OK] Qt %QT_VER%

:: ========== Detectar MinGW ==========
:: Busca mingw*_64 en las Tools del Qt encontrado (C:\Qt\Tools o D:\Qt\Qt\Tools)
set MINGW_ROOT=
:: Intentar en la carpeta Tools hermana del directorio Qt encontrado
for %%T in ("C:\Qt\Tools" "D:\Qt\Qt\Tools" "D:\Qt\Tools") do (
    if exist "%%~T" (
        for %%V in (mingw1310_64 mingw1120_64 mingw900_64) do (
            if exist "%%~T\%%V\bin\gcc.exe" (
                if not defined MINGW_ROOT set MINGW_ROOT=%%~T\%%V
            )
        )
    )
)
if not defined MINGW_ROOT (
    echo  [ERROR] MinGW no encontrado en C:\Qt\Tools ni D:\Qt\Qt\Tools\
    echo          Instala Qt con componente MinGW 64-bit desde el Qt Installer.
    pause &amp; exit /b 1
)

echo  [OK] MinGW: %MINGW_ROOT%

:: ========== Detectar Ninja ==========
set NINJA_EXE=
for %%T in ("C:\Qt\Tools\Ninja\ninja.exe" "D:\Qt\Qt\Tools\Ninja\ninja.exe" "D:\Qt\Tools\Ninja\ninja.exe") do (
    if exist %%~T if not defined NINJA_EXE set NINJA_EXE=%%~T
)
if not defined NINJA_EXE (
    where ninja >nul 2>&1 && set NINJA_EXE=ninja
)
if not defined NINJA_EXE (
    echo  [ERROR] Ninja no encontrado. Instala Qt Tools -^> Ninja en Qt Installer.
    pause & exit /b 1
)
echo  [OK] Ninja

:: ========== Detectar CMake ==========
:: Orden: sistema -> PATH -> Qt Tools. Se verifica ejecutabilidad.
:: cmake de Qt Tools puede estar bloqueado por AppLocker en equipos corporativos/SENA.
set CMAKE_EXE=

:: 1. cmake del sistema (C:\Program Files) - generalmente no esta bloqueado
if exist "C:\Program Files\CMake\bin\cmake.exe" (
    "C:\Program Files\CMake\bin\cmake.exe" --version >nul 2>&1
    if not errorlevel 1 set "CMAKE_EXE=C:\Program Files\CMake\bin\cmake.exe"
)

:: 2. cmake en PATH
if not defined CMAKE_EXE (
    where cmake >nul 2>&1
    if not errorlevel 1 (
        cmake --version >nul 2>&1
        if not errorlevel 1 set CMAKE_EXE=cmake
    )
)

:: 3. cmake de Qt Tools (puede estar bloqueado por AppLocker)
if not defined CMAKE_EXE if exist "D:\Qt\Qt\Tools\CMake_64\bin\cmake.exe" (
    "D:\Qt\Qt\Tools\CMake_64\bin\cmake.exe" --version >nul 2>&1
    if not errorlevel 1 set CMAKE_EXE=D:\Qt\Qt\Tools\CMake_64\bin\cmake.exe
)

if not defined CMAKE_EXE (
    echo  [ERROR] CMake no encontrado o bloqueado ^(AppLocker^).
    echo          Instala CMake desde https://cmake.org/download/ y agrega al PATH.
    pause & exit /b 1
)

echo  [OK] CMake: %CMAKE_EXE%

:: ========== Detectar Java (17 o 21) ==========
set JAVA_HOME=

:: Intentar JAVA_HOME del sistema primero
if defined JAVA_HOME (
    if exist "%JAVA_HOME%\bin\java.exe" goto :java_found
    set JAVA_HOME=
)

:: Buscar en rutas comunes (21 preferido, luego 17)
for %%J in ("C:\Program Files\Eclipse Adoptium\jdk-21.0.10.7-hotspot" "C:\Program Files\Eclipse Adoptium\jdk-21.0.7.6-hotspot" "C:\Program Files\Eclipse Adoptium\jdk-21.0.5.10-hotspot" "C:\Program Files\Eclipse Adoptium\jdk-21" "C:\Program Files\Microsoft\jdk-21.0.5.11-hotspot" "C:\Program Files\Java\jdk-21" "C:\Program Files\Eclipse Adoptium\jdk-17.0.13.11-hotspot" "C:\Program Files\Eclipse Adoptium\jdk-17" "C:\Program Files\Java\jdk-17") do (
    if exist %%~J\bin\java.exe if not defined JAVA_HOME set JAVA_HOME=%%~J
)

:: Ultimo recurso: buscar en registry (sin pipe, filtrar dentro del for)
if not defined JAVA_HOME (
    for /f "tokens=2*" %%A in ('reg query "HKLM\SOFTWARE\Eclipse Adoptium" /s /v "Path" 2^>nul') do (
        if /i "%%A"=="Path" if exist "%%B\bin\java.exe" if not defined JAVA_HOME set JAVA_HOME=%%B
    )
)

if not defined JAVA_HOME (
    echo  [ERROR] Java 17 o 21 no encontrado. https://adoptium.net
    pause & exit /b 1
)
:java_found
echo  [OK] Java: %JAVA_HOME%

:: ========== Detectar vcpkg ==========
set VCPKG_ROOT=
:: Primero buscar vcpkg_local incluido en el ZIP del proyecto
if exist "%~dp0..\..\vcpkg_local\vcpkg.exe" (
    pushd "%~dp0..\..\vcpkg_local"
    set "VCPKG_ROOT=!CD!"
    popd
)
if not defined VCPKG_ROOT if exist "C:\vcpkg\vcpkg.exe" set VCPKG_ROOT=C:\vcpkg
if not defined VCPKG_ROOT if exist "%USERPROFILE%\vcpkg\vcpkg.exe" set "VCPKG_ROOT=%USERPROFILE%\vcpkg"
if not defined VCPKG_ROOT if exist "%LOCALAPPDATA%\vcpkg\vcpkg.exe" set "VCPKG_ROOT=%LOCALAPPDATA%\vcpkg"
if not defined VCPKG_ROOT (
    echo  [ERROR] vcpkg no encontrado.
    echo          git clone https://github.com/microsoft/vcpkg C:\vcpkg
    echo          C:\vcpkg\bootstrap-vcpkg.bat
    pause & exit /b 1
)
echo  [OK] vcpkg: %VCPKG_ROOT%

:: ========== PATH temprano (MinGW necesario para vcpkg) ==========
set PATH=%MINGW_ROOT%\bin;D:\Qt\Qt\Tools\Ninja;%JAVA_HOME%\bin;%PATH%

:: ========== libnbtplusplus ==========
if not exist "libraries\libnbtplusplus\CMakeLists.txt" (
    echo.
    echo  [INFO] Descargando libnbtplusplus...
    git clone https://github.com/PrismaticallyMulticoloured/libnbtplusplus.git "libraries\libnbtplusplus"
    if errorlevel 1 (
        echo  [ERROR] No se pudo descargar libnbtplusplus. Revisa tu conexion.
        pause & exit /b 1
    )
    echo  [OK] libnbtplusplus descargado
)

:: ========== .git minimo ==========
if not exist ".git\HEAD" (
    if not exist ".git" mkdir ".git"
    if not exist ".git\refs\heads" mkdir ".git\refs\heads"
    echo ref: refs/heads/master>".git\HEAD"
    echo 0000000000000000000000000000000000000000>".git\refs\heads\master"
    type nul > ".git\packed-refs"
)
if not exist ".git\packed-refs" type nul > ".git\packed-refs"

:: ========== vcpkg install ==========
set BUILD_DIR=%CD%\build
set VCPKG_INSTALLED=%BUILD_DIR%\vcpkg_installed

if exist "%VCPKG_INSTALLED%\vcpkg\status" goto :vcpkg_done
echo.
echo  [PRE] Instalando dependencias vcpkg (primera vez, puede tardar 10-30 min)...
echo.
"%VCPKG_ROOT%\vcpkg.exe" install --triplet x64-mingw-dynamic --x-manifest-root="%CD%" --x-install-root="%VCPKG_INSTALLED%"
if errorlevel 1 (
    echo.
    echo  [ERROR] vcpkg install fallo.
    echo  Revisa que vcpkg este actualizado: cd C:\vcpkg ^&^& git pull ^&^& bootstrap-vcpkg.bat
    pause & exit /b 1
)
echo  [OK] Dependencias instaladas
:vcpkg_done

:: ========== pkgconf ==========
set PKGCONF_EXE=
if exist "%VCPKG_INSTALLED%\x64-mingw-dynamic\tools\pkgconf\pkgconf.exe" (
    set PKGCONF_EXE=%VCPKG_INSTALLED%\x64-mingw-dynamic\tools\pkgconf\pkgconf.exe
)
if not defined PKGCONF_EXE if exist "%VCPKG_INSTALLED%\x64-windows\tools\pkgconf\pkgconf.exe" (
    set PKGCONF_EXE=%VCPKG_INSTALLED%\x64-windows\tools\pkgconf\pkgconf.exe
)

:: ========== ECM ==========
set ECM_DIR=
for %%P in ("%VCPKG_ROOT%\packages\ecm_x64-windows\share\ECM" "%VCPKG_INSTALLED%\x64-windows\share\ECM" "%VCPKG_INSTALLED%\x64-mingw-dynamic\share\ECM") do (
    if exist "%%~P\ECMConfig.cmake" if not defined ECM_DIR set ECM_DIR=%%~P
)
if not defined ECM_DIR (
    echo  [ERROR] ECM no encontrado. Ejecuta: vcpkg install ecm:x64-windows
    pause & exit /b 1
)

:: ========== Nucleos de CPU ==========
set /a CPU_CORES=%NUMBER_OF_PROCESSORS%
if %CPU_CORES% LSS 1 set CPU_CORES=4

:: ========== Resumen ==========
echo.
echo  ==========================================
echo   Herramientas listas
echo  ==========================================
echo   Qt      : %QT_VER%  (%QT_ROOT%\mingw_64)
echo   MinGW   : %MINGW_ROOT%
echo   Java    : %JAVA_HOME%
echo   vcpkg   : %VCPKG_INSTALLED%
echo   ECM     : %ECM_DIR%
echo   Nucleos : %CPU_CORES%
echo  ==========================================
echo.

:: ========== Determinar BUILD_TYPE ==========
set BUILD_TYPE=Release
if "%MODE%"=="5" set BUILD_TYPE=RelWithDebInfo

:: ========== Solo configurar (modo 4) ==========
if "%MODE%"=="4" goto :do_configure

:: ========== Recompilar incremental (modo 2) ==========
if "%MODE%"=="2" goto :do_build

:do_configure
set PATH=%MINGW_ROOT%\bin;D:\Qt\Qt\Tools\Ninja;%JAVA_HOME%\bin;%PATH%

echo  [1/3] Configurando CMake (%BUILD_TYPE%)...
echo.

set LTO_FLAG=ON
if "%BUILD_TYPE%"=="RelWithDebInfo" set LTO_FLAG=OFF

:: Flags CPU: znver1 = Zen/Zen+ (Ryzen 3700U Picasso). NO usar znver2 en esta CPU (eso es Zen 2 desktop).
"%CMAKE_EXE%" -B build --no-warn-unused-cli ^
  -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
  "-DCMAKE_PREFIX_PATH=%QT_ROOT%/mingw_64" ^
  "-DCMAKE_C_COMPILER=%MINGW_ROOT%/bin/gcc.exe" ^
  "-DCMAKE_CXX_COMPILER=%MINGW_ROOT%/bin/g++.exe" ^
  "-DCMAKE_MAKE_PROGRAM=%NINJA_EXE%" ^
  -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake" ^
  -DVCPKG_TARGET_TRIPLET=x64-mingw-dynamic ^
  -DVCPKG_MANIFEST_INSTALL=OFF ^
  "-DVCPKG_INSTALLED_DIR=%VCPKG_INSTALLED%" ^
  "-DECM_DIR=%ECM_DIR%" ^
  "-DPKG_CONFIG_EXECUTABLE=%PKGCONF_EXE%" ^
  "-DJAVA_HOME=%JAVA_HOME%" ^
  -DCMAKE_SUPPRESS_REGENERATION=ON ^
  -DENABLE_LTO=%LTO_FLAG% ^
  "-DCMAKE_C_FLAGS_RELEASE=-O3 -march=znver1 -mtune=znver1 -ffunction-sections -fdata-sections" ^
  "-DCMAKE_CXX_FLAGS_RELEASE=-O3 -march=znver1 -mtune=znver1 -ffunction-sections -fdata-sections" ^
  "-DCMAKE_EXE_LINKER_FLAGS_RELEASE=-Wl,--gc-sections" ^
  -G Ninja

if errorlevel 1 (
    echo.
    echo  [ERROR] CMake configure fallo.
    echo  Causas comunes:
    echo    - Qt o MinGW mal instalado
    echo    - vcpkg desactualizado  ^(cd C:\vcpkg ^&^& git pull^)
    echo    - ECM no instalado     ^(vcpkg install ecm:x64-windows^)
    pause & exit /b 1
)

if "%MODE%"=="4" (
    echo.
    echo  [OK] Configure completado. Para compilar corre el bat de nuevo.
    pause & exit /b 0
)

:do_build
set PATH=%MINGW_ROOT%\bin;D:\Qt\Qt\Tools\Ninja;%JAVA_HOME%\bin;%PATH%
echo.
echo  [2/3] Compilando con %CPU_CORES% nucleos...
echo.

:: Guardar timestamp de inicio
set START_TIME=%TIME%

"%CMAKE_EXE%" --build build --parallel %CPU_CORES%

if errorlevel 1 (
    echo.
    echo  [ERROR] Compilacion fallo.
    echo  Consejo: corre modo [3] Limpiar y rebuild si el error es extrano.
    pause & exit /b 1
)

:: Calcular tiempo
set END_TIME=%TIME%
call :calc_elapsed "%START_TIME%" "%END_TIME%"

echo.
echo  [3/3] Copiando DLLs de Qt...

:: Buscar el ejecutable en varias ubicaciones posibles
set EXE_DIR=
set EXE_NAME=
for %%N in (beteliney.exe BetelineyLauncher.exe) do (
    for %%D in (build build\launcher) do (
        if exist "%%D\%%N" if not defined EXE_DIR (
            set EXE_DIR=%%D
            set EXE_NAME=%%N
        )
    )
)

if not defined EXE_DIR (
    echo  [WARN] Ejecutable no encontrado en las rutas habituales.
    echo  Buscando en build\...
    dir /S /B build\*.exe 2>nul | findstr /V /I test
    echo.
    pause & exit /b 1
)

:: windeployqt: desplegar DLLs Qt
pushd "%EXE_DIR%"
"%QT_ROOT%\mingw_64\bin\windeployqt.exe" ^
    --no-translations ^
    --no-opengl-sw ^
    --skip-plugin-types generic,networkinformation ^
    "%EXE_NAME%"
popd

:: Copiar DLLs de MinGW (libgcc, libstdc++, libwinpthread)
echo  [INFO] Copiando DLLs de runtime MinGW...
for %%D in (libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll) do (
    if exist "%MINGW_ROOT%\bin\%%D" (
        copy /y "%MINGW_ROOT%\bin\%%D" "%EXE_DIR%\" >nul
        echo  [OK] %%D
    )
)

:: Copiar DLLs de vcpkg (bin\ y lib\ - MinGW pone algunas como libqrencode en lib\)
echo  [INFO] Copiando DLLs de vcpkg...
for %%D in ("%VCPKG_INSTALLED%\x64-mingw-dynamic\bin\*.dll") do (
    copy /y "%%D" "%EXE_DIR%\" >nul
    echo  [OK] %%~nxD
)
for %%D in ("%VCPKG_INSTALLED%\x64-mingw-dynamic\lib\*.dll") do (
    copy /y "%%D" "%EXE_DIR%\" >nul
    echo  [OK] %%~nxD
)

:: Mostrar tamaño del ejecutable
set EXE_SIZE=
for %%F in ("%EXE_DIR%\%EXE_NAME%") do set EXE_SIZE=%%~zF
set /a EXE_SIZE_MB=%EXE_SIZE% / 1048576

echo.
echo  ==========================================
echo   BUILD EXITOSO
echo  ==========================================
echo   Archivo : %EXE_DIR%\%EXE_NAME%
echo   Tamano  : %EXE_SIZE_MB% MB
echo   Tiempo  : %ELAPSED_STR%
echo  ==========================================
echo.
echo  Presiona cualquier tecla para abrir la carpeta...
pause >nul
explorer "%EXE_DIR%"
exit /b 0

:: ========== Funcion: calcular tiempo transcurrido ==========
:calc_elapsed
set "T1=%~1"
set "T2=%~2"
for /f "tokens=1-4 delims=:,." %%a in ("%T1: =0%") do (
    set /a "h1=1%%a-100, m1=1%%b-100, s1=1%%c-100, cs1=1%%d-100"
)
for /f "tokens=1-4 delims=:,." %%a in ("%T2: =0%") do (
    set /a "h2=1%%a-100, m2=1%%b-100, s2=1%%c-100, cs2=1%%d-100"
)
set /a "total1=((h1*60+m1)*60+s1)*100+cs1"
set /a "total2=((h2*60+m2)*60+s2)*100+cs2"
set /a "diff=total2-total1"
if %diff% LSS 0 set /a "diff=diff+8640000"
set /a "es=diff/100, ecs=diff%%100"
set /a "em=es/60, es=es%%60"
set /a "eh=em/60, em=em%%60"
if %eh% GTR 0 (
    set ELAPSED_STR=%eh%h %em%m %es%s
) else if %em% GTR 0 (
    set ELAPSED_STR=%em%m %es%s
) else (
    set ELAPSED_STR=%es%.%ecs%s
)
goto :eof
