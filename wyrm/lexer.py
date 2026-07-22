import re
from typing import List, Tuple, Optional, Union

# Token types
TT_INT = 'INT'
TT_FLOAT = 'FLOAT'
TT_STRING = 'STRING'
TT_IDENTIFIER = 'IDENTIFIER'
TT_KEYWORD = 'KEYWORD'
TT_OPERATOR = 'OPERATOR'
TT_DELIMITER = 'DELIMITER'
TT_COMMENT = 'COMMENT'
TT_NEWLINE = 'NEWLINE'
TT_EOF = 'EOF'

# Keywords
KEYWORDS = {
    'fn', 'true', 'false', 'null',
    'if', 'elif', 'else',
    'repeat', 'til',
    'break', 'continue',
    'print', 'input',
    'int', 'float', 'str',
    'len', 'append', 'pop',
    'return',
    'var', 'dec', 'use'
}

# Operators - characters that can start an operator
OPERATORS = {'+', '-', '*', '/', '%', '=', '!', '<', '>', '&', '|'}

# Complete operators (for checking what we've built)
OPERATOR_KEYS = {
    '+', '-', '*', '/', '//', '%',
    '=', '+=', '-=', '*=', '/=', '%=',
    '==', '!=', '<', '>', '<=', '>=',
    '&', '&&', '|', '||',
    '!',
    'and', 'or', 'not'
}

# Delimiters
DELIMITERS = {
    '(', ')', '{', '}', '[', ']',
    ',', ':', ';', '.'
}

class Token:
    def __init__(self, type: str, value: Union[str, int, float], line: int, column: int):
        self.type = type
        self.value = value
        self.line = line
        self.column = column

    def __repr__(self):
        return f"Token({self.type}, {repr(self.value)}, line={self.line}, col={self.column})"

class Lexer:
    def __init__(self, text: str):
        self.text = text
        self.pos = 0
        self.line = 1
        self.column = 1
        self.current_char = self.text[0] if self.text else None

    def advance(self):
        """Advance to the next character."""
        if self.current_char == '\n':
            self.line += 1
            self.column = 1
        elif self.current_char is not None:
            self.column += 1

        self.pos += 1
        if self.pos >= len(self.text):
            self.current_char = None
        else:
            self.current_char = self.text[self.pos]

    def peek(self, n: int = 1) -> Optional[str]:
        """Look ahead n characters without consuming."""
        peek_pos = self.pos + n
        if peek_pos >= len(self.text):
            return None
        return self.text[peek_pos]

    def skip_whitespace(self):
        """Skip whitespace characters."""
        while self.current_char is not None and self.current_char.isspace():
            self.advance()

    def skip_comment(self):
        """Skip single-line comment."""
        while self.current_char is not None and self.current_char != '\n':
            self.advance()

    def read_number(self) -> Token:
        """Read a number (int or float)."""
        result = ''
        dot_count = 0

        while self.current_char is not None and (self.current_char.isdigit() or self.current_char == '.'):
            if self.current_char == '.':
                dot_count += 1
                if dot_count > 1:
                    break  # Only one decimal point allowed
            result += self.current_char
            self.advance()

        if dot_count == 1:
            return Token(TT_FLOAT, float(result), self.line, self.column - len(result))
        else:
            return Token(TT_INT, int(result), self.line, self.column - len(result))

    def read_string(self) -> Token:
        """Read a string literal."""
        quote_char = self.current_char  # Either " or '
        self.advance()  # Skip opening quote

        result = ''
        while self.current_char is not None and self.current_char != quote_char:
            if self.current_char == '\\':  # Handle escape sequences
                self.advance()
                if self.current_char == 'n':
                    result += '\n'
                elif self.current_char == 't':
                    result += '\t'
                elif self.current_char == 'r':
                    result += '\r'
                elif self.current_char == '\\':
                    result += '\\'
                elif self.current_char == quote_char:
                    result += quote_char
                else:
                    result += self.current_char  # Unknown escape, keep as-is
            else:
                result += self.current_char
            self.advance()

        if self.current_char == quote_char:
            self.advance()  # Skip closing quote
        return Token(TT_STRING, result, self.line, self.column - len(result) - 1)

    def read_identifier(self) -> Token:
        """Read an identifier or keyword."""
        result = ''
        while self.current_char is not None and (self.current_char.isalnum() or self.current_char == '_'):
            result += self.current_char
            self.advance()

        if result in KEYWORDS:
            return Token(TT_KEYWORD, result, self.line, self.column - len(result))
        else:
            return Token(TT_IDENTIFIER, result, self.line, self.column - len(result))

    def tokenize(self) -> List[Token]:
        """Convert input text into a list of tokens."""
        tokens = []

        while self.current_char is not None:
            if self.current_char.isspace():
                if self.current_char == '\n':
                    tokens.append(Token(TT_NEWLINE, '\n', self.line, self.column))
                self.advance()

            elif self.current_char.isdigit():
                tokens.append(self.read_number())

            elif self.current_char in ('"', "'"):
                tokens.append(self.read_string())

            elif self.current_char.isalpha() or self.current_char == '_':
                tokens.append(self.read_identifier())

            elif self.current_char == '/' and self.peek() == '/':
                # Single-line comment
                self.skip_comment()
                # Don't add comment token for now (can be added if needed)

            elif self.current_char in OPERATORS:
                # Find the longest matching operator starting at current position
                op = ""
                # Check progressively longer substrings
                for i in range(1, min(10, len(self.text) - self.pos + 1)):  # up to 10 chars or end of string
                    substring = self.text[self.pos:self.pos + i]
                    if substring in OPERATOR_KEYS:
                        op = substring  # keep the longest match found
                    else:
                        # If this substring isn't an operator, longer ones won't be either
                        break

                if op:  # we found a matching operator
                    # Record starting position
                    start_line = self.line
                    start_column = self.column
                    # Consume the operator characters
                    for _ in range(len(op)):
                        self.advance()
                    tokens.append(Token(TT_OPERATOR, op, start_line, start_column))
                else:
                    # Fallback: treat as single character (shouldn't happen)
                    start_line = self.line
                    start_column = self.column
                    self.advance()
                    tokens.append(Token(TT_OPERATOR, self.current_char, start_line, start_column))

            elif self.current_char in DELIMITERS:
                tokens.append(Token(TT_DELIMITER, self.current_char, self.line, self.column))
                self.advance()

            else:
                # Unknown character
                raise SyntaxError(f"Unknown character: {self.current_char} at line {self.line}, column {self.column}")

        tokens.append(Token(TT_EOF, None, self.line, self.column))
        return tokens