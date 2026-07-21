#!/usr/bin/env python3
"""
Placeholder for the future Wyrm compiler (wyrmc).
This script demonstrates the intended interface for the wyrmc command.
"""

import sys
import os
from pathlib import Path

def show_version():
    """Show the compiler version."""
    version_file = Path(__file__).parent / "VERSION"
    if version_file.exists():
        version = version_file.read_text().strip()
        print(f"wyrmc {version}")
    else:
        print("wyrmc 1.0.0 (development)")

def show_help():
    """Show help information."""
    print("""Wyrm Compiler (wyrmc) - Placeholder Implementation

Usage:
  wyrmc [command] [options]

Commands:
  build   Compile a Wyrm source file to an executable
  run     Compile and run a Wyrm source file
  test    Run the test suite
  doc     Generate documentation from source
  version Show version information
  help    Show this help message

Examples:
  wyrmc build hello.wyr
  wyrmc run hello.wyr
  wyrmc --version

Note: This is a placeholder implementation. The actual Wyrm compiler
is planned to be a native executable written in a systems language.
""")

def main():
    if len(sys.argv) < 2:
        show_help()
        return

    command = sys.argv[1]

    if command == "version" or sys.argv[1] in ("--version", "-v"):
        show_version()
    elif command == "help" or sys.argv[1] in ("--help", "-h"):
        show_help()
    else:
        print(f"Error: Unknown command '{command}'")
        print("Run 'wyrmc help' for usage information.")
        sys.exit(1)

if __name__ == "__main__":
    main()