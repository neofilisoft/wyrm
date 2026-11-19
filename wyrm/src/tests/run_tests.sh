#!/usr/bin/env bash
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_SOURCE="${HERE}/test_wyrpkg.cpp"
BUILD_DIR="${1:-$(mktemp -d)}"
mkdir -p "${BUILD_DIR}"
BINARY="${BUILD_DIR}/test_wyrpkg"

echo "Compiling wyrpkg tests into ${BUILD_DIR} ..."
g++ -std=c++20 -Wall -Wextra -Werror -O1 "${TEST_SOURCE}" -o "${BINARY}"

echo "Running wyrpkg tests ..."
"${BINARY}"
