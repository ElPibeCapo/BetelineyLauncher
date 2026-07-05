@echo off
title Beteliney Tools - Actualizar Drivers
chcp 65001 >nul 2>&1
echo.
echo =========================================
echo   Drivers desactualizados detectados
echo =========================================
echo.
echo Drivers con actualizaciones disponibles en tu HP ProBook 445R G6:
echo.
echo [1] Ethernet Realtek RTL8168 - driver del 2019 (muy viejo)
echo [2] WiFi Realtek RTL8822BE   - driver del 2021
echo [3] Bluetooth Realtek        - driver del 2021
echo [4] NVMe SK Hynix BC501      - usando driver generico Microsoft
echo [5] AMD Sensor Hub (SFH)     - driver del 2020
echo.
echo La forma mas segura es descargar todos desde HP Support:
echo (HP garantiza compatibilidad con tu modelo exacto)
echo.
echo Abriendo HP Support para ProBook 445R G6...
timeout /t 3 /nobreak >nul
start "" "https://support.hp.com/us-en/drivers/hp-probook-445r-g6-notebook-pc/2101561607"

echo.
echo Si prefieres ir directo a los fabricantes:
echo.
set /p OPT="Abrir paginas de drivers individuales? (s/n): "
if /i "%OPT%"=="s" (
    echo Abriendo AMD Chipset Software (AMD PSP + SFH)...
    start "" "https://www.amd.com/en/support/download/drivers.html"
    timeout /t 2 /nobreak >nul
    echo Abriendo AMD Adrenalin (GPU + chipset completo)...
    start "" "https://www.amd.com/en/support/apu/amd-ryzen-processors/amd-ryzen-7-mobile-processors-radeon-vega-graphics/amd-ryzen-7-3700u"
    timeout /t 2 /nobreak >nul
)
echo.
echo Nota: Actualiza primero con HP Support.
echo Si HP no tiene la version mas nueva, ahi si descarga directo del fabricante.
echo.
pause
