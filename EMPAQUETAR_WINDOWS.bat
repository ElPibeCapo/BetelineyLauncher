@echo off
setlocal EnableDelayedExpansion
title Beteliney Launcher - Empaquetador Windows
chcp 437 >nul 2>&1
cls

echo.
echo  ==========================================
echo    Beteliney Launcher  ^|  Empaquetar v7
echo  ==========================================
echo.
echo  Elige formato:
echo.
echo    [1]  ZIP portable    (sin instalador, copia y listo)
echo    [2]  Installer .exe  (requiere NSIS instalado)
echo    [0]  Salir
echo.
set /p PKGMODE="  Opcion [1]: "
if "%PKGMODE%"=="" set PKGMODE=1
if "%PKGMODE%"=="0" exit /b 0

:: ---- Verificar que haya build ----
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
    echo  [ERROR] No se encontro el ejecutable. Compila primero con COMPILAR_BETELINEY.bat
    pause & exit /b 1
)
echo  [OK] Ejecutable: %EXE_DIR%\%EXE_NAME%

:: ---- Fecha para nombre de archivo ----
for /f "tokens=1-3 delims=/-" %%a in ("%DATE%") do (
    set DAY=%%a & set MON=%%b & set YEAR=%%c
)
set FECHA=%YEAR%%MON%%DAY%

set OUTPUT_DIR=dist
if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"


if "%PKGMODE%"=="1" goto :make_zip
if "%PKGMODE%"=="2" goto :make_installer
echo  [ERROR] Opcion invalida. & pause & exit /b 1

:make_zip
echo.
echo  [INFO] Creando ZIP portable...

set ZIP_NAME=BetelineyLauncher-%FECHA%-Windows-x64.zip
set STAGE_DIR=%OUTPUT_DIR%\BetelineyLauncher

:: Limpiar y recrear staging
if exist "%STAGE_DIR%" rd /s /q "%STAGE_DIR%"
mkdir "%STAGE_DIR%"

:: Copiar todos los archivos del build (exe + dlls + plugins Qt)
xcopy /e /i /y "%EXE_DIR%\*" "%STAGE_DIR%\" >nul
echo  [OK] Archivos copiados

:: Copiar portable.txt si existe (Prism/Beteliney modo portatil)
if exist "program_info\portable.txt" (
    copy /y "program_info\portable.txt" "%STAGE_DIR%\" >nul
    echo  [OK] portable.txt incluido
)

:: Crear ZIP con PowerShell (disponible en Windows 10+)
powershell -Command "Compress-Archive -Path '%STAGE_DIR%\*' -DestinationPath '%OUTPUT_DIR%\%ZIP_NAME%' -Force" >nul 2>&1

if exist "%OUTPUT_DIR%\%ZIP_NAME%" (
    for %%F in ("%OUTPUT_DIR%\%ZIP_NAME%") do set ZIP_SIZE=%%~zF
    set /a ZIP_MB=!ZIP_SIZE! / 1048576
    echo.
    echo  ==========================================
    echo   ZIP CREADO
    echo  ==========================================
    echo   Archivo : %OUTPUT_DIR%\%ZIP_NAME%
    echo   Tamano  : !ZIP_MB! MB
    echo   Uso     : Extraer en cualquier carpeta y ejecutar %EXE_NAME%
    echo  ==========================================
) else (
    echo  [ERROR] ZIP no se creo.
)
rd /s /q "%STAGE_DIR%" >nul 2>&1
echo.
pause & exit /b 0


:make_installer
echo.
echo  [INFO] Verificando NSIS...

:: Buscar NSIS
set NSIS_EXE=
for %%P in (
    "C:\Program Files (x86)\NSIS\makensis.exe"
    "C:\Program Files\NSIS\makensis.exe"
) do if exist %%~P if not defined NSIS_EXE set NSIS_EXE=%%~P

if not defined NSIS_EXE (
    echo  [ERROR] NSIS no encontrado.
    echo.
    echo  Instala NSIS desde: https://nsis.sourceforge.io/Download
    echo  O usa la opcion [1] ZIP portable que no requiere nada extra.
    echo.
    pause & exit /b 1
)
echo  [OK] NSIS: %NSIS_EXE%

:: Verificar que exista el template NSI
set NSI_TEMPLATE=program_info\win_install.nsi.in
set NSI_FILE=%OUTPUT_DIR%\beteliney_install.nsi

if not exist "%NSI_TEMPLATE%" (
    echo  [ERROR] No se encontro %NSI_TEMPLATE%
    pause & exit /b 1
)

:: Reemplazar variables en el template NSI
:: Copiar template y hacer sustituciones basicas
copy /y "%NSI_TEMPLATE%" "%NSI_FILE%" >nul

:: Copiar archivos al staging para que NSIS los encuentre
set STAGE_DIR=%OUTPUT_DIR%\installer_stage
if exist "%STAGE_DIR%" rd /s /q "%STAGE_DIR%"
mkdir "%STAGE_DIR%"
xcopy /e /i /y "%EXE_DIR%\*" "%STAGE_DIR%\" >nul
if exist "program_info\beteliney.ico" copy /y "program_info\beteliney.ico" "%STAGE_DIR%\" >nul

echo  [INFO] Compilando installer con NSIS...
"%NSIS_EXE%" /DINSTALL_DIR="%STAGE_DIR%" /DOUT_FILE="%OUTPUT_DIR%\BetelineyLauncher-%FECHA%-Setup.exe" "%NSI_FILE%"

if errorlevel 1 (
    echo  [WARN] NSIS reporto errores. Verifica el template NSI.
    echo  Alternativa: usa opcion [1] ZIP portable.
) else (
    echo.
    echo  ==========================================
    echo   INSTALLER CREADO
    echo  ==========================================
    echo   Archivo : %OUTPUT_DIR%\BetelineyLauncher-%FECHA%-Setup.exe
    echo   Uso     : Ejecutar como administrador en la PC del amigo
    echo  ==========================================
)
rd /s /q "%STAGE_DIR%" >nul 2>&1
echo.
pause & exit /b 0
