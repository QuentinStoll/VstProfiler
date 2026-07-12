@echo off
:: ================================================
::  Profiler - compilation script (Windows)
::  Epitech Paris - EIP 2025-2026
:: ================================================

setlocal enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
set "BUILD_DIR=%SCRIPT_DIR%build"
set "CACHE_DIR=%SCRIPT_DIR%.cache"
set BUILD_PRESET="default"

if not exist %BUILD_DIR% mkdir %BUILD_DIR%
if not exist %CACHE_DIR% mkdir %CACHE_DIR%

if "%2"=="" set BUILD_PRESET="default"
if /I "%2"=="default" set BUILD_PRESET="default"
if /I "%2"=="release" set BUILD_PRESET="release"
if /I "%2"=="all-formats" set BUILD_PRESET="all-formats"
if /I "%2"=="dev" set BUILD_PRESET="dev"
if /I "%2"=="debug" set BUILD_PRESET="debug"
if /I "%2"=="-h" goto usage
if /I "%2"=="--help" goto usage

if "%1"=="" goto default
if /I "%1"=="all" goto default
if /I "%1"=="config" goto config
if /I "%1"=="build" goto build
if /I "%1"=="re" goto rebuild
if /I "%1"=="-h" goto usage
if /I "%1"=="--help" goto usage
goto unknown

:usage
echo Usage: install.bat ACTION [PRESET]
echo ACTIONS
echo   all				config + build
echo   config			cmake config only
echo   build			cmake build only
echo   re				cache delete + remake
echo PRESETS
echo   default			recommended (for dev or use)
echo   release			with standard release features
echo   all-formats		builds plugin in all availlable formats
echo   dev				most debug features
echo   debug			all debug features + performance profiling
goto end

:config
echo [INFO] Configuring cmake
cmake -S "%~dp0." -B "%~dp0build" -DPRESET_NAME=%BUILD_PRESET%
if errorlevel 1 (
    echo [ERROR] cmake configuration failed
    exit /b 1
)
echo [OK] cmake configured
goto end

:build
echo [INFO] Building project
cmake --build "%BUILD_DIR%" -j8
if errorlevel 1 (
    echo [ERROR] Build failed
    exit /b 1
)
echo [OK] Done building project
goto end

:rebuild
echo [INFO] Cleaning build and cache directories
if exist "%BUILD_DIR%" (
    rmdir /s /q "%BUILD_DIR%"
    echo [OK] Removed build directory
)
if exist "%CACHE_DIR%" (
    rmdir /s /q "%CACHE_DIR%"
    echo [OK] Removed cache directory
)
call :config
if errorlevel 1 exit /b 1
call :build
if errorlevel 1 exit /b 1
goto end

:default
call :config
if errorlevel 1 exit /b 1
call :build
if errorlevel 1 exit /b 1
goto end

:unknown
echo [ERROR] Unknown option: %1
echo.
call :usage
exit /b 1

:end
endlocal
exit /b 0