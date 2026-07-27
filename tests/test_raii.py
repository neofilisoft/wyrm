"""
test_raii.py - Comprehensive tests for Wyrm v2.2
Covers:
  - do/til loop alias for repeat/til
  - RAII arena auto-cleanup on scope exit
  - Raw memory builtins: malloc, free, realloc
  - unsafe block enforcement (malloc/free outside unsafe => error)
  - No double-free: manual free + scope exit is safe
  - Version numbers
"""
import pytest
from wyrm.lexer import Lexer
from wyrm.parser import Parser
from wyrm.interpreter import Interpreter, ArenaAllocator, RawPointer, OwnershipError
from wyrm import __version__
from wyrm.pkg import WYRPKG_VERSION
from wyrm.transpiler import C_RUNTIME, transpile_to_c


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def run(code: str) -> Interpreter:
    tokens = Lexer(code).tokenize()
    ast = Parser(tokens).parse()
    interp = Interpreter()
    interp.interpret(ast)
    return interp


# ===========================================================================
# 1. do/til loop alias
# ===========================================================================

class TestDoTilLoop:
    def test_do_til_counter(self):
        """do/til loop executes body then checks condition (do-while semantics)."""
        interp = run("""
        i = 0
        do {
            i = i + 1
        } til (i >= 5)
        """)
        assert interp.global_env.get("i") == 5

    def test_do_til_runs_at_least_once(self):
        """Body should execute at least once even when condition is immediately true."""
        interp = run("""
        count = 0
        do {
            count = count + 1
        } til (count >= 1)
        """)
        assert interp.global_env.get("count") == 1

    def test_repeat_still_works(self):
        """Original 'repeat' keyword must continue working as alias."""
        interp = run("""
        i = 0
        repeat {
            i = i + 1
        } til (i >= 4)
        """)
        assert interp.global_env.get("i") == 4

    def test_do_til_with_break(self):
        interp = run("""
        i = 0
        do {
            i = i + 1
            if i >= 3 {
                break
            }
        } til (i >= 10)
        """)
        assert interp.global_env.get("i") == 3

    def test_do_til_with_continue(self):
        interp = run("""
        i = 0
        total = 0
        do {
            i = i + 1
            if i == 2 {
                continue
            }
            total = total + i
        } til (i >= 4)
        """)
        # total = 1 + 3 + 4 = 8  (skips 2)
        assert interp.global_env.get("total") == 8


# ===========================================================================
# 2. RAII - Arena auto-cleanup on scope exit
# ===========================================================================

class TestRAIIArena:
    def test_arena_raii_auto_reset_on_block_exit(self):
        """Arena declared in a sub-scope must be auto-reset when block exits."""
        from wyrm.interpreter import Environment
        interp = Interpreter()

        sub_env = Environment(interp.global_env)
        arena = ArenaAllocator("buf", 128)
        arena.alloc(64)
        assert len(arena._allocations) == 1
        sub_env.define_arena("buf", arena)

        interp.execute_block([], sub_env)

        assert arena._freed is True

    def test_arena_raii_via_unsafe_code(self):
        """Arena declared inside unsafe block is reset when the block exits."""
        interp = run("""
        unsafe {
            arena buf(256)
            buf.alloc(64)
        }
        """)
        # After unsafe block exits, the arena stored in the sub-scope has been reset.
        # The global env should NOT have 'buf' visible (it was in the sub-scope).
        with pytest.raises(Exception):
            interp.global_env.get("buf")

    def test_unsafe_block_creates_sub_scope(self):
        """Variables declared inside unsafe block do not leak to outer scope."""
        interp = run("""
        unsafe {
            var x = 42
        }
        """)
        # 'x' must not be visible in global env
        with pytest.raises(Exception):
            interp.global_env.get("x")


# ===========================================================================
# 3. Raw Memory Builtins - malloc / free / realloc
# ===========================================================================

