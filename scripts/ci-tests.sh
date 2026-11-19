#!/usr/bin/env bash
# Reproducible build/unit/smoke gates for Wyrm, run from the repository root.
# Usage: scripts/ci-tests.sh [BUILD_DIR]
# All build output is placed in BUILD_DIR (a fresh scratch dir by default).
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"

BUILD_DIR="${1:-$(mktemp -d)}"
mkdir -p "$BUILD_DIR"
BUILD_DIR="$(cd "$BUILD_DIR" && pwd)"
OBJ_DIR="$BUILD_DIR/obj"
mkdir -p "$OBJ_DIR"

CXX="${CXX:-g++}"
CC="${CC:-gcc}"
CXXFLAGS=(-std=c++20 -Wall -Wextra -Werror -O2)
CFLAGS=(-std=c11 -Wall -Wextra -Werror -O2 -DWYRM_NO_CURL -Iwyrm/lib)

step() {
    printf '\n===== %s =====\n' "$1"
}

# ---------------------------------------------------------------------------
# Lexer unit tests (self-contained).
# ---------------------------------------------------------------------------
step "lexer unit tests"
"$CXX" "${CXXFLAGS[@]}" \
    compiler/lexer/tests/test_lexer.cpp \
    compiler/lexer/lexer.cpp \
    -o "$BUILD_DIR/test_lexer"
"$BUILD_DIR/test_lexer"

# ---------------------------------------------------------------------------
# Parser unit tests (self-contained).
# ---------------------------------------------------------------------------
step "parser unit tests"
"$CXX" "${CXXFLAGS[@]}" \
    compiler/parser/tests/test_parser.cpp \
    compiler/parser/parser.cpp \
    compiler/lexer/lexer.cpp \
    -o "$BUILD_DIR/test_parser"
"$BUILD_DIR/test_parser"

# ---------------------------------------------------------------------------
# C runtime objects, compiled once and shared by the interpreter, transpiler
# and wyrmc Stage 0 builds. WYRM_NO_CURL keeps the build free of libcurl deps.
# ---------------------------------------------------------------------------
step "C runtime objects"
RUNTIME_OBJS=()
for name in wyrm_core wyrm_arena wyrm_str wyrm_ffi; do
    obj="$OBJ_DIR/$name.o"
    "$CC" "${CFLAGS[@]}" -c "wyrm/lib/$name.c" -o "$obj"
    RUNTIME_OBJS+=("$obj")
done
for name in wyrm_std_json wyrm_std_yaml wyrm_std_collections wyrm_std_random \
            wyrm_std_time wyrm_std_os wyrm_std_fs wyrm_std_path wyrm_std_io \
            wyrm_std_http; do
    obj="$OBJ_DIR/$name.o"
    "$CC" "${CFLAGS[@]}" -c "wyrm/lib/stdlib/$name.c" -o "$obj"
    RUNTIME_OBJS+=("$obj")
done

# ---------------------------------------------------------------------------
# Interpreter unit tests.
# ---------------------------------------------------------------------------
step "interpreter unit tests"
"$CXX" "${CXXFLAGS[@]}" -DWYRM_NO_CURL \
    compiler/interpreter/tests/test_interpreter.cpp \
    compiler/interpreter/interpreter.cpp \
    compiler/interpreter/builtins.cpp \
    compiler/interpreter/stdlib_setup.cpp \
    compiler/parser/parser.cpp \
    compiler/lexer/lexer.cpp \
    "${RUNTIME_OBJS[@]}" \
    -o "$BUILD_DIR/test_interpreter"
"$BUILD_DIR/test_interpreter"

# ---------------------------------------------------------------------------
# Transpiler unit tests. The test emits C, compiles it with gcc and compares
# native output against the interpreter, so it validates both code paths.
# test_llvm_short_circuit is intentionally excluded: it needs an LLVM IR
# fixture emitted by the self-hosted backend, which this workflow does not
# build.
# ---------------------------------------------------------------------------
step "transpiler unit tests"
"$CXX" "${CXXFLAGS[@]}" \
    compiler/transpiler/tests/test_transpiler.cpp \
    compiler/transpiler/transpiler.cpp \
    compiler/lexer/lexer.cpp \
    compiler/parser/parser.cpp \
    compiler/interpreter/interpreter.cpp \
    compiler/interpreter/builtins.cpp \
    compiler/interpreter/stdlib_setup.cpp \
    "${RUNTIME_OBJS[@]}" \
    -o "$BUILD_DIR/test_transpiler"
"$BUILD_DIR/test_transpiler" "$BUILD_DIR/transpiler_scratch" "$REPO_ROOT"

# ---------------------------------------------------------------------------
# wyrpkg build + package-manager test suite.
# ---------------------------------------------------------------------------
step "wyrpkg tests"
"$CXX" -std=c++20 -Wall -Wextra -Werror -O1 \
    wyrm/src/wyrpkg.cpp \
    -o "$BUILD_DIR/wyrpkg"
bash wyrm/src/tests/run_tests.sh "$BUILD_DIR"

# ---------------------------------------------------------------------------
# wyrmc Stage 0 build + CLI regression tests.
# ---------------------------------------------------------------------------
step "wyrmc Stage 0 CLI tests"
"$CXX" "${CXXFLAGS[@]}" -DWYRM_NO_CURL \
    wyrm/src/wyrmc.cpp \
    compiler/lexer/lexer.cpp \
    compiler/parser/parser.cpp \
    compiler/interpreter/interpreter.cpp \
    compiler/interpreter/builtins.cpp \
    compiler/interpreter/stdlib_setup.cpp \
    compiler/transpiler/transpiler.cpp \
    "${RUNTIME_OBJS[@]}" \
    -o "$BUILD_DIR/wyrmc_stage0"
WYRMC_BIN="$BUILD_DIR/wyrmc_stage0" \
    bash tests/wyrmc/test_cli_formatting.sh "$BUILD_DIR"

# ---------------------------------------------------------------------------
# Smoke: run a tiny program through the freshly built compiler.
# ---------------------------------------------------------------------------
step "smoke: wyrmc_stage0 run"
SMOKE_DIR="$BUILD_DIR/smoke"
mkdir -p "$SMOKE_DIR"
cat > "$SMOKE_DIR/smoke.wyr" <<'WYR'
fn main() {
    print("wyrm-smoke-ok")
}
WYR
actual="$(cd "$SMOKE_DIR" && "$BUILD_DIR/wyrmc_stage0" run smoke.wyr)"
if [ "$actual" != "wyrm-smoke-ok" ]; then
    printf 'smoke output mismatch: expected "wyrm-smoke-ok", got "%s"\n' "$actual" >&2
    exit 1
fi

printf '\nAll CI gates passed.\n'
