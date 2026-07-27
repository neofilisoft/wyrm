"""
tests/test_ownership.py
Tests for the Wyrm memory safety system:
  - unsafe {} blocks
  - owned variable declarations
  - Arena allocator (interpreter level)
  - Ownership error: use-after-move
  - Arena use-after-reset (use-after-free analogue)
  - malloc bug fixes in C runtime (transpiler level)
"""
import pytest
from wyrm.lexer import Lexer
from wyrm.parser import Parser
from wyrm.interpreter import Interpreter, OwnershipError, ArenaAllocator, Environment


def parse_and_run(code: str) -> Interpreter:
    lexer = Lexer(code)
    tokens = lexer.tokenize()
    parser = Parser(tokens)
    ast = parser.parse()
    interp = Interpreter()
    interp.interpret(ast)
    return interp


# ---- Lexer / Parser: new keywords ----------------------------------------

def test_lexer_recognizes_unsafe():
    from wyrm.lexer import KEYWORDS
    assert 'unsafe' in KEYWORDS

def test_lexer_recognizes_owned():
    from wyrm.lexer import KEYWORDS
    assert 'owned' in KEYWORDS

def test_lexer_recognizes_arena():
    from wyrm.lexer import KEYWORDS
    assert 'arena' in KEYWORDS

def test_parser_unsafe_block():
    from wyrm.ast import UnsafeBlockNode
    code = "unsafe { var x = 1 }"
    lexer = Lexer(code)
    tokens = lexer.tokenize()
    ast = Parser(tokens).parse()
    assert len(ast) == 1
    assert isinstance(ast[0], UnsafeBlockNode)
    assert len(ast[0].body) == 1

def test_parser_owned_declaration():
    from wyrm.ast import OwnedDeclNode
    code = "owned var x = 42"
    lexer = Lexer(code)
    tokens = lexer.tokenize()
    ast = Parser(tokens).parse()
    assert isinstance(ast[0], OwnedDeclNode)
    assert ast[0].var_name == 'x'

def test_parser_arena_declaration():
    from wyrm.ast import ArenaNode
    code = "arena buf(1024)"
    lexer = Lexer(code)
    tokens = lexer.tokenize()
    ast = Parser(tokens).parse()
    assert isinstance(ast[0], ArenaNode)
    assert ast[0].name == 'buf'

def test_parser_arena_alloc():
    from wyrm.ast import ArenaAllocNode
    code = """
arena buf(1024)
buf.alloc(64)
"""
    lexer = Lexer(code)
    tokens = lexer.tokenize()
    ast = Parser(tokens).parse()
    from wyrm.ast import ArenaNode, ArenaAllocNode
    assert isinstance(ast[0], ArenaNode)
    assert isinstance(ast[1], ArenaAllocNode)
    assert ast[1].arena_name == 'buf'

def test_parser_arena_reset():
    from wyrm.ast import ArenaResetNode
    code = """
arena buf(512)
buf.reset()
"""
    lexer = Lexer(code)
    tokens = lexer.tokenize()
    ast = Parser(tokens).parse()
    from wyrm.ast import ArenaResetNode
    assert isinstance(ast[1], ArenaResetNode)
    assert ast[1].arena_name == 'buf'


# ---- Interpreter: unsafe block --------------------------------------------

def test_unsafe_block_executes():
    code = """
unsafe {
    var x = 99
}
"""
    # Should not raise - unsafe block is valid and runs
    interp = parse_and_run(code)

def test_unsafe_block_variables_isolated():
    """Variables declared inside unsafe {} are in a sub-scope (not visible outside)."""
    code = """
unsafe {
    var secret = 42
}
"""
    interp = parse_and_run(code)
    # 'secret' should NOT be visible in the global scope
    with pytest.raises(Exception):
        interp.global_env.get('secret')


# ---- Interpreter: owned variables -----------------------------------------

def test_owned_var_basic():
    code = "owned var count = 10"
    interp = parse_and_run(code)
    assert interp.global_env.get('count') == 10

def test_owned_var_is_marked_owned():
    code = "owned var count = 5"
    interp = parse_and_run(code)
    # The variable should be in the _owned set
    assert 'count' in interp.global_env._owned

def test_owned_var_can_be_read():
    code = """
owned var result = 100
"""
    interp = parse_and_run(code)
    assert interp.global_env.get('result') == 100

