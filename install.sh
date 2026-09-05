#!/usr/bin/env bash
# Wyrm Language Global Installer for Linux & macOS
# Installs native Wyrm tools into the user directory layout:
#   $HOME/.wyrm/wyrmc/wyrmc
#   $HOME/.wyrm/wyrpkg/wyrpkg

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VERSION_FILE="$SCRIPT_DIR/VERSION"

if [ -f "$VERSION_FILE" ]; then
    WYRM_VERSION="$(cat "$VERSION_FILE" | tr -d '[:space:]')"
else
    WYRM_VERSION="3.0.0"
fi

INSTALL_ROOT="$HOME/.wyrm"
WYRMC_DIR="$INSTALL_ROOT/wyrmc"
WYRPKG_DIR="$INSTALL_ROOT/wyrpkg"
PACKAGES_DIR="$INSTALL_ROOT/packages/wyrmlang"

# Colors for terminal output
RED='\033[0;31m'
GREEN='\033[0;32m'
CYAN='\033[0;36m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}Initializing Wyrm global installation (v${WYRM_VERSION})...${NC}"

# Check required compilers
if ! command -v gcc &> /dev/null && ! command -v clang &> /dev/null; then
    echo -e "${RED}Error: Neither gcc nor clang was found in PATH.${NC}"
    echo "Please install build tools (e.g. build-essential on Ubuntu/Debian, Xcode Command Line Tools on macOS)."
    exit 1
fi

if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
    echo -e "${RED}Error: Neither g++ nor clang++ was found in PATH.${NC}"
    exit 1
fi

CC="gcc"
if ! command -v gcc &> /dev/null; then
    CC="clang"
fi

CXX="g++"
if ! command -v g++ &> /dev/null; then
    CXX="clang++"
fi

echo -e "Using C compiler:   ${CYAN}${CC}${NC}"
echo -e "Using C++ compiler: ${CYAN}${CXX}${NC}"

# 1. Create destination directories
mkdir -p "$INSTALL_ROOT" "$WYRMC_DIR" "$WYRPKG_DIR" "$PACKAGES_DIR"

# 2. Copy Wyrm runtime files to packages directory
echo "Copying Wyrm runtime standard library files..."
cp -rf "$SCRIPT_DIR/wyrm/"* "$PACKAGES_DIR/"

cd "$SCRIPT_DIR"

# 3. Compile C runtime objects
echo "Compiling C runtime objects..."
$CC -std=c11 -O2 -c wyrm/lib/wyrm_core.c -o wyrm_core.o -Iwyrm/lib
$CC -std=c11 -O2 -c wyrm/lib/wyrm_arena.c -o wyrm_arena.o -Iwyrm/lib
$CC -std=c11 -O2 -c wyrm/lib/wyrm_str.c -o wyrm_str.o -Iwyrm/lib
$CC -std=c11 -O2 -c wyrm/lib/wyrm_ffi.c -o wyrm_ffi.o -Iwyrm/lib
$CC -std=c11 -O2 -c wyrm/lib/stdlib/wyrm_std_json.c -o wyrm_std_json.o -Iwyrm/lib
$CC -std=c11 -O2 -c wyrm/lib/stdlib/wyrm_std_yaml.c -o wyrm_std_yaml.o -Iwyrm/lib
$CC -std=c11 -O2 -c wyrm/lib/stdlib/wyrm_std_sdl.c -o wyrm_std_sdl.o -Iwyrm/lib
$CC -std=c11 -O2 -c wyrm/lib/stdlib/wyrm_std_collections.c -o wyrm_std_collections.o -Iwyrm/lib

# 4. Compile temporary C++ bootstrap compiler
echo "Compiling bootstrap compiler..."
$CXX wyrm/scr/wyrmc.cpp \
    compiler/lexer/lexer.cpp \
    compiler/parser/parser.cpp \
    compiler/interpreter/interpreter.cpp \
    compiler/interpreter/builtins.cpp \
    compiler/interpreter/stdlib_setup.cpp \
    compiler/transpiler/transpiler.cpp \
    wyrm_core.o wyrm_arena.o wyrm_str.o \
    wyrm_ffi.o wyrm_std_json.o wyrm_std_yaml.o \
    wyrm_std_sdl.o wyrm_std_collections.o \
    -o wyrmc_bootstrap -std=c++20 -O2

