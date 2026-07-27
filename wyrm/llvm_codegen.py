"""
LLVM-based compiler pipeline for Wyrm.
Uses clang to transpile Wyrm via C, emit LLVM IR, optimize it, and compile to a native binary.
Falls back to GCC if clang/LLVM is not installed.
"""

import os
import sys
import subprocess
import shutil
from pathlib import Path
from .transpiler import transpile_to_c

class LLVMCodeGen:
    def __init__(self, force_gcc=False):
        self.force_gcc = force_gcc
        self.clang_path = shutil.which("clang")
        self.opt_path = shutil.which("opt")
        self.gcc_path = shutil.which("gcc")

    def has_llvm_toolchain(self) -> bool:
        """Check if clang is available on the system."""
        if self.force_gcc:
            return False
        return self.clang_path is not None

    def compile(self, source_code: str, output_path: str, optimize: bool = True) -> bool:
        """Compile Wyrm source code to a native binary using LLVM/clang or GCC fallback."""
        output_file = Path(output_path)
        base_name = output_file.stem
        dir_name = output_file.parent

        c_temp = dir_name / f"{base_name}_temp.c"
        ll_temp = dir_name / f"{base_name}_temp.ll"
        bc_temp = dir_name / f"{base_name}_temp.bc"

        # Transpile Wyrm to C
        try:
            c_code = transpile_to_c(source_code)
            c_temp.write_text(c_code, encoding="utf-8")
        except Exception as e:
            print(f"Transpilation Error: {e}", file=sys.stderr)
            return False

        success = False
        try:
            if self.has_llvm_toolchain():
                print(f"[LLVM Pipeline] Compiling C to LLVM IR using clang...")
                # 1. Compile C to LLVM assembly (.ll)
                cmd_ir = [
                    self.clang_path, "-S", "-emit-llvm", "-std=c11",
                    str(c_temp), "-o", str(ll_temp)
                ]
                res_ir = subprocess.run(cmd_ir, capture_output=True, text=True)
                if res_ir.returncode != 0:
                    print("Clang LLVM IR Generation Error:", file=sys.stderr)
                    print(res_ir.stderr, file=sys.stderr)
                    return False

                # 2. Optimize LLVM IR (using 'opt' if available, otherwise clang handles optimization flags)
                optimized_ir_file = ll_temp
                if optimize and self.opt_path:
                    print("[LLVM Pipeline] Optimizing LLVM bitcode using opt...")
                    cmd_opt = [
                        self.opt_path, "-O2", str(ll_temp), "-o", str(bc_temp)
                    ]
                    res_opt = subprocess.run(cmd_opt, capture_output=True, text=True)
                    if res_opt.returncode == 0:
                        # Convert bitcode back to readable .ll or compile directly from bitcode (.bc)
                        optimized_ir_file = bc_temp
                    else:
                        print("Warning: LLVM Optimization via 'opt' failed, compiling original IR.", file=sys.stderr)

                # 3. Assemble and compile IR/Bitcode to native binary
                print(f"[LLVM Pipeline] Compiling LLVM IR to native binary: {output_file.name}...")
                cmd_native = [
                    self.clang_path, str(optimized_ir_file), "-o", str(output_file), "-O2", "-lm"
                ]
                res_native = subprocess.run(cmd_native, capture_output=True, text=True)
                if res_native.returncode != 0:
                    print("Clang Compilation Error:", file=sys.stderr)
                    print(res_native.stderr, file=sys.stderr)
                    return False
                success = True
            else:
                # Fallback to standard GCC toolchain
                if not self.gcc_path:
                    print("Error: Neither LLVM/clang nor GCC compiler was found on your system.", file=sys.stderr)
                    return False

                print(f"[GCC Fallback] Compiling directly using GCC...")
                cmd_gcc = [
                    self.gcc_path, str(c_temp), "-o", str(output_file), "-std=c11", "-O2", "-lm"
                ]
                res_gcc = subprocess.run(cmd_gcc, capture_output=True, text=True)
                if res_gcc.returncode != 0:
                    print("GCC Compilation Error:", file=sys.stderr)
                    print(res_gcc.stderr, file=sys.stderr)
                    return False
                success = True

        finally:
            # Clean up all temporary files
            for temp_file in (c_temp, ll_temp, bc_temp):
                if temp_file.exists():
                    try:
                        temp_file.unlink()
                    except OSError:
                        pass

        return success
