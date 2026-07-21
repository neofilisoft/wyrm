# Wyrm Language v1.0.0

Wyrm (.wyr) is a C-inspired programming language initially designed for educational purposes and language experimentation, with ongoing development toward becoming a systems programming language.

## Features

- Function declarations with `fn`
- Boolean values: `true`/`false`
- Logical operators: `&&`, `||`, `!` (with aliases `and`, `or`)
- Equality: `!=`
- Conditional statements: `if/elif/else`
- Block syntax: `{ }`
- Loops: `repeat/til`
- Strings and arrays: `#`
- Comments: `//`, `/* */`, `///`
- Optional semicolons
- First-class functions with closures
- Built-in functions: `print`, `input`, `int`, `float`, `str`, `len`, `type`, `abs`, `min`, `max`, `round`, `pow`

## Examples

See the `examples/` directory for sample Wyrm programs:
- `hello.wyr` - Classic Hello World
- `basics.wyr` - Variables, data types, basic operations
- `control_flow.wyr` - If-else statements, loops
- `functions.wyr` - Function definitions, parameters, recursion
- `data_structures.wyr` - Arrays/lists, string manipulation

## Installation

```bash
# From PyPI (once published)
pip install wyrmlang

# From source
git clone https://github.com/yourusername/wyrm.git
cd wyrm
pip install -e .
```

## Usage

### Running a Wyrm program
```bash
wyrm examples/hello.wyr
# or
python -m wyrm examples/hello.wyr
```

### Using the REPL
```bash
wyrm
# or
python -m wyrm
```

### From Python code
```python
from wyrm import Lexer, Parser, Interpreter

code = '''
fn main() {
    print("Hello from Wyrm!")
}
'''

lexer = Lexer(code)
tokens = lexer.tokenize()
parser = Parser(tokens)
ast = parser.parse()
interpreter = Interpreter()
interpreter.execute(ast)
```

## Running Tests

```bash
# Install test dependencies
pip install -r requirements.txt

# Run unit tests
python -m pytest tests/ -v

# Run all examples
python test_examples.py
```

## Building the Package

```bash
python setup.py sdist bdist_wheel
```

## Language Specification

See `Docs.md` for the original Wyrm language specification.

## Roadmap to Systems Programming Language

While the current implementation is an interpreter in Python suitable for learning and experimentation, the long-term goal is to evolve Wyrm into a systems programming language. See `TODO_SYSTEM_LANGUAGE.md` for detailed plans regarding:

- Compiled implementation (targeting C/C++/Rust)
- Manual memory management (no garbage collection)
- Self-hosting compiler aspirations
- Low-level system access capabilities
- Performance targets matching or exceeding C/C++

## Color Scheme

The official Wyrm color for branding and CI badges is #3B2A4E (RGB: 59,42,78).

## License

MIT License - see `LICENSE` file for details.

## Copyright

Copyright (c) 2026 Neofilisoft