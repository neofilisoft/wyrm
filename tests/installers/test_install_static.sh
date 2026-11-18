#!/usr/bin/env bash
# Static regression tests for the portability/installation findings.
#
# The installers write to the user's global HOME and shell profile and, on
# Windows, to the User PATH. They intentionally have no HOME/install-root
# override, so this suite NEVER executes them. It validates their source with
# bash -n and structural assertions instead. WYRM-07's runtime behaviour is
# covered by test_llvm_target_inference.sh.
set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
INSTALL_SH="$REPO_ROOT/install.sh"
INSTALL_PS1="$REPO_ROOT/install.ps1"
WYRMC_WYR="$REPO_ROOT/compiler/wyrmc.wyr"

PASS_COUNT=0
FAIL_COUNT=0

pass_case() {
    PASS_COUNT=$((PASS_COUNT + 1))
    printf 'PASS: %s\n' "$1"
}

fail_case() {
    FAIL_COUNT=$((FAIL_COUNT + 1))
    printf 'FAIL: %s\n' "$1"
}

check_true() {
    local label=$1
    shift
    if "$@"; then
        pass_case "$label"
    else
        fail_case "$label"
    fi
}

check_false() {
    local label=$1
    shift
    if "$@"; then
        fail_case "$label"
    else
        pass_case "$label"
    fi
}

# All fourteen runtime translation units that the bootstrap compiler links.
RUNTIME_OBJECTS=(
    wyrm_core wyrm_arena wyrm_str wyrm_ffi
    wyrm_std_json wyrm_std_yaml wyrm_std_collections wyrm_std_random
    wyrm_std_time wyrm_std_os wyrm_std_fs wyrm_std_path wyrm_std_io wyrm_std_http
)

for f in "$INSTALL_SH" "$INSTALL_PS1" "$WYRMC_WYR"; do
    if [ ! -f "$f" ]; then
        printf 'Missing required file: %s\n' "$f" >&2
        exit 2
    fi
done

# ---------------------------------------------------------------------------
# WYRM-13: install.sh
# ---------------------------------------------------------------------------
if bash -n "$INSTALL_SH" 2>/dev/null; then
    pass_case 'WYRM-13 install.sh has valid bash syntax'
else
    fail_case 'WYRM-13 install.sh has valid bash syntax'
fi

check_true 'WYRM-13 install.sh checks for clang explicitly' \
    grep -Fq 'command -v clang' "$INSTALL_SH"
check_true 'WYRM-13 install.sh explains gcc/g++ cannot replace clang' \
    grep -Fq 'gcc/g++ alone are not sufficient' "$INSTALL_SH"

for obj in "${RUNTIME_OBJECTS[@]}"; do
    check_true "WYRM-13 install.sh compiles $obj.o" \
        grep -Fq -- "-o $obj.o" "$INSTALL_SH"
done

INSTALL_SH_LINK=$(sed -n '/^\$CXX wyrm\/src\/wyrmc.cpp/,/-o wyrmc_bootstrap/p' "$INSTALL_SH")
for obj in "${RUNTIME_OBJECTS[@]}"; do
    case "$INSTALL_SH_LINK" in
        *"$obj.o"*) pass_case "WYRM-13 install.sh links $obj.o into bootstrap" ;;
        *) fail_case "WYRM-13 install.sh links $obj.o into bootstrap" ;;
    esac
done
check_true 'WYRM-13 install.sh selects dependency-free HTTP runtime' \
    grep -Fq -- '-DWYRM_NO_CURL' "$INSTALL_SH"

# ---------------------------------------------------------------------------
# WYRM-13 / WYRM-17: install.ps1
# ---------------------------------------------------------------------------
check_true 'WYRM-13 install.ps1 checks for clang explicitly' \
    grep -Fq 'Get-Command clang' "$INSTALL_PS1"
check_true 'WYRM-13 install.ps1 explains gcc/g++ cannot replace clang' \
    grep -Fq 'gcc/g++ alone are not sufficient' "$INSTALL_PS1"

for obj in "${RUNTIME_OBJECTS[@]}"; do
    check_true "WYRM-13 install.ps1 compiles $obj.o" \
        grep -Fq -- "-o $obj.o" "$INSTALL_PS1"
done

INSTALL_PS1_LINK=$(sed -n '/g++ wyrm\\src\\wyrmc.cpp/,/-o wyrmc_bootstrap.exe/p' "$INSTALL_PS1")
for obj in "${RUNTIME_OBJECTS[@]}"; do
    case "$INSTALL_PS1_LINK" in
        *"$obj.o"*) pass_case "WYRM-13 install.ps1 links $obj.o into bootstrap" ;;
        *) fail_case "WYRM-13 install.ps1 links $obj.o into bootstrap" ;;
    esac
done
check_true 'WYRM-13 install.ps1 selects dependency-free HTTP runtime' \
    grep -Fq -- '-DWYRM_NO_CURL' "$INSTALL_PS1"

check_false 'WYRM-17 install.ps1 has no global name-based process kill' \
    grep -Fq 'Get-Process wyrmc' "$INSTALL_PS1"
check_false 'WYRM-17 install.ps1 never Stop-Process by bare name list' \
    grep -Fq 'Stop-Process -Force -ErrorAction SilentlyContinue' "$INSTALL_PS1"
check_true 'WYRM-17 install.ps1 tracks processes it starts' \
    grep -Fq 'InstallerOwnedProcesses.Add' "$INSTALL_PS1"
check_true 'WYRM-17 install.ps1 stops only installer-owned processes' \
    grep -Fq 'Stop-InstallerOwnedProcesses' "$INSTALL_PS1"
check_true 'WYRM-17 install.ps1 stops a process by recorded Id' \
    grep -Fq 'Stop-Process -Id $proc.Id' "$INSTALL_PS1"
check_true 'WYRM-17 install.ps1 reports file-lock failures with guidance' \
    grep -Fq 'currently in use' "$INSTALL_PS1"

# ---------------------------------------------------------------------------
# WYRM-07: no hard-coded Windows target in emitted IR
# ---------------------------------------------------------------------------
check_false 'WYRM-07 no hard-coded Windows target triple' \
    grep -Fq 'target triple = \"x86_64-pc-windows-msvc\"' "$WYRMC_WYR"
check_false 'WYRM-07 no hard-coded Windows data layout' \
    grep -Fq 'target datalayout = \"e-m:w-' "$WYRMC_WYR"
check_true 'WYRM-07 documents why target lines are omitted' \
    grep -Fq 'Omitting both lets clang derive the' "$WYRMC_WYR"

printf '\nTOTAL: %d passed, %d failed\n' "$PASS_COUNT" "$FAIL_COUNT"
[ "$FAIL_COUNT" -eq 0 ]
