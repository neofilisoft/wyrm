#!/usr/bin/env python3
"""Wyrm Language Interpreter & Compiler - Main Entry Point"""

import sys
import os
from pathlib import Path
from .lexer import Lexer
from .parser import Parser
from .interpreter import Interpreter
from .bytecode import Compiler, VM, serialize_bytecode, deserialize_bytecode
from .llvm_codegen import LLVMCodeGen

def run_file(filepath):
    """Run a Wyrm source file or bytecode file."""
    path = Path(filepath)
    if not path.exists():
        print(f"Error: File '{filepath}' not found.", file=sys.stderr)
        sys.exit(1)

    source_dir = path.parent.resolve()

    if filepath.endswith('.wyb'):
        # Load .wyb and execute via bytecode VM
        try:
            with open(filepath, 'r', encoding='utf-8') as f:
                json_str = f.read()
            bytecode = deserialize_bytecode(json_str)
            vm = VM()
            vm.execute(bytecode)
        except Exception as e:
            print(f"Bytecode VM Execution Error: {e}", file=sys.stderr)
            sys.exit(1)
    else:
        # Execute source file via Interpreter
        try:
            with open(filepath, 'r', encoding='utf-8') as f:
                source = f.read()

            lexer = Lexer(source)
            tokens = lexer.tokenize()
            parser = Parser(tokens)
            ast = parser.parse()

            interpreter = Interpreter(source_dir=source_dir)
            interpreter.execute(ast)
        except Exception as e:
            print(f"Error: {e}", file=sys.stderr)
            sys.exit(1)

def build_file(filepath, backend="auto"):
    """Compile a Wyrm source file into a native executable using the LLVM or GCC backend."""
    path = Path(filepath)
    if not path.exists():
        print(f"Error: File '{filepath}' not found.", file=sys.stderr)
        sys.exit(1)

    if not filepath.endswith('.wyr'):
        print("Error: Can only compile '.wyr' source files.", file=sys.stderr)
        sys.exit(1)

    # Determine binary name based on platform
    base_path = filepath[:-4]
    ext = ".exe" if sys.platform == "win32" else ""
    output_bin = base_path + ext

    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            source = f.read()

        force_gcc = (backend == "gcc")
        codegen = LLVMCodeGen(force_gcc=force_gcc)

        # Check if requested backend is valid
        if backend == "llvm" and not codegen.has_llvm_toolchain():
            print("Warning: LLVM/clang backend requested but clang is not installed. Falling back to GCC.", file=sys.stderr)

        success = codegen.compile(source, output_bin, optimize=True)
        if not success:
            sys.exit(1)

        print(f"Successfully compiled '{filepath}' -> Native Binary '{output_bin}'")
    except Exception as e:
        print(f"Compilation Error: {e}", file=sys.stderr)
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
    # Parsing arguments manually to handle '--backend <name>' flag anywhere in args
    args = sys.argv[1:]
    backend = "auto"

    if "--backend" in args:
        idx = args.index("--backend")
        if idx + 1 < len(args):
            backend = args[idx + 1]
            # Remove the backend flag and its value from args
            args = args[:idx] + args[idx + 2:]
        else:
            print("Error: '--backend' requires a value (llvm or gcc).", file=sys.stderr)
            sys.exit(1)

    if len(args) > 0:
        arg1 = args[0]
        if arg1 == "build":
            if len(args) < 2:
                print("Error: Please specify the source file to build.", file=sys.stderr)
                sys.exit(1)
            build_file(args[1], backend=backend)
        elif arg1 == "run":
            if len(args) < 2:
                print("Error: Please specify the file to run.", file=sys.stderr)
                sys.exit(1)
            run_file(args[1])
        elif arg1 in ("--version", "-v", "version"):
            # Kept for standard package info CLI calls
            print("wyrm version 1.2")
        else:
            # Default to running the file
            run_file(arg1)
    else:
        run_repl()

if __name__ == "__main__":
    main()