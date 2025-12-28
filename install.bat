@echo off
:: ================================================
::  Profiler - compilation script (Windows)
::  Epitech Paris - EIP 2025-2026
:: ================================================

setlocal enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
set "BUILD_DIR=%SCRIPT_DIR%build"
set "CACHE_DIR=%SCRIPT_DIR%.cache"

if "%1"=="" goto default
if /I "%1"=="config" goto config
if /I "%1"=="build" goto build
if /I "%1"=="re" goto rebuild
if /I "%1"=="-h" goto usage
if /I "%1"=="--help" goto usage
goto unknown

:usage
echo Usage:
echo   install.bat           config + build (default)
echo   install.bat config    cmake config only
echo   install.bat build     cmake build only
echo   install.bat re        cache delete + remake
echo   install.bat -h        show this help
goto end

:config
echo [INFO] Configuring cmake
cmake -S "%SCRIPT_DIR%" -B "%BUILD_DIR%"
if errorlevel 1 (
    echo [ERROR] cmake configuration failed
    exit /b 1
)
echo [OK] cmake configured
goto end

:build
echo [INFO] Building project
cmake --build "%BUILD_DIR%"
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