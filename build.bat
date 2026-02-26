@echo off
echo Looking for Visual Studio installation...

REM Try to find VS Developer Command Prompt (64-bit)
set "VSPATH=C:\Program Files\Microsoft Visual Studio\18\Insiders"
if exist "%VSPATH%\Common7\Tools\VsDevCmd.bat" (
    echo Found VS 18 Insiders
    call "%VSPATH%\Common7\Tools\VsDevCmd.bat" -arch=amd64
    goto :build
)

set "VSPATH=C:\Program Files\Microsoft Visual Studio\2022\Community"
if exist "%VSPATH%\Common7\Tools\VsDevCmd.bat" (
    echo Found VS 2022 Community
    call "%VSPATH%\Common7\Tools\VsDevCmd.bat" -arch=amd64
    goto :build
)

set "VSPATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community"
if exist "%VSPATH%\Common7\Tools\VsDevCmd.bat" (
    echo Found VS 2019 Community
    call "%VSPATH%\Common7\Tools\VsDevCmd.bat" -arch=amd64
    goto :build
)

echo ERROR: Could not find Visual Studio installation!
pause
exit /b 1

:build
echo.
echo Cleaning old build...
if exist build rmdir /s /q build

echo.
echo Configuring project (x64, Ninja)...
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 (
    echo Configuration failed!
    pause
    exit /b 1
)

echo.
echo Building project...
cmake --build build
if errorlevel 1 (
    echo Build failed!
    pause
    exit /b 1
)

echo.
echo ========================================
echo Build successful!
echo Executable: build\MicBooster_artefacts\Release\Mic Booster.exe
echo ========================================
pause