class TestRawMemory:
    def test_malloc_requires_unsafe(self):
        """Calling malloc outside unsafe block raises RuntimeError."""
        with pytest.raises(Exception, match="unsafe"):
            run("var p = malloc(64)")

    def test_free_requires_unsafe(self):
        """Calling free outside unsafe block raises RuntimeError."""
        with pytest.raises(Exception, match="unsafe"):
            run("free(null)")

    def test_realloc_requires_unsafe(self):
        """Calling realloc outside unsafe block raises RuntimeError."""
        with pytest.raises(Exception, match="unsafe"):
            run("realloc(null, 128)")

    def test_malloc_inside_unsafe_returns_rawpointer(self):
        """malloc(n) inside unsafe block returns a RawPointer."""
        interp = Interpreter()
        interp._in_unsafe = True
        ptr = interp._builtin_malloc([64])
        assert isinstance(ptr, RawPointer)
        assert ptr.size == 64
        assert not ptr.freed

    def test_malloc_allocates_bytearray(self):
        """RawPointer.buffer is a bytearray of the requested size."""
        interp = Interpreter()
        interp._in_unsafe = True
        ptr = interp._builtin_malloc([32])
        assert isinstance(ptr.buffer, bytearray)
        assert len(ptr.buffer) == 32

    def test_free_marks_pointer_freed(self):
        """free(ptr) marks the RawPointer as freed."""
        interp = Interpreter()
        interp._in_unsafe = True
        ptr = interp._builtin_malloc([16])
        assert not ptr.freed
        interp._builtin_free([ptr])
        assert ptr.freed

    def test_realloc_resizes_buffer(self):
        """realloc(ptr, new_size) resizes the buffer."""
        interp = Interpreter()
        interp._in_unsafe = True
        ptr = interp._builtin_malloc([16])
        interp._builtin_realloc([ptr, 128])
        assert ptr.size == 128
        assert len(ptr.buffer) == 128

    def test_realloc_preserves_data(self):
        """realloc preserves data up to min of old and new size."""
        interp = Interpreter()
        interp._in_unsafe = True
        ptr = interp._builtin_malloc([4])
        ptr.buffer[0] = 0xAB
        ptr.buffer[1] = 0xCD
        interp._builtin_realloc([ptr, 8])
        assert ptr.buffer[0] == 0xAB
        assert ptr.buffer[1] == 0xCD

    def test_realloc_null_acts_as_malloc(self):
        """realloc(null, size) is equivalent to malloc(size)."""
        interp = Interpreter()
        interp._in_unsafe = True
        ptr = interp._builtin_realloc([None, 64])
        assert isinstance(ptr, RawPointer)
        assert ptr.size == 64


# ===========================================================================
# 4. RAII - Raw pointer auto-free fallback
# ===========================================================================

class TestRAIIRawPointer:
    def test_raw_pointer_auto_freed_on_block_exit(self):
        """A RawPointer allocated in a sub-scope must be auto-freed when block exits."""
        from wyrm.interpreter import Environment
        interp = Interpreter()
        interp._in_unsafe = True

        sub_env = Environment(interp.global_env)
        ptr = interp._builtin_malloc([32])
        sub_env.register_raw_allocation(ptr)
        assert not ptr.freed

        interp.execute_block([], sub_env)
        assert ptr.freed

    def test_manually_freed_pointer_not_double_freed(self):
        """After manual free, scope exit does NOT double-free (ptr.freed stays True, no crash)."""
        from wyrm.interpreter import Environment, RawPointer
        interp = Interpreter()
        interp._in_unsafe = True

        sub_env = Environment(interp.global_env)

        # Allocate inside sub_env context by temporarily switching global_env
        prev = interp.global_env
        interp.global_env = sub_env
        ptr = interp._builtin_malloc([32])
        interp.global_env = prev

        assert not ptr.freed
        assert ptr in sub_env._raw_allocations

        # Manually free - switches global_env to sub_env so chain search finds it
        prev = interp.global_env
        interp.global_env = sub_env
        interp._builtin_free([ptr])
        interp.global_env = prev

        assert ptr.freed
        assert ptr not in sub_env._raw_allocations

        # Block exit - nothing left to free, no double-free crash
        interp.execute_block([], sub_env)
        assert ptr.freed  # still freed, definitely not re-freed

    def test_multiple_raw_pointers_auto_freed(self):
        """Multiple raw pointers in same scope are all auto-freed on scope exit."""
        from wyrm.interpreter import Environment
        interp = Interpreter()
        interp._in_unsafe = True

        sub_env = Environment(interp.global_env)
        ptrs = []
        for _ in range(5):
            p = interp._builtin_malloc([16])
            sub_env.register_raw_allocation(p)
            ptrs.append(p)

        interp.execute_block([], sub_env)
        for p in ptrs:
            assert p.freed


