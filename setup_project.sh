# ================================================
#  Profiler - installation/build script (Linux)
#  Epitech Paris - EIP 2025-2026
# ================================================

echo  "\n========================================"
echo  "   Profiler - Automatic Setup & Build"
echo  "========================================\n"

# --- 1. Git check ---
if ! git --version >/dev/null 2>&1; then
    echo "[ERROR] Git isn't installed!"
    echo "Install Git here: https://git-scm.com/install"
    exit 1
fi

# --- 2. Profiler lastest download ---
if [ ! -d ".git" ]; then
    echo "[INFO] Cloning repo..."
    if ! git clone --recursive https://github.com/QuentinStoll/Profiler.git .; then
        echo "[ERROR] cloning error"
        exit 1
    fi
else
    echo "[INFO] Repo already downloaded. Updating..."
    git pull
    git submodule update --init --recursive
fi

# --- 3. Download JUCE if missing ---
if [ ! -d "Libs/JUCE" ]; then
    echo "[INFO] Downloading JUCE 8 (stable version)..."
    curl -o juce.zip -L "https://github.com/juce-framework/JUCE/releases/download/8.0.11/juce-8.0.11-linux.zip"
    unzip -q juce.zip -d Libs
    mv "Libs/JUCE-8.0.11" "Libs/JUCE"
    rm juce.zip
    echo "[OK] JUCE downloaded into Libs/JUCE"
else
    echo "[OK] JUCE already present."
fi

# --- 4. Projucer startup (GUI) ---
echo "[INFO] Building Projucer..."
if ! cd "Libs/JUCE/extras/Projucer/Builds/LinuxMakefile" || ! make; then
    echo "[ERROR] error while building Projucer."
    exit 1
fi
echo "[OK] Projucer is done building."
./Libs/JUCE/extras/Projucer/Builds/LinuxMakefile/build/Projucer "./Profiler.jucer"
exit 0
