@echo off
setlocal

echo ========================================
echo  SilkroadBot - Dependency Setup Script
echo ========================================

set "ROOT=%~dp0"
set "THIRD_PARTY=%ROOT%third_party"

set "JSON_DIR=%THIRD_PARTY%\json"
set "INTERCEPTION_DIR=%THIRD_PARTY%\Interception"

set "OPENCV_VERSION=4.10.0"
set "OPENCV_SRC_DIR=%THIRD_PARTY%\opencv"
set "OPENCV_CONTRIB_DIR=%THIRD_PARTY%\opencv_contrib"
set "OPENCV_BUILD_DIR=%THIRD_PARTY%\opencv_build"
set "OPENCV_INSTALL_DIR=%THIRD_PARTY%\opencv_install"

set "CUDA_ARCHITECTURES="

:: ============================================================
:: Git verification
:: ============================================================
where git >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Git is not installed or not in PATH.
    echo         Download from: https://git-scm.com/download/win
    pause
    exit /b 1
)

:: ============================================================
:: CMake verification
:: ============================================================
where cmake >nul 2>&1
if errorlevel 1 (
    echo [ERROR] CMake is not installed or not in PATH.
    echo         Download from: https://cmake.org/download
    pause
    exit /b 1
)

:: ============================================================
:: CUDA Toolkit verification
:: ============================================================
where nvcc >nul 2>&1
if errorlevel 1 (
    echo [ERROR] nvcc is not in PATH.
    echo         Install CUDA Toolkit and open a new terminal.
    pause
    exit /b 1
)

:: ============================================================
:: zlibwapi.dll (required by cuDNN)
:: ============================================================
echo.
echo Checking zlibwapi.dll for cuDNN...

set "ZLIB_URL=https://www.winimage.com/zLibDll/zlib123dllx64.zip"
set "ZLIB_ZIP=%THIRD_PARTY%\zlib.zip"
set "ZLIB_DIR=%THIRD_PARTY%\zlib"

:: Ensure directories exist
mkdir "%THIRD_PARTY%" 2>nul
mkdir "%ZLIB_DIR%"    2>nul

:: Detect CUDA bin directory from nvcc path
for /f "delims=" %%P in ('where nvcc') do set "CUDA_BIN=%%~dpP"
if "%CUDA_BIN:~-1%"=="\" set "CUDA_BIN=%CUDA_BIN:~0,-1%"

if not exist "%CUDA_BIN%\zlibwapi.dll" (
    echo      zlibwapi.dll not found, downloading...

    powershell -NoProfile -ExecutionPolicy Bypass -Command ^
        "Invoke-WebRequest -Uri '%ZLIB_URL%' -OutFile '%ZLIB_ZIP%'"
    if errorlevel 1 (
        echo [ERROR] Failed to download zlib.
        pause
        exit /b 1
    )

    powershell -NoProfile -ExecutionPolicy Bypass -Command ^
        "Expand-Archive -Path '%ZLIB_ZIP%' -DestinationPath '%ZLIB_DIR%' -Force"
    if errorlevel 1 (
        echo [ERROR] Failed to extract zlib.
        pause
        exit /b 1
    )

    :: Copy to CUDA bin (requires elevation)
    powershell -NoProfile -ExecutionPolicy Bypass -Command ^
        "Start-Process -Verb RunAs -Wait -FilePath 'cmd.exe' -ArgumentList '/c copy ""%ZLIB_DIR%\dll_x64\zlibwapi.dll"" ""%CUDA_BIN%\zlibwapi.dll""'"
    if errorlevel 1 (
        echo [ERROR] Failed to copy zlibwapi.dll to %CUDA_BIN%.
        pause
        exit /b 1
    )

    del "%ZLIB_ZIP%" >nul 2>&1
    echo      [OK] zlibwapi.dll installed to %CUDA_BIN%
) else (
    echo      [OK] zlibwapi.dll already present, skipping.
)

mkdir "%THIRD_PARTY%" 2>nul

:: ============================================================
:: 1. nlohmann/json
:: ============================================================
echo.
echo [1/4] Installing nlohmann/json...
if not exist "%JSON_DIR%\include\nlohmann\json.hpp" (
    echo      Cloning nlohmann/json v3.11.3...
    git clone --depth=1 --branch v3.11.3 https://github.com/nlohmann/json.git "%JSON_DIR%"
    if errorlevel 1 (
        echo [ERROR] Failed to clone json.
        pause
        exit /b 1
    )
) else (
    echo      nlohmann/json already present, skipping.
)

:: ============================================================
:: 2. Interception
:: ============================================================
echo.
echo [2/4] Installing Interception...
set "INTERCEPTION_ZIP=%THIRD_PARTY%\Interception.zip"