# 5. Build self-hosted compiler stage
echo -e "${CYAN}Self-hosting Stage 1: Building self-hosted compiler using bootstrap compiler...${NC}"
BUILT_WYRMC="wyrmc_bootstrap"
if ./wyrmc_bootstrap build compiler/wyrmc.wyr 2>/dev/null; then
    if [ -f "compiler/wyrmc" ] || [ -f "compiler/wyrmc.exe" ]; then
        STAGE1="compiler/wyrmc"
        [ -f "compiler/wyrmc.exe" ] && STAGE1="compiler/wyrmc.exe"
        cp -f "$STAGE1" ./wyrmc_stage1
        rm -f "$STAGE1"

        echo -e "${CYAN}Self-hosting Stage 2: Rebuilding self-hosted compiler using Stage 1 compiler...${NC}"
        if ./wyrmc_stage1 build compiler/wyrmc.wyr 2>/dev/null; then
            if [ -f "compiler/wyrmc" ] || [ -f "compiler/wyrmc.exe" ]; then
                STAGE2="compiler/wyrmc"
                [ -f "compiler/wyrmc.exe" ] && STAGE2="compiler/wyrmc.exe"
                cp -f "$STAGE2" ./wyrmc
                BUILT_WYRMC="wyrmc"
            fi
        fi
    fi
fi

if [ "$BUILT_WYRMC" = "wyrmc_bootstrap" ]; then
    cp -f wyrmc_bootstrap ./wyrmc
fi

# 6. Compile wyrpkg
echo "Compiling package manager (wyrpkg)..."
$CXX wyrm/scr/wyrpkg.cpp -o wyrpkg -std=c++20 -O2

# Clean up intermediate build artifacts
rm -f *.o wyrmc_bootstrap wyrmc_stage1 compiler/wyrmc compiler/wyrmc.exe 2>/dev/null || true

# 7. Install binaries into ~/.wyrm/wyrmc and ~/.wyrm/wyrpkg
cp -f wyrmc "$WYRMC_DIR/wyrmc"
cp -f wyrpkg "$WYRPKG_DIR/wyrpkg"
chmod +x "$WYRMC_DIR/wyrmc" "$WYRPKG_DIR/wyrpkg"

echo -e "  wyrmc  -> ${CYAN}${WYRMC_DIR}/wyrmc${NC}"
echo -e "  wyrpkg -> ${CYAN}${WYRPKG_DIR}/wyrpkg${NC}"

# 8. Configure PATH in shell config files
PATH_CONFIG_LINE="export PATH=\"$WYRMC_DIR:$WYRPKG_DIR:\$PATH\""
ADDED_PATH=false

for RC_FILE in "$HOME/.bashrc" "$HOME/.zshrc" "$HOME/.profile"; do
    if [ -f "$RC_FILE" ]; then
        if ! grep -q ".wyrm/wyrmc" "$RC_FILE"; then
            echo "" >> "$RC_FILE"
            echo "# Wyrm Language toolchain" >> "$RC_FILE"
            echo "$PATH_CONFIG_LINE" >> "$RC_FILE"
            echo -e "${GREEN}Added Wyrm to PATH in ${RC_FILE}${NC}"
            ADDED_PATH=true
        else
            echo -e "${YELLOW}Wyrm is already in PATH in ${RC_FILE}${NC}"
            ADDED_PATH=true
        fi
    fi
done

if [ "$ADDED_PATH" = false ]; then
    PROFILE_FILE="$HOME/.profile"
    echo "" >> "$PROFILE_FILE"
    echo "# Wyrm Language toolchain" >> "$PROFILE_FILE"
    echo "$PATH_CONFIG_LINE" >> "$PROFILE_FILE"
    echo -e "${GREEN}Added Wyrm to PATH in ${PROFILE_FILE}${NC}"
fi

echo ""
echo -e "${GREEN}Wyrm v${WYRM_VERSION} installed successfully!${NC}"
echo -e "To start using Wyrm, run: ${CYAN}source ~/.bashrc${NC} (or ~/.zshrc)"
echo -e "Then verify with: ${CYAN}wyrmc --version${NC} and ${CYAN}wyrpkg --version${NC}"