def test_mark_moved_raises_on_access():
    """Simulating a move: after mark_moved(), accessing the variable must raise OwnershipError."""
    env = Environment()
    env.define('x', 42, is_owned=True)
    assert env.get('x') == 42
    env.mark_moved('x')
    with pytest.raises(OwnershipError):
        env.get('x')

def test_moved_variable_cannot_be_reassigned():
    env = Environment()
    env.define('x', 10, is_owned=True)
    env.mark_moved('x')
    with pytest.raises(OwnershipError):
        env.set('x', 20)


# ---- Arena Allocator (interpreter) ----------------------------------------

def test_arena_create():
    code = "arena tmp(256)"
    interp = parse_and_run(code)
    arena = interp.global_env.get_arena('tmp')
    assert isinstance(arena, ArenaAllocator)
    assert arena.capacity == 256

def test_arena_alloc_returns_bytearray():
    code = "arena tmp(256)"
    interp = parse_and_run(code)
    arena = interp.global_env.get_arena('tmp')
    buf = arena.alloc(32)
    assert isinstance(buf, bytearray)
    assert len(buf) == 32

def test_arena_multiple_allocs():
    arena = ArenaAllocator('test', 1024)
    b1 = arena.alloc(64)
    b2 = arena.alloc(128)
    assert len(b1) == 64
    assert len(b2) == 128

def test_arena_reset_prevents_further_alloc():
    arena = ArenaAllocator('test', 1024)
    arena.alloc(64)
    arena.reset()
    with pytest.raises(OwnershipError, match="use-after-free"):
        arena.alloc(32)

def test_arena_alloc_invalid_size():
    arena = ArenaAllocator('test', 512)
    with pytest.raises(OwnershipError, match="positive"):
        arena.alloc(0)

def test_arena_interpreter_alloc_reset():
    code = """
arena scratch(1024)
scratch.alloc(32)
scratch.reset()
"""
    # Should run without error
    parse_and_run(code)

def test_arena_use_after_reset_raises():
    code = """
arena scratch(512)
scratch.reset()
"""
    interp = parse_and_run(code)
    arena = interp.global_env.get_arena('scratch')
    # After reset, allocating must fail
    with pytest.raises(OwnershipError):
        arena.alloc(16)


# ---- Transpiler: malloc bug checks ----------------------------------------

def test_transpiler_c_runtime_has_null_checks():
    """Check that val_array_create in the C runtime includes malloc null-checks."""
    from wyrm.transpiler import C_RUNTIME
    # Both malloc calls in val_array_create must be followed by a null check
    assert "if (!v.as.array)" in C_RUNTIME
    assert "if (!v.as.array->data)" in C_RUNTIME

def test_transpiler_arena_c_runtime_included():
    """Verify that the arena C runtime structs and functions are embedded."""
    from wyrm.transpiler import C_RUNTIME
    assert "WyrmArena" in C_RUNTIME
    assert "arena_create" in C_RUNTIME
    assert "arena_alloc" in C_RUNTIME
    assert "arena_reset" in C_RUNTIME
    assert "arena_destroy" in C_RUNTIME

def test_transpiler_arena_runtime_has_oom_checks():
    """arena_create must check malloc return values."""
    from wyrm.transpiler import C_RUNTIME
    assert "Out of memory" in C_RUNTIME

def test_transpiler_unsafe_emits_markers(tmp_path):
    """Transpiling an unsafe block emits /* unsafe block begin/end */ markers in C."""
    from wyrm.transpiler import transpile_to_c
    code = """
fn main() {
    unsafe {
        var x = 1
    }
}
"""
    c_source = transpile_to_c(code)
    assert "/* unsafe block begin */" in c_source
    assert "/* unsafe block end */" in c_source

def test_transpiler_owned_decl_emits_owned_comment(tmp_path):
    """Transpiling an owned variable includes /* owned */ annotation in C."""
    from wyrm.transpiler import transpile_to_c
    code = """
fn main() {
    owned var result = 42
}
"""
    c_source = transpile_to_c(code)
    assert "/* owned */" in c_source

def test_transpiler_arena_emits_arena_create():
    """Transpiling arena declaration emits arena_create() call."""
    from wyrm.transpiler import transpile_to_c
    code = """
fn main() {
    arena buf(512)
    buf.reset()
}
"""
    c_source = transpile_to_c(code)
    assert "arena_create" in c_source
    assert "arena_reset" in c_source
