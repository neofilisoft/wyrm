#!/usr/bin/env python3
"""Test script to run all example Wyrm programs."""

import os
import sys
from wyrm.lexer import Lexer
from wyrm.parser import Parser
from wyrm.interpreter import Interpreter

def run_example(filename):
    """Run a single Wyrm example file."""
    filepath = os.path.join("examples", filename)
    if not os.path.exists(filepath):
        print(f"Error: File '{filepath}' not found.")
        return False

    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            source = f.read()

        print(f"\n=== Running {filename} ===")
        print("Source code:")
        print(source)
        print("\nOutput:")

        lexer = Lexer(source)
        tokens = lexer.tokenize()
        parser = Parser(tokens)
        statements = parser.parse()

        interpreter = Interpreter()
        interpreter.execute(statements)

        print(f"=== {filename} completed successfully ===")
        return True

    except Exception as e:
        print(f"Error running {filename}: {e}")
        import traceback
        traceback.print_exc()
        return False

def main():
    """Run all example files."""
    examples = [
        "hello.wyr",
        "basics.wyr",
        "control_flow.wyr",
        "functions.wyr",
        "data_structures.wyr"
    ]

    success_count = 0
    for example in examples:
        if run_example(example):
            success_count += 1

    print(f"\n\nSummary: {success_count}/{len(examples)} examples ran successfully")

    if success_count == len(examples):
        print("All examples passed!")
        return 0
    else:
        print("Some examples failed.")
        return 1

if __name__ == "__main__":
    sys.exit(main())