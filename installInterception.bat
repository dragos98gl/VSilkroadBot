@echo off
setlocal

set "ROOT=%~dp0"
set "INTERCEPTION_DIR=%ROOT%third_party\Interception"

echo.
echo Installing Interception driver...
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
    "Start-Process -FilePath '%INTERCEPTION_DIR%\command line installer\install-interception.exe' -ArgumentList '/install' -Verb RunAs -Wait"
if errorlevel 1 (
    echo [ERROR] Interception driver installation failed or was rejected.
    pause
    exit /b 1
)
echo      [OK] Interception driver installed. Restarting computer is required for changes to take effect.
pause