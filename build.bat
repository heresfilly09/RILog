@echo off
setlocal

echo [*] Setting up build environment...

:: Check if CMake is installed
where cmake >nul 2>nul
if %errorlevel% neq 0 (
    echo [!] CMake is not installed or not in PATH.
    echo [!] Please install CMake from https://cmake.org/download/
    exit /b 1
)

:: Create build directory if it doesn't exist
if not exist build (
    mkdir build
)

cd build

echo [*] Generating build files...
cmake ..
if %errorlevel% neq 0 (
    echo [!] CMake generation failed.
    exit /b 1
)

echo [*] Compiling project...
cmake --build . --config Release
if %errorlevel% neq 0 (
    echo [!] Build failed.
    exit /b 1
)

echo [*] Build successful! 
echo [*] Executable is located in the build\Release\ directory.
pause
