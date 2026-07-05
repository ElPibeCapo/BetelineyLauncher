@echo off
title Beteliney Tools - Deshabilitar Fast Startup
chcp 65001 >nul 2>&1
echo.
echo =========================================
echo   Deshabilitar Fast Startup (Windows)
echo =========================================
echo.
echo Por que: Fast Startup hace apagados parciales (hibernacion).
echo El SSD reporto 92 apagados inseguros - esto los causa.
echo Deshabilitarlo hace apagados completos y reduce desgaste del SSD.
echo.
echo Verificando estado actual...

reg query "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Power" /v HiberbootEnabled 2>nul | findstr "0x1" >nul
if not errorlevel 1 (
    echo [INFO] Fast Startup esta ACTIVADO. Deshabilitando...
    reg add "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Power" /v HiberbootEnabled /t REG_DWORD /d 0 /f >nul
    if not errorlevel 1 (
        echo [OK] Fast Startup deshabilitado correctamente.
    ) else (
        echo [ERROR] No se pudo modificar el registro. Ejecuta como Administrador.
    )
) else (
    echo [OK] Fast Startup ya estaba deshabilitado. Nada que hacer.
)

echo.
echo Tambien puedes verificar en:
echo Panel de Control ^> Opciones de energia ^> 
echo "Elegir comportamiento botones inicio/apagado" ^>
echo Desactivar "Activar inicio rapido"
echo.
pause
