#!/usr/bin/env bash
# ================================================
#  Profiler - compilation script (Linux)
#  Epitech Paris - EIP 2025-2026
# ================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
CACHE_DIR="$SCRIPT_DIR/.cache"

set -e
mkdir -p $BUILD_DIR
mkdir -p $CACHE_DIR


usage() {
	echo "Usage:"
	echo "  ./install.sh all       config + build (default)"
	echo "  ./install.sh config    cmake config only"
	echo "  ./install.sh build     cmake build only"
	echo "  ./install.sh test      run automated tests"
	echo "  ./install.sh re    	   cache delete + remake"
}

config() {
	echo "[INFO] Configuring cmake"
	cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S $SCRIPT_DIR -B $BUILD_DIR
	echo "[OK] cmake configured"
}

build() {
	echo "[INFO] Building project"
	cmake --build $BUILD_DIR
	echo "[OK] Done building project"
}

test_project() {
	if [ ! -f "$BUILD_DIR/CMakeCache.txt" ]; then
		config
	fi
	echo "[INFO] Building automated tests"
	cmake --build "$BUILD_DIR" --target ProfilerTests
	echo "[INFO] Running automated tests"
	ctest --test-dir "$BUILD_DIR" --output-on-failure
	echo "[INFO] Test summary"
	TEST_EXE="$BUILD_DIR/Tests/ProfilerTests"
	if [ ! -x "$TEST_EXE" ] && [ -x "$BUILD_DIR/Tests/Release/ProfilerTests" ]; then
		TEST_EXE="$BUILD_DIR/Tests/Release/ProfilerTests"
	fi
	if [ ! -x "$TEST_EXE" ] && [ -x "$BUILD_DIR/Tests/Release/ProfilerTests.exe" ]; then
		TEST_EXE="$BUILD_DIR/Tests/Release/ProfilerTests.exe"
	fi
	"$TEST_EXE" --quiet
	echo "[OK] Tests passed"
}

case "$1" in
	"" | "all")
		config
		build
		;;
	config )
		config
		;;
	build )
		build
		;;
	test )
		test_project
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
