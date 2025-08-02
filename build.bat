@echo off
setlocal enabledelayedexpansion

:: Plugin System Build Script
echo ===================================
echo    Plugin System Build Script
echo ===================================

:: Set default values
set BUILD_DIR=build
set BUILD_TYPE=Debug
set BUILD_SHARED=ON
set BUILD_EXAMPLES=ON
set BUILD_TESTS=ON
set GENERATOR="Visual Studio 17 2022"
set ARCH=x64
set FORCE_CONFIG=OFF

:: Parse command line arguments
:parse_args
if "%~1"=="" goto :done_parsing
if /i "%~1"=="--help" goto :show_help
if /i "%~1"=="-h" goto :show_help
if /i "%~1"=="--force" (
    set FORCE_CONFIG=ON
    shift
    goto :parse_args
)
if /i "%~1"=="-f" (
    set FORCE_CONFIG=ON
    shift
    goto :parse_args
)

if /i "%~1"=="--build-dir" (
    set BUILD_DIR=%~2
    shift
    shift
    goto :parse_args
)

if /i "%~1"=="--build-type" (
    set BUILD_TYPE=%~2
    shift
    shift
    goto :parse_args
)

if /i "%~1"=="--shared" (
    set BUILD_SHARED=%~2
    shift
    shift
    goto :parse_args
)

if /i "%~1"=="--examples" (
    set BUILD_EXAMPLES=%~2
    shift
    shift
    goto :parse_args
)

if /i "%~1"=="--tests" (
    set BUILD_TESTS=%~2
    shift
    shift
    goto :parse_args
)

if /i "%~1"=="--generator" (
    set GENERATOR=%~2
    shift
    shift
    goto :parse_args
)

if /i "%~1"=="--arch" (
    set ARCH=%~2
    shift
    shift
    goto :parse_args
)

echo Unknown parameter: %~1
shift
goto :parse_args

:done_parsing

:: Display configuration information
echo.
echo Build configuration:
echo   Build directory: %BUILD_DIR%
echo   Build type: %BUILD_TYPE%
echo   Shared libraries: %BUILD_SHARED%
echo   Build examples: %BUILD_EXAMPLES%
echo   Build tests: %BUILD_TESTS%
echo   Generator: %GENERATOR%
echo   Architecture: %ARCH%
echo.

:: Check if CMake is installed
where cmake >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo Error: CMake not found. Please install CMake and ensure it's in your PATH.
    exit /b 1
)

:: Create build directory
if not exist %BUILD_DIR% (
    echo Creating build directory: %BUILD_DIR%
    mkdir %BUILD_DIR%
    set FORCE_CONFIG=ON
)

:: Configure project
if /i "%FORCE_CONFIG%"=="ON" (
    echo Configuring project...
    cmake -S . -B %BUILD_DIR% -G %GENERATOR% -A %ARCH% ^
        -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
        -DBUILD_SHARED_LIBS=%BUILD_SHARED% ^
        -DBUILD_EXAMPLES=%BUILD_EXAMPLES% ^
        -DBUILD_TESTS=%BUILD_TESTS%

    if %ERRORLEVEL% neq 0 (
        echo Error: CMake configuration failed.
        exit /b 1
    )
) else (
    echo Build directory already exists, skipping configuration step.
    echo Use --force or -f to force reconfiguration.
)

:: Build project
echo Building project...
cmake --build %BUILD_DIR% --config %BUILD_TYPE%

if %ERRORLEVEL% neq 0 (
    echo Error: Build failed.
    exit /b 1
)

echo.
echo Build successful!
echo Executables are located at: %BUILD_DIR%\bin\%BUILD_TYPE%

goto :eof

:show_help
echo Usage: build.bat [options]
echo.
echo Options:
echo   --help, -h          Display this help information
echo   --build-dir DIR     Set build directory (default: build)
echo   --build-type TYPE   Set build type (default: Debug)
echo                       Options: Debug, Release, RelWithDebInfo, MinSizeRel
echo   --shared ON/OFF     Whether to build shared libraries (default: ON)
echo   --examples ON/OFF   Whether to build examples (default: ON)
echo   --tests ON/OFF      Whether to build tests (default: OFF)
echo   --generator GEN     Set CMake generator (default: "Visual Studio 17 2022")
echo   --arch ARCH         Set target architecture (default: x64)
echo   --force, -f         Force CMake reconfiguration even if build directory exists
echo.
echo Examples:
echo   build.bat --build-type Release --shared OFF
echo   build.bat --build-dir build-vs2019 --generator "Visual Studio 16 2019"
echo   build.bat --force
echo.

exit /b 0