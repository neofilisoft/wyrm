# Implementation Plan - Standardize Null, Clean Unused Files, Fix Linguist & CI

This plan addresses four items:
1. Standardizing empty value handling to `null` across Python VM/Bytecode and Interpreter.
2. Cleaning up 31 unused temporary `.py` and `.wyr` scratch files in the root directory.
3. Explaining why GitHub detects Python as the primary language and updating `.gitattributes` so C is recognized as the primary repository language.
4. Auditing and improving `.github/workflows/ci.yml` to compile and test native C components (`wyrmc`, `wyrpkg`) alongside Python tests.

## User Review Required

> [!IMPORTANT]
> - Cleaning up root scratch files will remove 14 temporary `.py` files and 17 temporary `.wyr` files from root directory. All official tests remain intact under `tests/` and official examples under `examples/`.
> - GitHub Linguist requires `linguist-vendored=true` or `linguist-detectable=false` on `*.py` files so that Python VM infrastructure does not skew the GitHub language stats bar away from C / Wyrm native codebase.

## Open Questions

> [!NOTE]
> No critical open questions. All requirements are clear and follow repository guidelines.

## Proposed Changes

---

### Core Interpreter & Bytecode

#### [MODIFY] [bytecode.py](file:///c:/Users/BEST/Desktop/wyrm/wyrm/bytecode.py)
- Update `VM._init_builtins()` to register `null` as `None` (and lowercase `true`, `false`).
- Update `OpCode.PRINT` in `VM._execute_instruction()` to print `"null"` when values are `None`, matching `interpreter.py` and `wyrmc.c`.

---

### File Cleanup (Unused Root Files)

#### [DELETE] Temporary `.py` Files (14 files)
- `debug_bytecode.py`
- `final_test.py`
- `test_bytecode.py`
- `test_bytecode_features.py`
- `test_examples.py`
- `test_final.py`
- `test_hello.py`
- `test_hello_debug.py`
- `test_hello_temp.py`
- `test_none.py`
- `test_run.py`
- `test_simple_bytecode.py`
- `test_very_simple.py`
- `version_check.py`

#### [DELETE] Temporary `.wyr` Files (17 files)
- `simple_test.wyr`
- `test_amp.wyr`
- `test_and_op.wyr`
- `test_bool.wyr`
- `test_bytecode_features.wyr`
- `test_debug.wyr`
- `test_if.wyr`
- `test_logic_ops.wyr`
- `test_logical_symbolic.wyr`
- `test_loop.wyr`
- `test_module_helper.wyr`
- `test_simple_assign.wyr`
- `test_symbolic_ops.wyr`
- `test_var.wyr`
- `test_var_dec.wyr`
- `test_var_dec_errors.wyr`
- `test_var_dec_errors2.wyr`

---

### GitHub Linguist Configuration

#### [MODIFY] [.gitattributes](file:///c:/Users/BEST/Desktop/wyrm/.gitattributes)
- Set `*.py linguist-vendored=true` and `*.py linguist-detectable=false` so Python infrastructure files are excluded from language statistics.
- Ensure `*.c` (`wyrmc.c`, `wyrpkg.c`) remains `linguist-detectable=true` and `linguist-language=C`.

---

### CI Workflow Integration

#### [MODIFY] [ci.yml](file:///c:/Users/BEST/Desktop/wyrm/.github/workflows/ci.yml)
- Add build step to compile `wyrmc.c` with GCC (`gcc -std=c11 wyrmc.c -o wyrmc`).
- Add build step to compile `wyrpkg.c` with GCC (`gcc -std=c11 wyrpkg.c -o wyrpkg`).
- Add test run step for compiled `wyrmc` binary.

---

## Verification Plan

### Automated Tests
- Run `python -m pytest tests/ -v` to ensure all standard tests pass.
- Run `gcc -std=c11 wyrmc.c -o wyrmc.exe` and test running bytecode execution.

### Manual Verification
- Verify root directory contains only clean configuration, C source files, docs, `wyrm/`, `tests/`, and `examples/`.
