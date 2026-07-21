from unittest.mock import patch
from wyrm.lexer import Lexer
from wyrm.parser import Parser
from wyrm.interpreter import Interpreter

def test_input_builtin():
    code = """
    fn main() {
        var name = input("Your name: ")
        print("Hello, " + name)
    }
    """
    tokens = Lexer(code).tokenize()
    ast = Parser(tokens).parse()
    interpreter = Interpreter()

    with patch('builtins.input', return_value="WyrmUser"):
        import io
        from contextlib import redirect_stdout
        buf = io.StringIO()
        with redirect_stdout(buf):
            interpreter.interpret(ast)
        assert buf.getvalue().strip() == "Hello, WyrmUser"