if not exist "%INTERCEPTION_DIR%\library\x64\interception.lib" (
    echo      Download Interception release...

    powershell -NoProfile -ExecutionPolicy Bypass -Command ^
        "Invoke-WebRequest -Uri 'https://github.com/oblitum/Interception/releases/download/v1.0.1/Interception.zip' -OutFile '%INTERCEPTION_ZIP%'"
    if errorlevel 1 (
        echo [ERROR] Failed to download Interception.
        pause
        exit /b 1
    )

    echo      Extracting...
    powershell -NoProfile -ExecutionPolicy Bypass -Command ^
        "Expand-Archive -Path '%INTERCEPTION_ZIP%' -DestinationPath '%THIRD_PARTY%' -Force"
    if errorlevel 1 (
        echo [ERROR] Failed to extract Interception.
        pause
        exit /b 1
    )

    del "%INTERCEPTION_ZIP%" >nul 2>&1

    echo.
    echo [WARNING] Install the Interception driver as Administrator:
    echo    "%INTERCEPTION_DIR%\command line installer\install-interception.exe" /install
    echo    Then restart your computer.
    echo.
) else (
    echo      Interception already present, skipping.
)

:: ============================================================
:: 3. OpenCV + opencv_contrib
:: ============================================================
echo.
echo [3/4] Installing OpenCV 4.10.0 + opencv_contrib...

if not exist "%OPENCV_SRC_DIR%\.git" (
    echo      Cloning opencv...
    git clone --depth=1 --branch %OPENCV_VERSION% https://github.com/opencv/opencv.git "%OPENCV_SRC_DIR%"
    if errorlevel 1 (
        echo [ERROR] Failed to clone opencv.
        pause
        exit /b 1
    )
) else (
    echo      opencv already present, skipping.
)

if not exist "%OPENCV_CONTRIB_DIR%\.git" (
    echo      Cloning opencv_contrib...
    git clone --depth=1 --branch %OPENCV_VERSION% https://github.com/opencv/opencv_contrib.git "%OPENCV_CONTRIB_DIR%"
    if errorlevel 1 (
        echo [ERROR] Failed to clone opencv_contrib.
        pause
        exit /b 1
    )
) else (
    echo      opencv_contrib already present, skipping.
)

if not exist "%OPENCV_BUILD_DIR%" mkdir "%OPENCV_BUILD_DIR%"
if not exist "%OPENCV_INSTALL_DIR%" mkdir "%OPENCV_INSTALL_DIR%"

set "CUDA_ARCH_ARG="
if defined CUDA_ARCHITECTURES (
    set "CUDA_ARCH_ARG=-DCMAKE_CUDA_ARCHITECTURES=%CUDA_ARCHITECTURES%"
)

echo CUDA_ARCH_ARG: %CUDA_ARCH_ARG%
echo.
echo      Configuring OpenCV...
cmake -S "%OPENCV_SRC_DIR%" -B "%OPENCV_BUILD_DIR%" ^
  -G "Visual Studio 17 2022" -A x64 ^
  -DCMAKE_INSTALL_PREFIX="%OPENCV_INSTALL_DIR%" ^
  -DOPENCV_EXTRA_MODULES_PATH="%OPENCV_CONTRIB_DIR%\modules" ^
  -DBUILD_LIST=core,imgproc,highgui,imgcodecs,video,videoio,dnn,features2d,calib3d,flann,cudev,cudaarithm,cudaimgproc,cudafilters,cudalegacy ^
  -DWITH_CUDA=ON ^
  -DCUDA_TOOLKIT_ROOT_DIR="%CUDA_ROOT%" ^
  -DCUDA_FAST_MATH=ON ^
  -DBUILD_SHARED_LIBS=ON ^
  -DBUILD_TESTS=OFF ^
  -DBUILD_PERF_TESTS=OFF ^
  -DBUILD_EXAMPLES=OFF ^
  -DBUILD_DOCS=OFF ^
  -DBUILD_opencv_apps=OFF ^
  -DBUILD_opencv_python2=OFF ^
  -DBUILD_opencv_python3=OFF ^
  -DBUILD_JAVA=OFF ^
  -DBUILD_opencv_world=OFF ^
  -DWITH_CUDNN=ON ^
  -DWITH_OPENCL=OFF ^
  -DWITH_QT=OFF ^
  -DWITH_OPENGL=OFF ^
  -DWITH_FFMPEG=ON ^
  -DWITH_IPP=ON ^
  -DWITH_TBB=ON ^
  -DWITH_EIGEN=ON


if errorlevel 1 (
    echo [ERROR] OpenCV configuration failed.
    pause
    exit /b 1
)

echo.
echo      Building Release + INSTALL...
cmake --build "%OPENCV_BUILD_DIR%" --config Release --target INSTALL
if errorlevel 1 (
    echo [ERROR] Build/OpenCV INSTALL failed.
    pause
    exit /b 1
)

echo.
echo ========================================
echo  Setup complete!
echo ========================================
echo.
echo  Remaining manual steps:
echo   1. [ADMIN] Install Interception driver + reboot:
echo      "%INTERCEPTION_DIR%\command line installer\install-interception.exe" /install
echo.
echo   2. Use OpenCV from:
echo      %OPENCV_INSTALL_DIR%
echo.
echo   3. In the project, load:
echo      cmake -C third_party/paths.cmake -B build -G "Visual Studio 17 2022" -A x64
echo      cmake --build build --config Release
echo.
pause