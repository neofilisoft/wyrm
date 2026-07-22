# Wyrm Language 

[![License: MIT](https://img.shields.io/badge/License-MIT-333333.svg)](LICENSE)
[![Version](https://img.shields.io/badge/version-1.2.0-B10C1A)](https://github.com/neofilisoft/wyrm/releases)

Wyrm (.wyr) is a C-inspired programming language initially designed for educational purposes and language experimentation, with ongoing development toward becoming a systems programming language.

## Features

- Function declarations with `fn`
- Boolean values: `true`/`false`
- Logical operators: `&&`, `||`, `!` (with aliases `and`, `or`, `not`)
- Equality and comparison: `==`, `!=`, `<`, `>`, `<=`, `>=`
- Conditional statements: `if/elif/else`
- Block syntax: `{ }`
- Loops: `repeat/til`
- Arrays: `[1, 2, 3]` with subscript reading (`arr[0]`) and assignment (`arr[0] = 10`)
- Module import system: `use module.wyr` or `use module.wyr;`
- Package manager: `wyrpkg`
- Comments: `//`, `/* */`, `///`
- Semicolons: Optional
- First-class functions with closures
- Multi-argument print: `print("Numbers:", 1, 2, 3)` (space-separated output)
- Built-in functions: `print`, `input`, `int`, `float`, `str`, `len`, `type`, `abs`, `min`, `max`, `round`, `pow`

## Examples

See the `examples/` directory for sample Wyrm programs:
- `hello.wyr` - Classic Hello World
- `basics.wyr` - Variables, data types, basic operations
- `control_flow.wyr` - If-else statements, loops
- `functions.wyr` - Function definitions, parameters, recursion
- `data_structures.wyr` - Arrays/lists, indexing, array modifications

## Installation & Setup

```bash
# Clone the repository
git clone https://github.com/neofilisoft/wyrm.git
cd wyrm

# Set up virtual environment and dependencies
python -m venv venv
./venv/bin/pip install -r requirements.txt
```

## Usage

### Running a Wyrm program
Using the Python interpreter:
```bash
python -m wyrm examples/hello.wyr
```

Using the compiled C11 Wyrm Virtual Machine (`wyrmc.exe`):
```bash
./wyrmc.exe examples/hello.wyr
```

### Package Manager (`wyrpkg`)
```bash
# Install package
python wyrpkg.py install my_package

# List installed packages
python wyrpkg.py list
```

### Using the REPL
```bash
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
# Run unit tests
python -m pytest tests/ -v

# Run all example programs
python test_examples.py
```

## Building the C VM Compiler (`wyrmc`)

```bash
gcc wyrmc.c -o wyrmc.exe -std=c11 -O2
```

## Language Specification

See `Docs.md` for the updated Wyrm language specification.

## Roadmap to Systems Programming Language

Wyrm is designed to evolve into a native systems programming language with performance close to C/C++/Rust. The memory safety model is positioned as:

```
[Most safe]   Rust   - Borrow Checker + Ownership, no raw pointers by default
              Wyrm   - Ownership + RAII + Safe References + optional unsafe {}
[Less safe]   C++    - RAII + Smart Pointers, but raw pointers still accessible
```

Key design goals:
- **No Garbage Collector** - deterministic memory, suitable for real-time systems
- **Ownership semantics** - each value has exactly one owner
- **RAII** - resources freed automatically when leaving scope
- **`unsafe {}` block** - opt-in for low-level hardware access
- **Performance target**: >=90% of C for general workloads

See `TODO_SYSTEM_LANGUAGE.md` for the full development roadmap.

## License

MIT License - see `LICENSE` file for details.

## Copyright

Copyright (c) 2026 Neofilisoft
