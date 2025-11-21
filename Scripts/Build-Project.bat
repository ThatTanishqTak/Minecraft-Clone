@echo off
REM Build the project with Visual Studio 2022 using MSVC and C++20.
REM The script asks for the desired configuration (Debug/Release) and builds accordingly.

setlocal enableextensions enabledelayedexpansion

REM Navigate to repository root from Scripts folder
pushd "%~dp0.."

set "BuildDirectory=build"
set "DefaultConfiguration=Debug"
set "Generator=Visual Studio 17 2022"
set "Architecture=x64"

REM Prompt user for configuration
set /p SelectedConfiguration="Choose build configuration [Debug/Release] (Default: !DefaultConfiguration!): "
if "!SelectedConfiguration!"=="" set "SelectedConfiguration=!DefaultConfiguration!"

REM Normalize input to standard casing
if /I "!SelectedConfiguration!"=="debug" set "SelectedConfiguration=Debug"
if /I "!SelectedConfiguration!"=="release" set "SelectedConfiguration=Release"

REM Validate configuration choice
if /I not "!SelectedConfiguration!"=="Debug" if /I not "!SelectedConfiguration!"=="Release" (
    echo Invalid configuration "!SelectedConfiguration!". Using default: !DefaultConfiguration!
    set "SelectedConfiguration=!DefaultConfiguration!"
)

REM Create build directory if needed
if not exist "!BuildDirectory!" mkdir "!BuildDirectory!"

REM Configure the project with C++20 enforced
cmake -S . -B "!BuildDirectory!" -G "!Generator!" -A "!Architecture!" -DCMAKE_CXX_STANDARD=20 -DCMAKE_CXX_STANDARD_REQUIRED=ON
if errorlevel 1 (
    echo CMake configuration failed.
    popd
    exit /b 1
)

REM Build with the chosen configuration
cmake --build "!BuildDirectory!" --config "!SelectedConfiguration!"
if errorlevel 1 (
    echo Build failed.
    popd
    exit /b 1
)

REM Return to original directory
popd

echo Build completed with configuration: !SelectedConfiguration!
pause