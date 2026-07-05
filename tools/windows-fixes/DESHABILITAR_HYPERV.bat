@echo off
title Beteliney Tools - Deshabilitar Hyper-V
chcp 65001 >nul 2>&1
echo.
echo =========================================
echo   Deshabilitar Hyper-V
echo =========================================
echo.
echo ADVERTENCIA IMPORTANTE:
echo Si usas alguna de estas apps, NO deshabilites Hyper-V:
echo   - Docker Desktop
echo   - WSL2 (subsistema Linux)
echo   - Maquinas virtuales de Hyper-V
echo.
echo Si solo usas VirtualBox o VMware, esas NO necesitan Hyper-V.
echo.
echo Por que importa: Hyper-V convierte Windows en una VM de su propio
echo hipervisor, añadiendo latencia a toda operacion de hardware.
echo HWiNFO reporto: "Advertencia de maquina virtual - Hyper-V activo"
echo.
set /p CONFIRM="Deshabilitar Hyper-V? (s/n): "
if /i not "%CONFIRM%"=="s" (
    echo Cancelado.
    pause
    exit /b 0
)

echo.
echo Verificando si Hyper-V esta activo...
dism /online /get-featureinfo /featurename:Microsoft-Hyper-V-All 2>nul | findstr "State : Enabled" >nul
if not errorlevel 1 (
    echo [INFO] Hyper-V esta activo. Deshabilitando...
    powershell -Command "Disable-WindowsOptionalFeature -Online -FeatureName Microsoft-Hyper-V-All -NoRestart" 2>nul
    if not errorlevel 1 (
        echo.
        echo [OK] Hyper-V deshabilitado. Requiere reiniciar para aplicar.
        set /p REBOOT="Reiniciar ahora? (s/n): "
        if /i "%REBOOT%"=="s" shutdown /r /t 10 /c "Reiniciando para aplicar cambios de Hyper-V"
    ) else (
        echo [ERROR] Fallo. Ejecuta como Administrador.
    )
) else (
    echo [OK] Hyper-V no esta activo o no esta instalado. Nada que hacer.
)
echo.
pause
