@echo off
setlocal

set "ROOT=%~dp0"
set "INTERCEPTION_DIR=%ROOT%third_party\Interception"

:: ============================================================
:: 1. Dependencies
:: ============================================================
echo.
echo [1/5] Running deps.bat...
call "%ROOT%deps.bat"
if errorlevel 1 (
    echo [ERROR] deps.bat failed.
    pause
    exit /b 1
)

:: ============================================================
:: 2. Interception driver (requires UAC elevation)
:: ============================================================
echo.
echo [2/5] Installing Interception driver...
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
    "Start-Process -FilePath '%INTERCEPTION_DIR%\command line installer\install-interception.exe' -ArgumentList '/install' -Verb RunAs -Wait"
if errorlevel 1 (
    echo [ERROR] Interception driver installation failed or was rejected.
    pause
    exit /b 1
)
echo      [OK] Interception driver installed. Restarting computer is required for changes to take effect.

:: ============================================================
:: 3. Python environments
:: ============================================================
echo.
echo [3/5] Setting up Python environments...
cd /d "%ROOT%python_venvs"
if errorlevel 1 (
    echo [ERROR] Directory python_venvs not found.
    pause
    exit /b 1
)

call install_digitsNN.bat
if errorlevel 1 (
    echo [ERROR] install_digitsNN.bat failed.
    pause
    exit /b 1
)

call install_yolotrain.bat
if errorlevel 1 (
    echo [ERROR] install_yolotrain.bat failed.
    pause
    exit /b 1
)

:: ============================================================
:: 4. Train digitsNN
:: ============================================================
echo.
echo [4/5] Running digitsNN training...
cd /d "%ROOT%assets\digitsNN_train"
if errorlevel 1 (
    echo [ERROR] Directory assets/digitsNN_train not found.
    pause
    exit /b 1
)

call train.bat
if errorlevel 1 (
    echo [ERROR] train.bat failed.
    pause
    exit /b 1
)

:: ============================================================
:: 5. Build
:: ============================================================
echo.
echo [5/5] Running build...
cd /d "%ROOT%"
call build.bat
if errorlevel 1 (
    echo [ERROR] build.bat failed.
    pause
    exit /b 1
)

:: ============================================================
:: Done
:: ============================================================
echo.
echo ========================================
echo  All steps completed successfully!
echo ========================================
echo.
pause