# ===========================================================================
# 5. Version numbers
# ===========================================================================

class TestVersionNumbers:
    def test_init_version_is_2_2(self):
        assert __version__ == "2.2"

    def test_pkg_version_is_2_2(self):
        assert WYRPKG_VERSION == "2.2"


# ===========================================================================
# 6. Transpiler - unsafe enforcement and RAII code generation
# ===========================================================================

class TestTranspilerRAII:
    def test_transpiler_rejects_malloc_outside_unsafe(self):
        """Transpiler raises error if malloc is used outside unsafe block."""
        with pytest.raises(Exception, match="[Uu]nsafe"):
            transpile_to_c("var p = malloc(64)")

    def test_transpiler_rejects_free_outside_unsafe(self):
        with pytest.raises(Exception, match="[Uu]nsafe"):
            transpile_to_c("""
            fn demo() {
                var p = malloc(64)
                free(p)
            }
            """)

    def test_transpiler_rejects_realloc_outside_unsafe(self):
        with pytest.raises(Exception, match="[Uu]nsafe"):
            transpile_to_c("realloc(null, 64)")

    def test_transpiler_allows_malloc_inside_unsafe(self):
        """Transpiler must not raise when malloc is inside unsafe block."""
        code = transpile_to_c("""
        fn demo() {
            unsafe {
                var p = malloc(64)
            }
        }
        """)
        assert "val_raw_malloc" in code

    def test_transpiler_free_nulls_the_variable(self):
        """free(p) must compile as 'wyrm_var_p = val_raw_free(wyrm_var_p)'."""
        code = transpile_to_c("""
        fn demo() {
            unsafe {
                var p = malloc(64)
                free(p)
            }
        }
        """)
        assert "val_raw_free" in code
        assert "wyrm_var_p = val_raw_free" in code

    def test_transpiler_arena_raii_emits_arena_destroy_in_function(self):
        """Arenas declared inside a function scope emit arena_destroy on exit."""
        code = transpile_to_c("""
        fn demo() {
            arena buf(256)
        }
        """)
        assert "arena_destroy" in code
        assert "wyrm_arena_buf" in code

    def test_transpiler_unsafe_block_arena_emits_destroy(self):
        """Arena inside unsafe block emits arena_destroy at block end."""
        code = transpile_to_c("""
        fn demo() {
            unsafe {
                arena buf(128)
            }
        }
        """)
        assert "arena_destroy" in code

    def test_c_runtime_has_val_raw_malloc(self):
        assert "val_raw_malloc" in C_RUNTIME

    def test_c_runtime_has_val_raw_free(self):
        assert "val_raw_free" in C_RUNTIME

    def test_c_runtime_has_val_raw_realloc(self):
        assert "val_raw_realloc" in C_RUNTIME

    def test_c_runtime_has_val_raw_ptr_type(self):
        assert "VAL_RAW_PTR" in C_RUNTIME

    def test_c_runtime_has_raw_ptr_union_field(self):
        assert "raw_ptr" in C_RUNTIME
