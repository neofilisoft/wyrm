# Wyrm C++20 Lexer Subsystem

The Lexer module represents the first stage of the native C++20 Wyrm bootstrap compiler frontend. It is responsible for converting a raw Wyrm source code string (`std::string_view`) into a sequential stream of strongly-typed `Token` structures.

## Module Structure

```
compiler/
├── token/
│   └── token.hpp         # Token structure and TokenType definitions
└── lexer/
    ├── lexer.hpp         # Lexer class declaration
    ├── lexer.cpp         # Tokenizer implementation
    └── tests/
        ├── test_lexer.cpp # Unit tests
        └── compile_tests.bat # Test compiler/runner script
```

## API Usage

```cpp
#include "compiler/lexer/lexer.hpp"
#include <iostream>

int main() {
    std::string_view source = "fn main() { print(\"Hello World\"); }";
    wyrm::Lexer lexer(source);
    
    try {
        std::vector<wyrm::Token> tokens = lexer.tokenize();
        for (const auto& token : tokens) {
            std::cout << token << std::endl;
        }
    } catch (const std::runtime_error& err) {
        std::cerr << "Lexical Error: " << err.what() << std::endl;
        return 1;
    }
    return 0;
}
```

## Token Specifications

The Lexer categorizes input character sequences into the following standard token types:

| TokenType | Example Content | Description |
|---|---|---|
| `INT` | `42`, `0` | Base-10 integer literals |
| `FLOAT` | `3.14`, `0.5` | Double-precision decimal floating-point literals |
| `STRING` | `"hello"`, `'world'` | Single/double quoted string literals supporting escape chars |
| `IDENTIFIER` | `my_var`, `funcName` | Standard identifiers containing alpha-numeric characters and underscores |
| `KEYWORD` | `fn`, `if`, `var`, `arena` | Language reserved keywords (28 keywords recognized) |
| `OPERATOR` | `+`, `&&`, `==`, `+=` | Arithmetic, relational, logical, assignment, and compound operators |
| `DELIMITER` | `(`, `}`, `;`, `.` | Syntax grouping and separation marks |
| `NEWLINE` | `\n` | Explicit line breaks |
| `END_OF_FILE` | (empty value) | Marks the end of tokenization stream |

## Lexing Behavior & Design Rules

1. **Greedy Operator Matching**:
   Operators are scanned using greedy matching up to 10 characters to correctly resolve compound operators (e.g. `==` before `=`, `&&` before `&`).
2. **Comment Isolation**:
   Single-line comments starting with `//` are skipped. In accordance with Wyrm syntax rules, `//` is only allowed at the beginning of a line (or preceded only by whitespace/newlines). Comments occurring mid-statement trigger a lexical `SyntaxError`.
3. **Floating-point Dot Ambiguity**:
   The compiler isolates floating-point numbers from member access dot operators. A period `.` is only parsed as part of a `FLOAT` literal if it is followed by a digit. If followed by an alphabetic char (like `buf.alloc`), the `.` is tokenized as a `DELIMITER` followed by an `IDENTIFIER`.
4. **Error Handling**:
   Malformed tokens (such as unknown symbols or unterminated string literals) result in throwing a `std::runtime_error` detailing the error line and column, ensuring compiler diagnostic accuracy.
