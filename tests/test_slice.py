import pytest
import subprocess
from wyrm.lexer import Lexer
from wyrm.parser import Parser
from wyrm.interpreter import Interpreter
from wyrm.bytecode import Compiler, VM

def run_code_interpreter(code):
    lexer = Lexer(code)
    tokens = lexer.tokenize()
    parser = Parser(tokens)
    ast = parser.parse()
    interpreter = Interpreter()
    interpreter.interpret(ast)
    return interpreter

def run_code_vm(code):
    lexer = Lexer(code)
    tokens = lexer.tokenize()
    parser = Parser(tokens)
    ast = parser.parse()
    compiler = Compiler()
    bytecode = compiler.compile(ast)
    vm = VM()
    vm.execute(bytecode)
    return vm

def test_slicing_and_negative_indexing_interpreter():
    code = """
    var arr = [10, 20, 30, 40, 50]
    var s1 = arr[1:3]
    var s2 = arr[:2]
    var s3 = arr[3:]
    var s4 = arr[:]
    var s5 = arr[1:-1]
    var s6 = arr[-3:-1]
    
    var last = arr[-1]
    var second_last = arr[-2]
    
    var str_val = "hello"
    var str_slice = str_val[1:4]
    var str_neg = str_val[-1]
    """
    interpreter = run_code_interpreter(code)
    assert interpreter.global_env.get("s1") == [20, 30]
    assert interpreter.global_env.get("s2") == [10, 20]
    assert interpreter.global_env.get("s3") == [40, 50]
    assert interpreter.global_env.get("s4") == [10, 20, 30, 40, 50]
    assert interpreter.global_env.get("s5") == [20, 30, 40]
    assert interpreter.global_env.get("s6") == [30, 40]
    assert interpreter.global_env.get("last") == 50
    assert interpreter.global_env.get("second_last") == 40
    assert interpreter.global_env.get("str_slice") == "ell"
    assert interpreter.global_env.get("str_neg") == "o"

def test_slicing_and_negative_indexing_vm():
    code = """
    var arr = [10, 20, 30, 40, 50]
    var s1 = arr[1:3]
    var s2 = arr[:2]
    var s3 = arr[3:]
    var s4 = arr[:]
    var s5 = arr[1:-1]
    var s6 = arr[-3:-1]
    
    var last = arr[-1]
    var second_last = arr[-2]
    
    var str_val = "hello"
    var str_slice = str_val[1:4]
    var str_neg = str_val[-1]
    """
    vm = run_code_vm(code)
    assert vm.globals.get("s1") == [20, 30]
    assert vm.globals.get("s2") == [10, 20]
    assert vm.globals.get("s3") == [40, 50]
    assert vm.globals.get("s4") == [10, 20, 30, 40, 50]
    assert vm.globals.get("s5") == [20, 30, 40]
    assert vm.globals.get("s6") == [30, 40]
    assert vm.globals.get("last") == 50
    assert vm.globals.get("second_last") == 40
    assert vm.globals.get("str_slice") == "ell"
    assert vm.globals.get("str_neg") == "o"

def test_transpiler_slicing(tmp_path):
    code = """
    fn main() {
        var arr = [10, 20, 30, 40, 50]
        var s1 = arr[1:3]
        var last = arr[-1]
        print(s1[0], s1[1], last)
    }
    """
    from wyrm.llvm_codegen import LLVMCodeGen
    codegen = LLVMCodeGen()
    out_exe = tmp_path / "test_slice_bin"
    success = codegen.compile(code, str(out_exe))
    assert success
    
    res = subprocess.run([str(out_exe)], capture_output=True, text=True)
    assert res.returncode == 0
    assert res.stdout.strip() == "20 30 50"
