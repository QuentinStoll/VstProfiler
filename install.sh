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
	echo "  ./install.sh test --coverage"
	echo "                         run automated tests and show coverage"
	echo "  ./install.sh re    	   cache delete + remake"
}

config() {
	echo "[INFO] Configuring cmake"
	cmake -DPROFILER_ENABLE_COVERAGE=OFF -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S $SCRIPT_DIR -B $BUILD_DIR
	echo "[OK] cmake configured"
}

coverage_is_enabled() {
	[ -f "$BUILD_DIR/CMakeCache.txt" ] && grep -q "PROFILER_ENABLE_COVERAGE:BOOL=ON" "$BUILD_DIR/CMakeCache.txt"
}

ensure_normal_config() {
	if [ ! -f "$BUILD_DIR/CMakeCache.txt" ] || coverage_is_enabled; then
		config
	fi
}

ensure_coverage_config() {
	if [ ! -f "$BUILD_DIR/CMakeCache.txt" ] || ! coverage_is_enabled; then
		echo "[INFO] Configuring coverage"
		cmake -DPROFILER_BUILD_TESTS=ON -DPROFILER_ENABLE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S "$SCRIPT_DIR" -B "$BUILD_DIR"
	fi
}

build() {
	ensure_normal_config
	echo "[INFO] Building project"
	cmake --build $BUILD_DIR
	echo "[OK] Done building project"
}

test_project() {
	ensure_normal_config
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

test_project_with_coverage() {
	ensure_coverage_config
	echo "[INFO] Running automated tests with coverage"
	cmake --build "$BUILD_DIR" --target ProfilerCoverage
	echo "[OK] Coverage report: $BUILD_DIR/coverage/html/index.html"
}

case "$1" in
	"" | "all")
		build
		;;
	config )
		config
		;;
	build )
		build
		;;
	test )
		if [ "$2" = "--coverage" ]; then
			test_project_with_coverage
		else
			test_project
		fi
		;;
	re )
		rm -fr $BUILD_DIR
		rm -fr $CACHE_DIR
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
