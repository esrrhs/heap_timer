#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
BUILD_TYPE="${1:-Release}"

case "${BUILD_TYPE}" in
    Release|release)
        BUILD_TYPE="Release"
        ;;
    Debug|debug)
        BUILD_TYPE="Debug"
        ;;
    *)
        echo "usage: $0 [Release|Debug]"
        exit 1
        ;;
esac

echo "==> Configuring heap_timer (${BUILD_TYPE})..."
cmake -B "${BUILD_DIR}" -S "${SCRIPT_DIR}" -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"

echo "==> Building heap_timer..."
cmake --build "${BUILD_DIR}" --config "${BUILD_TYPE}" -j"$(nproc 2>/dev/null || echo 2)"

echo "==> Running unit tests..."
ctest --test-dir "${BUILD_DIR}" --output-on-failure -C "${BUILD_TYPE}"

echo "==> Build successful"
echo "    header: ${SCRIPT_DIR}/heap_timer.h"
echo "    test:   ${BUILD_DIR}/bin/heap_timer_test"
echo "    optional benchmark: ${BUILD_DIR}/bin/heap_timer_test benchmark"
