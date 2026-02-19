#!/usr/bin/env bash
# ================================================
#  Profiler - compilation script (Linux)
#  Epitech Paris - EIP 2025-2026
# ================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
CACHE_DIR="$SCRIPT_DIR/.cache"
BUILD_PRESET="default"

set -e

usage() {
	echo "Usage: install.sh ACTION [PRESET]"
	echo "ACTIONS"
	echo "  all				config + build"
	echo "  config			cmake config only"
	echo "  build			cmake build only"
	echo "  re				cache delete + remake"
	echo "PRESETS"
	echo "  default			recommended (for dev or use)"
	echo "  release			with standard release features"
	echo "  all-formats		builds plugin in all availlable formats"
	echo "  dev				all debug features"
}

presets() {
	case "$2" in
	"" )
		BUILD_PRESET="default"
		;;
	default )
		BUILD_PRESET="default"
		;;
	release )
		BUILD_PRESET="release"
		;;
	all-formats )
		BUILD_PRESET="all-formats"
		;;
	dev )
		BUILD_PRESET="dev"
		;;
	-h|--help )
		usage
		exit 0
		;;
	* )
		echo "[ERROR] Unknown option: $2"
		usage
		exit 1
		;;
	esac
}

config() {
	echo "[INFO] Configuring cmake"
	cmake -S $SCRIPT_DIR -B $BUILD_DIR -DPRESET_NAME=$BUILD_PRESET
	echo "[OK] cmake configured"
}

build() {
	echo "[INFO] Building project"
	cmake --build $BUILD_DIR
	echo "[OK] Done building project"
}

case "$1" in
	all )
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
		rm -fr $BUILD_DIR
		rm -fr $CACHE_DIR
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