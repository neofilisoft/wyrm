import pytest
from wyrm.lexer import Lexer
from wyrm.parser import Parser
from wyrm.interpreter import Interpreter

def test_if_elif_else_control_flow():
    code = """
    fn main() {
        x = 7
        if x > 10 {
            print("big")
        } elif x > 5 {
            print("medium")
        } else {
            print("small")
        }
    }
    """
    tokens = Lexer(code).tokenize()
    ast = Parser(tokens).parse()
    interpreter = Interpreter()
    # Should not raise RecursionError
    interpreter.interpret(ast)

def test_repeat_til_loop_control_flow():
    code = """
    fn main() {
        i = 0
        repeat {
            print(i)
            i = i + 1
        } til (i == 5)
    }
    """
    tokens = Lexer(code).tokenize()
    ast = Parser(tokens).parse()
    interpreter = Interpreter()
    # Should not raise RecursionError
    interpreter.interpret(ast)
