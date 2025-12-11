@echo off
:: ================================================
::  Profiler - installation/build script (Windows)
::  Epitech Paris - EIP 2025-2026
:: ================================================

echo.
echo  ========================================
echo    Profiler - Automatic Setup & Build
echo  ========================================
echo.

:: --- 1. Git check ---
git --version >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] Git isn't installed!
    echo Install Git here: https://git-scm.com/install
    pause
    exit /b 1
)

:: --- 2. Profiler lastest download ---
if not exist ".git" (
    echo [INFO] Cloning repo...
    git clone --recursive https://github.com/QuentinStoll/Profiler.git .
    if %errorlevel% neq 0 (
        echo [ERROR] cloning error
        pause
        exit /b 1
    )
) else (
    echo [INFO] Repo already downloaded. Updating...
    git pull
    git submodule update --init --recursive
)

:: --- 3. Download JUCE if missing ---
if not exist "Libs\JUCE\" (
    echo [INFO] Downloading JUCE 8 (stable version)...
    powershell -Command "Invoke-WebRequest -Uri 'https://github.com/juce-framework/JUCE/releases/download/8.0.11/juce-8.0.11-windows.zip' -OutFile 'juce.zip'"
    powershell -Command "Expand-Archive -Force 'juce.zip' 'Libs'"
    ren "Libs\JUCE-8.0.11" "JUCE"
    del juce.zip
    echo [OK] JUCE downloaded into Libs/JUCE
) else (
    echo [OK] JUCE already present.
)

:: --- 4. Projucer startup (GUI)  ---
echo.
echo [INFO] Building Projucer...
start "" "Libs\JUCE\extras\Projucer\Builds\VisualStudio2022\Release\Projucer.exe" "Profiler.jucer"
echo [OK] Projucer is done building

echo.
echo [WARNING] In Projucer:
echo   1. Check that Visual Studio 20XX is selected
echo   2. Enable: Copy plugin after build
echo   3. Click on "Save Project and Open in IDE"
echo.
pause
exit
