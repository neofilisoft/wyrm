import pytest
from wyrm.lexer import Lexer
from wyrm.parser import Parser
from wyrm.interpreter import Interpreter

def test_use_module():
    code = """
    use test_module_helper.wyr;

    fn main() {
        print(GREETING)
        res = add_nums(10, 20)
        print(res)
    }
    """
    tokens = Lexer(code).tokenize()
    ast = Parser(tokens).parse()
    interpreter = Interpreter()
    interpreter.interpret(ast)

def test_use_module_without_semicolon():
    code = """
    use test_module_helper.wyr

    fn main() {
        print(GREETING)
    }
    """
    tokens = Lexer(code).tokenize()
    ast = Parser(tokens).parse()
    interpreter = Interpreter()
    interpreter.interpret(ast)
