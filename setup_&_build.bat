@echo off
:: ================================================
::  Profiler - Script d'installation & build (Windows)
::  Epitech Paris - EIP 2025-2026
:: ================================================

echo.
echo  ========================================
echo     Profiler - Setup & Build Automatique
echo  ========================================
echo.

:: --- 1. Vérifier Git ---
git --version >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERREUR] Git n'est pas installe !
    echo Installez Git : https://git-scm.com/download/win
    pause
    exit /b 1
)

:: --- 2. Cloner ou mettre à jour ---
if not exist ".git" (
    echo [INFO] Clonage du repo...
    git clone --recursive https://github.com/votre-org/Profiler.git .
    if %errorlevel% neq 0 (
        echo [ERREUR] Echec du clone.
        pause
        exit /b 1
    )
) else (
    echo [INFO] Repo deja present. Mise a jour...
    git pull
    git submodule update --init --recursive
)

:: --- 3. Télécharger JUCE si absent ---
if not exist "Libs\JUCE\" (
    echo [INFO] Telechargement de JUCE 7 (version stable)...
    powershell -Command "Invoke-WebRequest -Uri 'https://github.com/juce-framework/JUCE/archive/refs/tags/7.0.9.zip' -OutFile 'juce.zip'"
    powershell -Command "Expand-Archive -Force 'juce.zip' 'Libs'"
    ren "Libs\JUCE-7.0.9" "JUCE"
    del juce.zip
    echo [OK] JUCE telecharge dans Libs/JUCE
) else (
    echo [OK] JUCE deja present.
)

:: --- 4. Lancer le Projucer (GUI) ---
echo.
echo [INFO] Ouverture du Projucer...
start "" "Libs\JUCE\extras\Projucer\Builds\VisualStudio2022\Release\Projucer.exe" "Profiler.jucer"

echo.
echo [ATTENTION] Dans le Projucer :
echo   1. Verifiez que Visual Studio 2022 est selectionne
echo   2. Activez : Copy plugin after build
echo   3. Cliquez sur "Save Project and Open in IDE"
echo.
pause

:: --- 5. Attendre que l'utilisateur ferme VS, puis copier le plugin ---
:wait_vs
tasklist | find "devenv.exe" >nul
if %errorlevel% equ 0 (
    timeout /t 5 >nul
    goto wait_vs
)

:: --- 6. Copier le .vst3 dans le dossier VST3 systeme ---
set "VST3_DEST=%COMMONPROGRAMFILES%\VST3"
if not exist "%VST3_DEST%" set "VST3_DEST=%PROGRAMFILES%\Common Files\VST3"

if exist "Builds\VisualStudio2022\x64\Release\VST3\Profiler.vst3" (
    echo [OK] Copie du plugin vers %VST3_DEST%...
    xcopy /Y /E /I "Builds\VisualStudio2022\x64\Release\VST3\Profiler.vst3" "%VST3_DEST%\Profiler.vst3\"
    echo.
    echo [SUCCES] Plugin installe ! Ouvrez votre DAW et cherchez "Profiler"
) else (
    echo [ERREUR] Plugin non trouve. Avez-vous compile en Release ?
)

:: --- 7. Lancer le Standalone ---
if exist "Builds\VisualStudio2022\x64\Release\Standalone\Profiler.exe" (
    echo.
    echo [LANCEMENT] Demarrage du Standalone...
    start "" "Builds\VisualStudio2022\x64\Release\Standalone\Profiler.exe"
)

echo.
echo  Setup termine !
pause
