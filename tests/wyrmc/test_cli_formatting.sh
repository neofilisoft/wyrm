#!/usr/bin/env bash
# Regression tests for the Stage 0 wyrmc CLI formatting, clean and help output.
# Run with WYRMC_BIN pointing at a freshly built wyrmc_stage0 binary.
set -u

WYRMC_BIN=${WYRMC_BIN:-./wyrmc_stage0}
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

if [ ! -x "$WYRMC_BIN" ]; then
    printf 'Cannot execute WYRMC_BIN=%s\n' "$WYRMC_BIN" >&2
    exit 2
fi

WORK_DIR=$(mktemp -d)
trap 'rm -rf "$WORK_DIR"' EXIT

# ---------------------------------------------------------------------------
# WYRM-09: lexical-state-aware formatting
# ---------------------------------------------------------------------------
FMT_DIR="$WORK_DIR/fmt"
mkdir -p "$FMT_DIR"

cat > "$FMT_DIR/literals.wyr" <<'EOF'
// line comment with braces { unmatched and } paired
fn main() {
    print("Hello {world} and \"quoted\" braces {x}")
    print('single {quote} and \'escaped\'')
    /* block comment
       opening { brace
       closing } brace
    */
    if (true) {
        print("nested {value}")
    }
}
EOF

"$WYRMC_BIN" check "$FMT_DIR/literals.wyr" >"$FMT_DIR/pre.out" 2>"$FMT_DIR/pre.err"
check_true 'WYRM-09 source parses before formatting' test "$?" -eq 0
cp "$FMT_DIR/literals.wyr" "$FMT_DIR/literals.orig"

"$WYRMC_BIN" fmt "$FMT_DIR/literals.wyr" >"$FMT_DIR/fmt.out" 2>"$FMT_DIR/fmt.err"
check_true 'WYRM-09 fmt succeeds on valid source' test "$?" -eq 0

check_true 'WYRM-09 backup saved original' \
    cmp -s "$FMT_DIR/literals.wyr.bak" "$FMT_DIR/literals.orig"

check_true 'WYRM-09 string braces preserved' \
    grep -Fqx '    print("Hello {world} and \"quoted\" braces {x}")' "$FMT_DIR/literals.wyr"
check_true 'WYRM-09 single-quote braces preserved' \
    grep -Fqx "    print('single {quote} and \\'escaped\\'')" "$FMT_DIR/literals.wyr"
check_true 'WYRM-09 line-comment braces preserved' \
    grep -Fqx '// line comment with braces { unmatched and } paired' "$FMT_DIR/literals.wyr"
check_true 'WYRM-09 block-comment brace lines preserved' \
    grep -Fqx '       opening { brace' "$FMT_DIR/literals.wyr"
check_true 'WYRM-09 nested indentation stable' \
    grep -Fqx '        print("nested {value}")' "$FMT_DIR/literals.wyr"

cp "$FMT_DIR/literals.wyr" "$FMT_DIR/pass1.wyr"
"$WYRMC_BIN" fmt "$FMT_DIR/literals.wyr" >/dev/null 2>&1
check_true 'WYRM-09 formatting is idempotent' \
    cmp -s "$FMT_DIR/pass1.wyr" "$FMT_DIR/literals.wyr"

"$WYRMC_BIN" check "$FMT_DIR/literals.wyr" >"$FMT_DIR/post.out" 2>"$FMT_DIR/post.err"
check_true 'WYRM-09 source parses after formatting' test "$?" -eq 0

cat > "$FMT_DIR/floor.wyr" <<'EOF'
fn main() {
    var q = 7 // 2
    print(q)
}
EOF
"$WYRMC_BIN" check "$FMT_DIR/floor.wyr" >/dev/null 2>&1
check_true 'WYRM-09 floor-division fixture parses' test "$?" -eq 0
"$WYRMC_BIN" fmt "$FMT_DIR/floor.wyr" >/dev/null 2>&1
check_true 'WYRM-09 floor division is not misread as a comment' \
    grep -Fqx '    var q = 7 // 2' "$FMT_DIR/floor.wyr"

cat > "$FMT_DIR/continuations.wyr" <<'EOF'
fn main() {
    var x = 1
    if (x == 1) {
        print("one")
    } elif (x == 2) {
        print("two")
    } else {
        print("other")
    }
    do {
        x = x + 1
    } til x > 3
}
EOF
"$WYRMC_BIN" check "$FMT_DIR/continuations.wyr" >/dev/null 2>&1
check_true 'WYRM-09 continuation fixture parses before formatting' test "$?" -eq 0
"$WYRMC_BIN" fmt "$FMT_DIR/continuations.wyr" >/dev/null 2>&1
check_true 'WYRM-09 } elif stays attached' \
    grep -Fqx '    } elif (x == 2) {' "$FMT_DIR/continuations.wyr"
check_true 'WYRM-09 } else stays attached' \
    grep -Fqx '    } else {' "$FMT_DIR/continuations.wyr"
