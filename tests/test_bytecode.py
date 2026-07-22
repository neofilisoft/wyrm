import pytest
from wyrm.lexer import Lexer
from wyrm.parser import Parser
from wyrm.bytecode import Compiler, VM, compile_and_execute

def test_bytecode_arithmetic_and_variables():
    source = """
    var x = 10
    var y = 5
    var z = (x + y * 2) - 4 / 2
    """
    vm = compile_and_execute(source)
    assert vm.globals.get("x") == 10
    assert vm.globals.get("y") == 5
    assert vm.globals.get("z") == 18.0

def test_bytecode_if_elif_else():
    source_then = """
    var score = 95
    var result = ""
    if score >= 90 {
        result = "A"
    } elif score >= 80 {
        result = "B"
    } else {
        result = "C"
    }
    """
    vm_then = compile_and_execute(source_then)
    assert vm_then.globals.get("result") == "A"

    source_elif = """
    var score = 85
    var result = ""
    if score >= 90 {
        result = "A"
    } elif score >= 80 {
        result = "B"
    } else {
        result = "C"
    }
    """
    vm_elif = compile_and_execute(source_elif)
    assert vm_elif.globals.get("result") == "B"

    source_else = """
    var score = 70
    var result = ""
    if score >= 90 {
        result = "A"
    } elif score >= 80 {
        result = "B"
    } else {
        result = "C"
    }
    """
    vm_else = compile_and_execute(source_else)
    assert vm_else.globals.get("result") == "C"

def test_bytecode_loop_break_continue():
    # Test repeat/til loop with break
    source_break = """
    var i = 0
    repeat {
        i = i + 1
        if i == 3 {
            break
        }
    } til (i >= 5)
    """
    vm_break = compile_and_execute(source_break)
    assert vm_break.globals.get("i") == 3

    # Test repeat/til loop with continue
    source_continue = """
    var sum = 0
    var k = 0
    repeat {
        k = k + 1
        if k == 2 {
            continue
        }
        sum = sum + k
    } til (k >= 3)
    """
    vm_continue = compile_and_execute(source_continue)
    assert vm_continue.globals.get("sum") == 4 # 1 + 3 (skips 2)
    assert vm_continue.globals.get("k") == 3

def test_bytecode_functions_and_recursion():
    source = """
    fn add(a, b) {
        return a + b
    }

    fn factorial(n) {
        if n <= 1 {
            return 1
        }
        return n * factorial(n - 1)
    }

    var sum = add(10, 20)
    var fact = factorial(5)
    """
    vm = compile_and_execute(source)
    assert vm.globals.get("sum") == 30
    assert vm.globals.get("fact") == 120

def test_bytecode_lists_and_indexing():
    source = """
    var lst = [10, 20, 30]
    var first = lst[0]
    lst[1] = 99
    var length = len(lst)
    append(lst, 40)
    var popped = pop(lst)
    """
    vm = compile_and_execute(source)
    assert vm.globals.get("first") == 10
    assert vm.globals.get("lst") == [10, 99, 30]
    assert vm.globals.get("length") == 3
    assert vm.globals.get("popped") == 40
