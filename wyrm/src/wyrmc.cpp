#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <filesystem>
#include "../../compiler/lexer/lexer.hpp"
#include "../../compiler/parser/parser.hpp"
#include "../../compiler/interpreter/interpreter.hpp"
#include "../../compiler/transpiler/transpiler.hpp"

#ifndef WYRMC_VERSION
#define WYRMC_VERSION "1.1.0"
#endif

namespace fs = std::filesystem;
using namespace wyrm;

int run_native_interpreter(const std::string& filename, const std::vector<std::string>& cli_args) {
    std::ifstream file(filename);
    if (!file.good()) {
        std::cerr << "Error: Could not open file '" << filename << "'" << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    // Determine directory of source file for relative 'use' statements
    std::string source_dir = ".";
    size_t last_slash = filename.find_last_of("/\\");
    if (last_slash != std::string::npos) {
        source_dir = filename.substr(0, last_slash);
    }

    try {
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        Parser parser(tokens);
        auto ast = parser.parse();

        Interpreter interpreter(source_dir, cli_args);
        interpreter.interpret(ast);
    } catch (const std::exception& e) {
        std::cerr << "Runtime Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

int run_native_compiler(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.good()) {
        std::cerr << "Error: Could not open file '" << filename << "'" << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    // Determine directory of source file for relative 'use' statements
    std::string source_dir = ".";
    size_t last_slash = filename.find_last_of("/\\");
    if (last_slash != std::string::npos) {
        source_dir = filename.substr(0, last_slash);
    }

    std::string c_code;
    try {
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        Parser parser(tokens);
        auto ast = parser.parse();

        Transpiler transpiler(source_dir);
        c_code = transpiler.transpile(ast);
    } catch (const std::exception& e) {
        std::cerr << "Transpilation Error: " << e.what() << std::endl;
        return 1;
    }

    // Determine output executable name
    std::string output_exe = "output.exe";
    size_t dot_wyr = filename.rfind(".wyr");
    if (dot_wyr != std::string::npos) {
        output_exe = filename.substr(0, dot_wyr) + ".exe";
    }

    // Write temporary C file
    std::string temp_c_file = "_wyrm_temp.c";
    std::ofstream out(temp_c_file);
    if (!out.good()) {
        std::cerr << "Error: Could not create temporary C file" << std::endl;
        return 1;
    }
    out << c_code;
    out.close();

    // Compile with gcc
    std::string home_dir;
    if (const char* h = std::getenv("USERPROFILE")) {
        home_dir = h;
    } else if (const char* h = std::getenv("HOME")) {
        home_dir = h;
    }
    std::string lib_dir = home_dir + "/.wyrm/packages/wyrmlang/lib";
    std::string inc_flags = "-Iwyrm/lib -I\"" + lib_dir + "\" -D_CRT_SECURE_NO_WARNINGS -Wno-deprecated-declarations";
    std::string build_cmd = "gcc -O2 -std=c11 " + inc_flags + " \"" + temp_c_file + "\" -o \"" + output_exe + "\"";
    int compile_res = std::system(build_cmd.c_str());

    // Clean up temp file
    // std::remove(temp_c_file.c_str());

    if (compile_res != 0) {
        std::cerr << "Compilation Error: gcc failed to compile the generated C code" << std::endl;
        return 1;
    }

    std::cout << "[wyrmc " << WYRMC_VERSION << "] Successfully compiled '" << filename << "' -> Native Binary '" << output_exe << "'" << std::endl;
    return 0;
}

int run_clean(bool clean_all) {
    int removed_count = 0;
    try {
        for (const auto& entry : fs::directory_iterator(".")) {
            if (!entry.is_regular_file()) continue;
            std::string name = entry.path().filename().string();
            bool should_remove = false;

            if (name == "_wyrm_temp.c") {
                should_remove = true;
            } else if (name.size() > 8 && name.substr(name.size() - 8) == "_temp.ll") {
                should_remove = true;
            } else if (name.size() > 8 && name.substr(name.size() - 8) == "_run.exe") {
                should_remove = true;
            } else if (name.size() > 2 && name.substr(name.size() - 2) == ".o") {
                should_remove = true;
            } else if (name.size() > 4 && name.substr(name.size() - 4) == ".obj") {
                should_remove = true;
            } else if (clean_all && name.size() > 4 && name.substr(name.size() - 4) == ".exe") {
                std::string base = name.substr(0, name.size() - 4);
                if (fs::exists(base + ".wyr") || name == "output.exe" || name == "a.exe") {
                    should_remove = true;
                }
            }

            if (should_remove) {
                std::error_code ec;
                if (fs::remove(entry.path(), ec)) {
                    std::cout << "  Removed: " << name << std::endl;
                    removed_count++;
                }
            }
        }

        if (fs::exists(".wyrm_cache") && fs::is_directory(".wyrm_cache")) {
            std::error_code ec;
            std::uintmax_t n = fs::remove_all(".wyrm_cache", ec);
            if (n > 0) {
                std::cout << "  Removed .wyrm_cache directory (" << n << " items)" << std::endl;
                removed_count += static_cast<int>(n);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Clean error: " << e.what() << std::endl;
        return 1;
    }

    if (removed_count == 0) {
        std::cout << "[wyrmc " << WYRMC_VERSION << "] Already clean. No temporary files found." << std::endl;
    } else {
        std::cout << "[wyrmc " << WYRMC_VERSION << "] Cleaned " << removed_count << " temporary build file(s)." << std::endl;
    }
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::string arg1 = argv[1];
        if (arg1 == "--version" || arg1 == "-v" || arg1 == "version") {
            std::cout << "wyrmc " << WYRMC_VERSION << std::endl;
            return 0;
        }
        if (arg1 == "--help" || arg1 == "-h" || arg1 == "help") {
            std::cout << "wyrmc " << WYRMC_VERSION << "\n\n"
                      << "Usage:\n"
                      << "  wyrmc [command] [options] <file.wyr>\n\n"
                      << "Commands:\n"
                      << "  build <file.wyr>  Compile source file to native binary\n"
                      << "  run <file.wyr>    Run source file (native C++ interpreter)\n"
                      << "  clean [--all]     Clean temporary build artifacts and cache\n"
                      << "  <file.wyr>        Run source file directly (default)\n\n"
                      << "Options:\n"
                      << "  --all, -a         Used with clean to remove compiled binaries (.exe)\n"
                      << "  --version, -v     Display compiler/VM version\n"
                      << "  --help, -h        Display help information\n";
            return 0;
        }

        if (arg1 == "clean") {
            bool clean_all = false;
            for (int i = 2; i < argc; ++i) {
                std::string flag = argv[i];
                if (flag == "--all" || flag == "-a") {
                    clean_all = true;
                }
            }
            return run_clean(clean_all);
        }

        if (arg1 == "build") {
            if (argc < 3) {
                std::cerr << "Error: Please specify the source file to build." << std::endl;
                return 1;
            }
            return run_native_compiler(argv[2]);
        } else if (arg1 == "run") {
            if (argc < 3) {
                std::cerr << "Error: Please specify the file to run." << std::endl;
                return 1;
            }
            std::vector<std::string> cli_args;
            for (int i = 2; i < argc; ++i) {
                cli_args.push_back(argv[i]);
            }
            return run_native_interpreter(argv[2], cli_args);
        } else {
            std::vector<std::string> cli_args;
            for (int i = 1; i < argc; ++i) {
                cli_args.push_back(argv[i]);
            }
            return run_native_interpreter(argv[1], cli_args);
        }
    }

    std::cout << "wyrmc " << WYRMC_VERSION << "\n"
              << "Use 'wyrmc --help' for usage." << std::endl;
    return 0;
}
