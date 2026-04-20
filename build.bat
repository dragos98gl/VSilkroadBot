set QTFRAMEWORK_BYPASS_LICENSE_CHECK=1
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
cmake -S . -B build
cmake --build build --config Release
C:\Qt\6.10.1\msvc2022_64\bin\windeployqt.exe build/Release/VSilkroadBot.exe
build\Release\VSilkroadBot.exe
