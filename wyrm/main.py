#!/usr/bin/env python3
"""Wyrm Language Interpreter - Main Entry Point"""

import sys
import os
from .lexer import Lexer
from .parser import Parser
from .interpreter import Interpreter

def run_file(filepath):
    """Run a Wyrm source file."""
    if not os.path.exists(filepath):
        print(f"Error: File '{filepath}' not found.")
        sys.exit(1)

    with open(filepath, 'r', encoding='utf-8') as f:
        source = f.read()

    try:
        lexer = Lexer(source)
        tokens = lexer.tokenize()
        parser = Parser(tokens)
        ast = parser.parse()
        interpreter = Interpreter()
        interpreter.execute(ast)
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)

def run_repl():
    """Run the REPL (Read-Eval-Print Loop)."""
    print("Wyrm REPL - Type 'exit()' to quit")
    interpreter = Interpreter()

    while True:
        try:
            line = input("wyrm> ")
            if line.strip() == "exit()":
                break

            lexer = Lexer(line)
            tokens = lexer.tokenize()
            parser = Parser(tokens)
            ast = parser.parse()
            result = interpreter.interpret(ast)
            if result is not None:
                print(result)
        except KeyboardInterrupt:
            print("\nUse exit() to quit")
            continue
        except Exception as e:
            print(f"Error: {e}")

def main():
    if len(sys.argv) > 1:
        run_file(sys.argv[1])
    else:
        run_repl()

if __name__ == "__main__":
    main()