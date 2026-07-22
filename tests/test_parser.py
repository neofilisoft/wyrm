import pytest
from wyrm.lexer import Lexer
from wyrm.parser import Parser
from wyrm.ast import *

def test_simple_expression():
    """Test parsing a simple expression."""
    code = "x = 5 + 3"
    lexer = Lexer(code)
    tokens = lexer.tokenize()
    parser = Parser(tokens)
    statements = parser.parse()

    assert len(statements) == 1
    stmt = statements[0]
    assert isinstance(stmt, AssignmentNode)
    assert isinstance(stmt.var_name, IdentifierNode)
    assert stmt.var_name.name == "x"
    assert isinstance(stmt.value, BinaryOpNode)
    assert stmt.value.op.value == "+"
    assert isinstance(stmt.value.left, NumberNode)
    assert stmt.value.left.value == 5
    assert isinstance(stmt.value.right, NumberNode)
    assert stmt.value.right.value == 3

def test_string_expression():
    """Test parsing a string expression."""
    code = 'msg = "Hello, " + "World!"'
    lexer = Lexer(code)
    tokens = lexer.tokenize()
    parser = Parser(tokens)
    statements = parser.parse()

    assert len(statements) == 1
    stmt = statements[0]
    assert isinstance(stmt, AssignmentNode)
    assert isinstance(stmt.var_name, IdentifierNode)
    assert stmt.var_name.name == "msg"
    assert isinstance(stmt.value, BinaryOpNode)
    assert stmt.value.op.value == "+"
    assert isinstance(stmt.value.left, StringNode)
    assert stmt.value.left.value == "Hello, "
    assert isinstance(stmt.value.right, StringNode)
    assert stmt.value.right.value == "World!"

def test_print_statement():
    """Test parsing a print statement."""
    code = 'print("Hello World")'
    lexer = Lexer(code)
    tokens = lexer.tokenize()
    parser = Parser(tokens)
    statements = parser.parse()

    assert len(statements) == 1
    stmt = statements[0]
    assert isinstance(stmt, PrintNode)
    assert isinstance(stmt.expression, StringNode)
    assert stmt.expression.value == "Hello World"

def test_if_statement():
    """Test parsing an if statement."""
    code = """
    if x > 0 {
        print("positive")
    } else {
        print("zero or negative")
    }
    """
    lexer = Lexer(code)
    tokens = lexer.tokenize()
    parser = Parser(tokens)
    statements = parser.parse()

    assert len(statements) == 1
    stmt = statements[0]
    assert isinstance(stmt, IfNode)
    assert isinstance(stmt.condition, BinaryOpNode)
    assert stmt.condition.op.value == ">"
    assert len(stmt.then_body) == 1
    assert isinstance(stmt.then_body[0], PrintNode)
    assert stmt.else_body is not None
    assert len(stmt.else_body) == 1
    assert isinstance(stmt.else_body[0], PrintNode)

def test_repeat_statement():
    """Test parsing a repeat statement."""
    code = """
    i = 0
    repeat {
        print(i)
        i = i + 1
    } til (i >= 5)
    """
    lexer = Lexer(code)
    tokens = lexer.tokenize()
    parser = Parser(tokens)
    statements = parser.parse()

    assert len(statements) == 2  # assignment + repeat loop
    assert isinstance(statements[0], AssignmentNode)
    assert isinstance(statements[1], RepeatNode)
    repeat_stmt = statements[1]
    assert isinstance(repeat_stmt.condition, BinaryOpNode)
    assert repeat_stmt.condition.op.value == ">="
    assert len(repeat_stmt.body) == 2
    assert isinstance(repeat_stmt.body[0], PrintNode)
    assert isinstance(repeat_stmt.body[1], AssignmentNode)

def test_function_definition():
    """Test parsing a function definition."""
    code = """
    fn add(a, b) {
        return a + b
    }
    """
    lexer = Lexer(code)
    tokens = lexer.tokenize()
    parser = Parser(tokens)
    statements = parser.parse()

    assert len(statements) == 1
    stmt = statements[0]
    assert isinstance(stmt, FunctionDefNode)
    assert isinstance(stmt.name, IdentifierNode)
    assert stmt.name.name == "add"
    assert len(stmt.params) == 2
    assert isinstance(stmt.params[0], IdentifierNode)
    assert stmt.params[0].name == "a"
    assert isinstance(stmt.params[1], IdentifierNode)
    assert stmt.params[1].name == "b"
    assert len(stmt.body) == 1
    # Note: We don't have return statement implemented yet, so this would be an expression statement

def test_function_call():
    """Test parsing a function call."""
    code = "result = add(2, 3)"
    lexer = Lexer(code)
    tokens = lexer.tokenize()
    parser = Parser(tokens)
    statements = parser.parse()

    assert len(statements) == 1
    stmt = statements[0]
    assert isinstance(stmt, AssignmentNode)
    assert isinstance(stmt.var_name, IdentifierNode)
    assert stmt.var_name.name == "result"
    assert isinstance(stmt.value, FunctionCallNode)
    assert isinstance(stmt.value.name, IdentifierNode)
    assert stmt.value.name.name == "add"
    assert len(stmt.value.args) == 2
    assert isinstance(stmt.value.args[0], NumberNode)
    assert stmt.value.args[0].value == 2
    assert isinstance(stmt.value.args[1], NumberNode)
    assert stmt.value.args[1].value == 3

def test_list_literal():
    """Test parsing a list literal."""
    code = "numbers = [1, 2, 3, 4, 5]"
    lexer = Lexer(code)
    tokens = lexer.tokenize()
    parser = Parser(tokens)
    statements = parser.parse()

    assert len(statements) == 1
    stmt = statements[0]
    assert isinstance(stmt, AssignmentNode)
    assert isinstance(stmt.var_name, IdentifierNode)
    assert stmt.var_name.name == "numbers"
    assert isinstance(stmt.value, ListNode)
    assert len(stmt.value.elements) == 5
    assert all(isinstance(elem, NumberNode) for elem in stmt.value.elements)
    assert stmt.value.elements[0].value == 1
    assert stmt.value.elements[4].value == 5