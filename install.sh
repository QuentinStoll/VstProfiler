#!/usr/bin/env bash
# ================================================
#  Profiler - compilation script (Linux)
#  Epitech Paris - EIP 2025-2026
# ================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
CACHE_DIR="$SCRIPT_DIR/.cache"

set -e

usage() {
	echo "Usage:"
	echo "  ./install.sh           config + build (default)"
	echo "  ./install.sh config    cmake config only"
	echo "  ./install.sh build     cmake build only"
	echo "  ./install.sh re    	   cache delete + remake"
}

config() {
	echo "[INFO] Configuring cmake"
	cmake -S $SCRIPT_DIR -B $BUILD_DIR
	echo "[OK] cmake configured"
}

build() {
	echo "[INFO] Building project"
	cmake --build $BUILD_DIR
	echo "[OK] Done building project"
}

case "$1" in
	"" )
		config
		build
		;;
	config )
		config
		;;
	build )
		build
		;;
	re )
		rm $BUILD_DIR
		rm $CACHE_DIR
		config
		build
		;;
	-h|--help )
		usage
		;;
	* )
		echo "[ERROR] Unknown option: $1"
		usage
		exit 1
		;;
esac