check_true 'WYRM-09 } til stays attached' \
    grep -Fqx '    } til x > 3' "$FMT_DIR/continuations.wyr"
"$WYRMC_BIN" check "$FMT_DIR/continuations.wyr" >/dev/null 2>&1
check_true 'WYRM-09 continuation fixture parses after formatting' test "$?" -eq 0

# Malformed inputs are rejected without overwriting the source.
make_malformed_case() {
    local name=$1
    local body=$2
    local dir="$FMT_DIR/$name"
    mkdir -p "$dir"
    printf '%s' "$body" > "$dir/case.wyr"
    cp "$dir/case.wyr" "$dir/before.wyr"
    "$WYRMC_BIN" fmt "$dir/case.wyr" >"$dir/out" 2>"$dir/err"
    local rc=$?
    check_true "WYRM-09 $name rejected" test "$rc" -ne 0
    check_true "WYRM-09 $name leaves source unchanged" \
        cmp -s "$dir/before.wyr" "$dir/case.wyr"
    check_false "WYRM-09 $name creates no backup" test -e "$dir/case.wyr.bak"
}

make_malformed_case 'missing_brace' 'fn main() {
    print("x")
'
make_malformed_case 'extra_brace' 'fn main() {
}
}'
make_malformed_case 'unterminated_string' 'fn main() {
    print("oops)
}'
make_malformed_case 'unterminated_comment' 'fn main() {
    /* never closed
}'

# ---------------------------------------------------------------------------
# WYRM-15: clean is confined to Wyrm-owned artifacts
# ---------------------------------------------------------------------------
CLEAN_DIR="$WORK_DIR/clean"
mkdir -p "$CLEAN_DIR/subdir"
: > "$CLEAN_DIR/_wyrm_temp.c"
mkdir -p "$CLEAN_DIR/.wyrm_cache"
: > "$CLEAN_DIR/.wyrm_cache/cached.dat"
: > "$CLEAN_DIR/main.wyr"
: > "$CLEAN_DIR/main.exe"
: > "$CLEAN_DIR/foreign.o"
: > "$CLEAN_DIR/foreign.obj"
: > "$CLEAN_DIR/other_temp.ll"
: > "$CLEAN_DIR/runner_run.exe"
: > "$CLEAN_DIR/notes.txt"
: > "$CLEAN_DIR/keep.exe"
: > "$CLEAN_DIR/subdir/nested.o"

( cd "$CLEAN_DIR" && "$WYRMC_BIN" clean >/dev/null 2>&1 )
check_true 'WYRM-15 clean succeeds' test "$?" -eq 0
check_false 'WYRM-15 removes _wyrm_temp.c' test -e "$CLEAN_DIR/_wyrm_temp.c"
check_false 'WYRM-15 removes .wyrm_cache' test -e "$CLEAN_DIR/.wyrm_cache"
check_true 'WYRM-15 keeps foreign .o' test -e "$CLEAN_DIR/foreign.o"
check_true 'WYRM-15 keeps foreign .obj' test -e "$CLEAN_DIR/foreign.obj"
check_true 'WYRM-15 keeps foreign *_temp.ll' test -e "$CLEAN_DIR/other_temp.ll"
check_true 'WYRM-15 keeps foreign *_run.exe' test -e "$CLEAN_DIR/runner_run.exe"
check_true 'WYRM-15 keeps notes.txt' test -e "$CLEAN_DIR/notes.txt"

( cd "$CLEAN_DIR" && "$WYRMC_BIN" clean --all >/dev/null 2>&1 )
check_true 'WYRM-15 clean --all succeeds' test "$?" -eq 0
check_false 'WYRM-15 --all removes exe with matching .wyr' test -e "$CLEAN_DIR/main.exe"
check_true 'WYRM-15 --all keeps unrelated exe' test -e "$CLEAN_DIR/keep.exe"
check_true 'WYRM-15 --all keeps foreign objects' test -e "$CLEAN_DIR/foreign.o"
check_true 'WYRM-15 stays inside current directory' test -e "$CLEAN_DIR/subdir/nested.o"

# ---------------------------------------------------------------------------
# WYRM-11: help text is truthful
# ---------------------------------------------------------------------------
HELP=$("$WYRMC_BIN" --help)
case "$HELP" in
    *'Parse/lint only (no type checking or execution)'*)
        pass_case 'WYRM-11 check help avoids type-checking claim' ;;
    *) fail_case 'WYRM-11 check help avoids type-checking claim' ;;
esac
case "$HELP" in
    *'test, check and fmt are'*)
        pass_case 'WYRM-11 help states self-hosted command gap' ;;
    *) fail_case 'WYRM-11 help states self-hosted command gap' ;;
esac
case "$HELP" in
    *'Type-check and lint only'*)
        fail_case 'WYRM-11 help has no stale type-check claim' ;;
    *) pass_case 'WYRM-11 help has no stale type-check claim' ;;
esac

printf '\nTOTAL: %d passed, %d failed\n' "$PASS_COUNT" "$FAIL_COUNT"
[ "$FAIL_COUNT" -eq 0 ]
