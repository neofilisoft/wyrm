import pytest
from wyrm.lexer import Lexer, Token, TT_INT, TT_FLOAT, TT_STRING, TT_IDENTIFIER, TT_KEYWORD, TT_OPERATOR, TT_DELIMITER, TT_NEWLINE, TT_EOF

def test_basic_tokens():
    """Test basic tokenization."""
    code = """
    fn main() {
        x = 42
        y = 3.14
        s = "hello"
        b = true
        print(x)
    }
    """
    lexer = Lexer(code)
    tokens = lexer.tokenize()

    # Check that we got some tokens
    assert len(tokens) > 0
    # Check that we have an EOF token at the end
    assert tokens[-1].type == TT_EOF

def test_numbers():
    """Test number tokenization."""
    code = "x = 42\ny = 3.14\nz = 0"
    lexer = Lexer(code)
    tokens = lexer.tokenize()

    # Find the number tokens
    int_tokens = [t for t in tokens if t.type == TT_INT]
    float_tokens = [t for t in tokens if t.type == TT_FLOAT]

    assert len(int_tokens) >= 2  # 42 and 0
    assert len(float_tokens) >= 1  # 3.14

    # Check values
    int_values = [t.value for t in int_tokens]
    assert 42 in int_values
    assert 0 in int_values

    float_values = [t.value for t in float_tokens]
    assert 3.14 in float_values

def test_strings():
    """Test string tokenization."""
    code = 's = "hello"\nt = \'world\''
    lexer = Lexer(code)
    tokens = lexer.tokenize()

    string_tokens = [t for t in tokens if t.type == TT_STRING]
    assert len(string_tokens) >= 2

    string_values = [t.value for t in string_tokens]
    assert "hello" in string_values
    assert "world" in string_values

def test_keywords():
    """Test keyword tokenization."""
    code = "fn main() { if true else false }"
    lexer = Lexer(code)
    tokens = lexer.tokenize()

    keyword_tokens = [t for t in tokens if t.type == TT_KEYWORD]
    keyword_values = [t.value for t in keyword_tokens]

    assert "fn" in keyword_values
    assert "if" in keyword_values
    assert "else" in keyword_values
    assert "true" in keyword_values
    assert "false" in keyword_values

def test_operators():
    """Test operator tokenization."""
    code = "a = b + c * d / e - f"
    lexer = Lexer(code)
    tokens = lexer.tokenize()

    op_tokens = [t for t in tokens if t.type == TT_OPERATOR]
    op_values = [t.value for t in op_tokens]

    assert "=" in op_values
    assert "+" in op_values
    assert "*" in op_values
    assert "/" in op_values
    assert "-" in op_values

def test_delimiters():
    """Test delimiter tokenization."""
    code = "if (x > 0) { return x; }"
    lexer = Lexer(code)
    tokens = lexer.tokenize()

    delim_tokens = [t for t in tokens if t.type == TT_DELIMITER]
    delim_values = [t.value for t in delim_tokens]

    assert "(" in delim_values
    assert ")" in delim_values
    assert "{" in delim_values
    assert "}" in delim_values
    assert ";" in delim_values