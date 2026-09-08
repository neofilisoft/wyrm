#!/usr/bin/env bash
# Wyrm Language Global Installer for Linux & macOS
# Installs native Wyrm tools into the user directory layout:
#   $HOME/.wyrm/wyrmc/wyrmc
#   $HOME/.wyrm/wyrpkg/wyrpkg
#   $HOME/.wyrm/packages/wyrmlang
#   $HOME/.wyrm/library

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VERSION_FILE="$SCRIPT_DIR/VERSION"

if [ -f "$VERSION_FILE" ]; then
    WYRM_VERSION="$(cat "$VERSION_FILE" | tr -d '[:space:]')"
else
    WYRM_VERSION="1.0.0"
fi

INSTALL_ROOT="$HOME/.wyrm"
WYRMC_DIR="$INSTALL_ROOT/wyrmc"
WYRPKG_DIR="$INSTALL_ROOT/wyrpkg"
PACKAGES_DIR="$INSTALL_ROOT/packages/wyrmlang"
LIBRARY_DIR="$INSTALL_ROOT/library"

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
mkdir -p "$INSTALL_ROOT" "$WYRMC_DIR" "$WYRPKG_DIR" "$PACKAGES_DIR" "$LIBRARY_DIR"

# 2. Copy Wyrm runtime standard library files to packages directory
echo "Copying Wyrm runtime standard library files..."
cp -rf "$SCRIPT_DIR/wyrm/"* "$PACKAGES_DIR/"

# Copy third-party libraries (e.g. glsl) to library directory if available
if [ -d "$SCRIPT_DIR/library" ]; then
    echo "Copying Wyrm third-party libraries..."
    cp -rf "$SCRIPT_DIR/library/"* "$LIBRARY_DIR/"
fi

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
$CC -std=c11 -O2 -c wyrm/lib/stdlib/wyrm_std_random.c -o wyrm_std_random.o -Iwyrm/lib
$CC -std=c11 -O2 -c wyrm/lib/stdlib/wyrm_std_time.c -o wyrm_std_time.o -Iwyrm/lib

# 4. Compile temporary C++ bootstrap compiler
echo "Compiling bootstrap compiler..."
$CXX wyrm/src/wyrmc.cpp \
    compiler/lexer/lexer.cpp \
    compiler/parser/parser.cpp \
    compiler/interpreter/interpreter.cpp \
    compiler/interpreter/builtins.cpp \
    compiler/interpreter/stdlib_setup.cpp \
    compiler/transpiler/transpiler.cpp \
    wyrm_core.o wyrm_arena.o wyrm_str.o \
    wyrm_ffi.o wyrm_std_json.o wyrm_std_yaml.o \
    wyrm_std_sdl.o wyrm_std_collections.o wyrm_std_random.o wyrm_std_time.o \
    -o wyrmc_bootstrap -std=c++20 -O2

# 5. Build self-hosted compiler stages
echo -e "${CYAN}Self-hosting Stage 1: Building self-hosted compiler using bootstrap compiler...${NC}"
rm -f compiler/wyrmc_temp.ll wyrmc_temp.ll compiler/wyrmc compiler/wyrmc.exe wyrmc_stage1 wyrmc 2>/dev/null || true

./wyrmc_bootstrap build compiler/wyrmc.wyr
if [ -f "compiler/wyrmc" ]; then
    cp -f compiler/wyrmc ./wyrmc_stage1
elif [ -f "compiler/wyrmc.exe" ]; then
    cp -f compiler/wyrmc.exe ./wyrmc_stage1
else
    echo -e "${RED}Error: Failed to compile Stage 1 self-hosted compiler.${NC}"
    exit 1
fi
rm -f compiler/wyrmc compiler/wyrmc.exe compiler/wyrmc_temp.ll 2>/dev/null || true

echo -e "${CYAN}Self-hosting Stage 2: Rebuilding self-hosted compiler using Stage 1 compiler...${NC}"
./wyrmc_stage1 build compiler/wyrmc.wyr -o wyrmc
if [ ! -f "wyrmc" ] && [ -f "compiler/wyrmc" ]; then
    cp -f compiler/wyrmc ./wyrmc
elif [ ! -f "wyrmc" ] && [ -f "wyrmc.exe" ]; then
    cp -f wyrmc.exe ./wyrmc
fi

if [ ! -f "wyrmc" ]; then
    echo -e "${RED}Error: Failed to compile Stage 2 self-hosted compiler.${NC}"
    exit 1
fi
rm -f compiler/wyrmc_temp.ll wyrmc_temp.ll 2>/dev/null || true

# 6. Compile wyrpkg
echo "Compiling package manager (wyrpkg)..."
$CXX wyrm/src/wyrpkg.cpp -o wyrpkg -std=c++20 -O2

# Clean up intermediate build artifacts
rm -f *.o wyrmc_bootstrap wyrmc_stage1 compiler/wyrmc compiler/wyrmc.exe 2>/dev/null || true

# 7. Install binaries into ~/.wyrm/wyrmc and ~/.wyrm/wyrpkg
cp -f wyrmc "$WYRMC_DIR/wyrmc"
cp -f wyrpkg "$WYRPKG_DIR/wyrpkg"
chmod +x "$WYRMC_DIR/wyrmc" "$WYRPKG_DIR/wyrpkg"

echo -e "  wyrmc  -> ${CYAN}${WYRMC_DIR}/wyrmc${NC}"
echo -e "  wyrpkg -> ${CYAN}${WYRPKG_DIR}/wyrpkg${NC}"

# 8. Install or update IDE syntax extensions (VS Code, Antigravity IDE, Cursor)
EXT_DIRS=(
    "$HOME/.vscode/extensions"
    "$HOME/.antigravity-ide/extensions"
    "$HOME/.cursor/extensions"
)
for EXT_DIR in "${EXT_DIRS[@]}"; do
    PARENT_DIR="$(dirname "$EXT_DIR")"
    if [ -d "$PARENT_DIR" ]; then
        mkdir -p "$EXT_DIR"
        TARGET_SYNTAX_DIR="$EXT_DIR/neofilisoft.wyrm-syntax-$WYRM_VERSION"
        rm -rf "$EXT_DIR"/neofilisoft.wyrm-syntax-* 2>/dev/null || true
        mkdir -p "$TARGET_SYNTAX_DIR"
        cp -rf "$SCRIPT_DIR/extension/"* "$TARGET_SYNTAX_DIR/"
        echo -e "  syntax -> ${CYAN}${TARGET_SYNTAX_DIR}${NC}"
    fi
done

# 9. Configure PATH in shell config files
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
