#!/usr/bin/env python3
"""Wyrm Language Interpreter & Compiler - Main Entry Point"""

import sys
import os
from .lexer import Lexer
from .parser import Parser
from .interpreter import Interpreter
from .bytecode import Compiler, VM, serialize_bytecode, deserialize_bytecode

def run_file(filepath):
    """Run a Wyrm source file or bytecode file."""
    if not os.path.exists(filepath):
        print(f"Error: File '{filepath}' not found.")
        sys.exit(1)

    if filepath.endswith('.wyb'):
        # Load .wyb and execute via Interpreter (source is embedded)
        # The bytecode VM is still being developed - using Interpreter as backend
        try:
            import json
            with open(filepath, 'r', encoding='utf-8') as f:
                data = json.load(f)
            source = data.get('source', '')
            if not source:
                print(f"Error: .wyb file has no embedded source.")
                sys.exit(1)
            lexer = Lexer(source)
            tokens = lexer.tokenize()
            parser = Parser(tokens)
            ast = parser.parse()
            interpreter = Interpreter()
            interpreter.execute(ast)
        except Exception as e:
            print(f"Bytecode Execution Error: {e}")
            sys.exit(1)
    else:
        # Execute source file
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

def build_file(filepath):
    """Compile a Wyrm source file into a native executable (.exe) using GCC."""
    if not os.path.exists(filepath):
        print(f"Error: File '{filepath}' not found.")
        sys.exit(1)

    if not filepath.endswith('.wyr'):
        print("Error: Can only compile '.wyr' source files.")
        sys.exit(1)

    # Determine binary name
    base_path = filepath[:-4]
    c_temp_path = base_path + "_temp.c"
    output_exe = base_path + ".exe"

    with open(filepath, 'r', encoding='utf-8') as f:
        source = f.read()

    try:
        from .transpiler import transpile_to_c
        import subprocess

        # Transpile to C
        c_code = transpile_to_c(source)
        
        with open(c_temp_path, 'w', encoding='utf-8') as f:
            f.write(c_code)

        # Call GCC to compile to Native Binary
        cmd = ["gcc", c_temp_path, "-o", output_exe, "-std=c11", "-O2", "-lm"]
        result = subprocess.run(cmd, capture_output=True, text=True)

        # Clean up temp C file
        if os.path.exists(c_temp_path):
            os.remove(c_temp_path)

        if result.returncode != 0:
            print("C Compilation Error:")
            print(result.stderr)
            sys.exit(1)

        print(f"Successfully compiled '{filepath}' -> Native Binary '{output_exe}'")
    except Exception as e:
        print(f"Compilation Error: {e}")
        if os.path.exists(c_temp_path):
            os.remove(c_temp_path)
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
        arg1 = sys.argv[1]
        if arg1 == "build":
            if len(sys.argv) < 3:
                print("Error: Please specify the source file to build.")
                sys.exit(1)
            build_file(sys.argv[2])
        elif arg1 == "run":
            if len(sys.argv) < 3:
                print("Error: Please specify the file to run.")
                sys.exit(1)
            run_file(sys.argv[2])
        else:
            # Default to running the file
            run_file(arg1)
    else:
        run_repl()

if __name__ == "__main__":
    main()