import pytest
from wyrm.lexer import Lexer
from wyrm.parser import Parser
from wyrm.interpreter import Interpreter

def test_number_arithmetic():
    """Test basic arithmetic operations."""
    code = """
    x = 10
    y = 3
    sum = x + y
    diff = x - y
    prod = x * y
    quot = x / y
    rem = x % y
    """
    lexer = Lexer(code)
    tokens = lexer.tokenize()
    parser = Parser(tokens)
    statements = parser.parse()

    interpreter = Interpreter()
    interpreter.execute(statements)

    assert interpreter.global_env.get("x") == 10
    assert interpreter.global_env.get("y") == 3
    assert interpreter.global_env.get("sum") == 13
    assert interpreter.global_env.get("diff") == 7
    assert interpreter.global_env.get("prod") == 30
    assert interpreter.global_env.get("quot") == 10 / 3
    assert interpreter.global_env.get("rem") == 1

def test_string_operations():
    """Test string concatenation."""
    code = """
    a = "Hello"
    b = "World"
    c = a + " " + b + "!"
    """
    lexer = Lexer(code)
    tokens = lexer.tokenize()
    parser = Parser(tokens)
    statements = parser.parse()

    interpreter = Interpreter()
    interpreter.execute(statements)

    assert interpreter.global_env.get("a") == "Hello"
    assert interpreter.global_env.get("b") == "World"
    assert interpreter.global_env.get("c") == "Hello World!"

def test_boolean_operations():
    """Test boolean operations."""
    code = """
    a = true
    b = false
    c = a and b
    d = a or b
    e = not a
    """
    lexer = Lexer(code)
    tokens = lexer.tokenize()
    parser = Parser(tokens)
    statements = parser.parse()

    interpreter = Interpreter()
    interpreter.execute(statements)

    assert interpreter.global_env.get("a") == True
    assert interpreter.global_env.get("b") == False
    assert interpreter.global_env.get("c") == False
    assert interpreter.global_env.get("d") == True
    assert interpreter.global_env.get("e") == False

def test_comparison_operations():
    """Test comparison operations."""
    code = """
    x = 10
    y = 5
    eq = x == y
    ne = x != y
    lt = x < y
    gt = x > y
    le = x <= y
    ge = x >= y
    """
    lexer = Lexer(code)
    tokens = lexer.tokenize()
    parser = Parser(tokens)
    statements = parser.parse()

    interpreter = Interpreter()
    interpreter.execute(statements)

    assert interpreter.global_env.get("eq") == False
    assert interpreter.global_env.get("ne") == True
    assert interpreter.global_env.get("lt") == False
    assert interpreter.global_env.get("gt") == True
    assert interpreter.global_env.get("le") == False
    assert interpreter.global_env.get("ge") == True

def test_if_statement():
    """Test if statement execution."""
    code = """
    x = 10
    if x > 5 {
        result = "greater"
    } else {
        result = "less or equal"
    }
    """
    lexer = Lexer(code)
    tokens = lexer.tokenize()
    parser = Parser(tokens)
    statements = parser.parse()

    interpreter = Interpreter()
    interpreter.execute(statements)

    assert interpreter.global_env.get("result") == "greater"

def test_repeat_loop():
    """Test repeat loop execution."""
    code = """
    i = 0
    repeat {
        i = i + 1
    } til (i >= 3)
    """
    lexer = Lexer(code)
    tokens = lexer.tokenize()
    parser = Parser(tokens)
    statements = parser.parse()

    interpreter = Interpreter()
    interpreter.execute(statements)

    assert interpreter.global_env.get("i") == 3

def test_function_definition_and_call():
    """Test function definition and call."""
    code = """
    fn add(a, b) {
        return a + b
    }
    result = add(5, 3)
    """
    lexer = Lexer(code)
    tokens = lexer.tokenize()
    parser = Parser(tokens)
    statements = parser.parse()

    interpreter = Interpreter()
    interpreter.execute(statements)

    assert interpreter.global_env.get("result") == 8

def test_print_output(capsys):
    """Test print statement output."""
    code = 'print("Hello World")'
    lexer = Lexer(code)
    tokens = lexer.tokenize()
    parser = Parser(tokens)
    statements = parser.parse()

    interpreter = Interpreter()
    interpreter.execute(statements)

    captured = capsys.readouterr()
    assert captured.out.strip() == "Hello World"

def test_multi_arg_print_output(capsys):
    """Test multi-argument print statement output separated by spaces."""
    code = 'print("Numbers:", 1, 2, 3)'
    lexer = Lexer(code)
    tokens = lexer.tokenize()
    parser = Parser(tokens)
    statements = parser.parse()

    interpreter = Interpreter()
    interpreter.execute(statements)

    captured = capsys.readouterr()
    assert captured.out.strip() == "Numbers: 1 2 3"