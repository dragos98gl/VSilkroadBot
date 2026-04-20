@echo off

python --version >nul 2>&1
if errorlevel 1 (
    echo Python not found!
    pause
    exit /b
)

for /f "tokens=2 delims= " %%v in ('python --version') do set PYVER=%%v

echo Python version: %PYVER%

echo %PYVER% | findstr /b "3.10" >nul
if errorlevel 1 (
    echo Python 3.10 is required!
    pause
    exit /b
)

python -m venv yolotrain
call yolotrain\Scripts\activate

python -m pip install --upgrade pip
pip install -r yolotrain_requirements.txt --no-deps
pause