#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cctype>
#include <filesystem>
#include "../../compiler/lexer/lexer.hpp"
#include "../../compiler/parser/parser.hpp"
#include "../../compiler/interpreter/interpreter.hpp"
#include "../../compiler/transpiler/transpiler.hpp"

#ifndef WYRMC_VERSION
#define WYRMC_VERSION "1.2.1"
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

int run_native_compiler(const std::string& filename, const std::string& output_override = "") {
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
    std::string output_exe = output_override;
    if (output_exe.empty()) {
        output_exe = "output.exe";
        size_t dot_wyr = filename.rfind(".wyr");
        if (dot_wyr != std::string::npos) {
            output_exe = filename.substr(0, dot_wyr) + ".exe";
        }
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
    // HTTP is optional. The default build must remain self-contained on
    // systems that have curl headers but do not provide the curl linker
    // library. Set WYRM_ENABLE_CURL=1 when libcurl is configured.
    std::string curl_flag = std::getenv("WYRM_ENABLE_CURL") ? "" : " -DWYRM_NO_CURL";
    std::string inc_flags = "-Iwyrm/lib -I\"" + lib_dir + "\" -D_CRT_SECURE_NO_WARNINGS -Wno-deprecated-declarations" + curl_flag;
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
        // Only files that Wyrm itself creates are eligible for removal.
        // Generic object/executable patterns are deliberately excluded so
        // artifacts produced by other toolchains are never touched.
        const auto remove_wyrm_file = [&](const fs::path& path) {
            std::error_code ec;
            if (fs::is_regular_file(path, ec) && fs::remove(path, ec)) {
                std::cout << "  Removed: " << path.filename().string() << std::endl;
                ++removed_count;
            }
        };

        // Intermediate C file emitted by run_native_compiler.
        remove_wyrm_file(fs::path("_wyrm_temp.c"));

        if (clean_all) {
            for (const auto& entry : fs::directory_iterator(".")) {
                std::error_code ec;
                if (!entry.is_regular_file(ec)) continue;
                const fs::path path = entry.path();
                if (path.extension() != ".exe") continue;
                // Remove an executable only when a Wyrm source with the same
                // base name exists in this directory.
                fs::path source = path;
                source.replace_extension(".wyr");
                if (fs::is_regular_file(source, ec)) {
                    remove_wyrm_file(path);
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

namespace {

constexpr std::string_view kWyrmIndentUnit = "    ";

struct FormatResult {
    bool ok = false;
    std::string text;
    std::string error;
};

// Re-indent code while treating string literals and comments as opaque regions.
// Braces inside those regions are copied byte-for-byte and never change the
// indentation level. Lexically malformed input is rejected so the original
// source is never overwritten with corrupted output.
FormatResult format_wyrm_source(const std::string& source) {
    std::string out;
    out.reserve(source.size() + source.size() / 8);

    int indent = 0;
    bool at_line_start = true;
    bool line_has_code = false;

    const auto emit_indent = [&]() {
        if (at_line_start) {
            for (int level = 0; level < indent; ++level) out += kWyrmIndentUnit;
            at_line_start = false;
        }
    };

    const auto emit_char = [&](char value) {
        emit_indent();
        out += value;
    };

    const auto finish_line = [&]() {
        if (at_line_start) return;
        while (!out.empty() && out.back() == ' ') out.pop_back();
        out += '\n';
        at_line_start = true;
        line_has_code = false;
    };

    const auto handle_newline = [&]() {
        if (at_line_start) {
            out += '\n';
        } else {
            finish_line();
        }
        line_has_code = false;
    };

    size_t pos = 0;

    // A brace already ends its output line; drop the matching source newline
    // so it is not emitted a second time as a blank line.
    const auto consume_newline_after_brace = [&]() {
        while (pos < source.size()) {
            const char value = source[pos];
            if (value == '\n') {
                ++pos;
                return;
            }
            if (value == ' ' || value == '\t' || value == '\r') {
                ++pos;
                continue;
            }
            break;
        }
    };

    while (pos < source.size()) {
        const char ch = source[pos];

        if (ch == '\r') {
            ++pos;
            continue;
        }
        if (ch == '\n') {
            handle_newline();
            ++pos;
            continue;
        }

        if (ch == ' ' || ch == '\t') {
            if (at_line_start) {
                ++pos;
                continue;
            }
            emit_char(' ');
            while (pos < source.size() &&
                   (source[pos] == ' ' || source[pos] == '\t')) {
                ++pos;
            }
            continue;
        }

        // In Wyrm, '//' starts a comment only before any code on the physical
        // line; elsewhere it is the floor-division operator.
        const bool line_comment = ch == '/' && pos + 1 < source.size() &&
                                  source[pos + 1] == '/' && !line_has_code;
        if (line_comment) {
            while (pos < source.size() && source[pos] != '\n' &&
                   source[pos] != '\r') {
                emit_char(source[pos]);
                ++pos;
            }
            continue;
        }

        if (ch == '/' && pos + 1 < source.size() && source[pos + 1] == '*') {
            const size_t close = source.find("*/", pos + 2);
            if (close == std::string::npos) {
                return {false, {}, "Unterminated block comment"};
            }
            emit_indent();
            out.append(source, pos, close + 2 - pos);
            pos = close + 2;
            continue;
        }

        if (ch == '"' || ch == '\'') {
            const char quote = ch;
            emit_char(quote);
            ++pos;
            bool closed = false;
            while (pos < source.size()) {
                const char inner = source[pos];
                emit_char(inner);
                ++pos;
                if (inner == '\\' && pos < source.size()) {
                    emit_char(source[pos]);
                    ++pos;
                    continue;
                }
                if (inner == quote) {
                    closed = true;
                    break;
                }
            }
            if (!closed) {
                return {false, {}, "Unterminated string literal"};
            }
            line_has_code = true;
            continue;
        }

        if (ch == '{') {
            while (!out.empty() && out.back() == ' ') out.pop_back();
            if (!at_line_start) emit_char(' ');
            emit_char('{');
            line_has_code = true;
            finish_line();
            ++indent;
            ++pos;
            consume_newline_after_brace();
            continue;
        }

        if (ch == '}') {
            if (indent == 0) {
                return {false, {}, "Unbalanced closing brace '}'"};
            }
            --indent;
            finish_line();
            emit_char('}');
            line_has_code = true;
            ++pos;

            // The parser expects these continuation keywords immediately after
            // the closing brace, so keep them on the same output line. Look
            // ahead past spacing and block comments, but leave pos untouched
            // so those comments are still emitted.
            size_t look = pos;
            while (look < source.size()) {
                if (source[look] == ' ' || source[look] == '\t') {
                    ++look;
                    continue;
                }
                if (source[look] == '/' && look + 1 < source.size() &&
                    source[look + 1] == '*') {
                    const size_t close = source.find("*/", look + 2);
                    if (close != std::string::npos) {
                        look = close + 2;
                        continue;
                    }
                }
                break;
            }
            size_t word_end = look;
            while (word_end < source.size() &&
                   (std::isalnum(static_cast<unsigned char>(source[word_end])) ||
                    source[word_end] == '_')) {
                ++word_end;
            }
            const std::string_view word(source.data() + look, word_end - look);
            const bool attached = word == "elif" || word == "else" ||
                                  word == "til" || word == "catch" ||
                                  word == "finally";
            if (attached) continue;

            finish_line();
            consume_newline_after_brace();
            continue;
        }

        emit_char(ch);
        line_has_code = true;
        ++pos;
    }

    if (indent != 0) {
        return {false, {}, "Unbalanced braces: missing closing '}'"};
    }
    if (!out.empty() && out.back() != '\n') out += '\n';
    return {true, std::move(out), {}};
}

} // namespace

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
                      << "  test [dir]        Run all test_*.wyr files, report PASS/FAIL\n"
                      << "  check <file.wyr>  Parse/lint only (no type checking or execution)\n"
                      << "  fmt <file.wyr>    Format source file in-place (writes <file>.bak)\n"
                      << "  clean [--all]     Remove Wyrm-owned temp files and cache\n"
                      << "  <file.wyr>        Run source file directly (default)\n\n"
                      << "Options:\n"
                      << "  --all, -a         With clean, also remove .exe files having a matching .wyr\n"
                      << "  --version, -v     Display compiler/VM version\n"
                      << "  --help, -h        Display help information\n\n"
                      << "This is the Stage 0 bootstrap compiler. The self-hosted 'wyrmc' built from\n"
                      << "compiler/wyrmc.wyr supports build/run/clean only; test, check and fmt are\n"
                      << "not available there.\n";
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

        if (arg1 == "test") {
            // Discover and run all test_*.wyr files
            std::string search_dir = (argc >= 3) ? argv[2] : ".";
            std::vector<std::string> test_files;
            try {
                for (const auto& e : fs::recursive_directory_iterator(search_dir)) {
                    if (!e.is_regular_file()) continue;
                    std::string n = e.path().filename().string();
                    if (n.size() > 4 && n.substr(0, 5) == "test_" && n.substr(n.size()-4) == ".wyr") {
                        test_files.push_back(e.path().string());
                    }
                }
            } catch (const std::exception& e) {
                std::cerr << "Error scanning '" << search_dir << "': " << e.what() << std::endl;
                return 1;
            }
            if (test_files.empty()) {
                std::cout << "[wyrmc " << WYRMC_VERSION << "] No test_*.wyr files found in '" << search_dir << "'" << std::endl;
                return 0;
            }
            int passed = 0, failed = 0;
            for (const auto& f : test_files) {
                std::vector<std::string> test_args = { f };
                // Capture stderr to detect errors
                std::string self_exe = argv[0];
                std::string cmd = "\"" + self_exe + "\" run \"" + f + "\" 2>wyrm_test_err.tmp";
                int r = std::system(cmd.c_str());
                std::string err_content;
                {
                    std::ifstream ef("wyrm_test_err.tmp");
                    if (ef.good()) {
                        std::stringstream buf; buf << ef.rdbuf();
                        err_content = buf.str();
                    }
                }
                std::error_code ec;
                fs::remove("wyrm_test_err.tmp", ec);
                if (r == 0 && err_content.empty()) {
                    std::cout << "  PASS  " << f << std::endl;
                    passed++;
                } else {
                    std::cout << "  FAIL  " << f;
                    if (!err_content.empty()) std::cout << "  ->  " << err_content.substr(0, 120);
                    std::cout << std::endl;
                    failed++;
                }
            }
            std::cout << "\n[wyrmc " << WYRMC_VERSION << "] Test results: "
                      << passed << " passed, " << failed << " failed, "
                      << (passed + failed) << " total" << std::endl;
            return (failed > 0) ? 1 : 0;
        }

        if (arg1 == "check") {
            if (argc < 3) {
                std::cerr << "Error: Please specify the source file to check." << std::endl;
                return 1;
            }
            std::string filename = argv[2];
            std::ifstream file(filename);
            if (!file.good()) {
                std::cerr << "Error: Cannot open '" << filename << "'" << std::endl;
                return 1;
            }
            std::stringstream buf; buf << file.rdbuf();
            std::string source = buf.str();
            try {
                wyrm::Lexer lexer(source);
                auto tokens = lexer.tokenize();
                wyrm::Parser parser(tokens);
                auto ast = parser.parse();
                std::cout << "[wyrmc " << WYRMC_VERSION << "] " << filename << " OK (" << ast.size() << " top-level statements)" << std::endl;
                return 0;
            } catch (const std::exception& e) {
                std::cerr << "SyntaxError in '" << filename << "': " << e.what() << std::endl;
                return 1;
            }
        }

        if (arg1 == "fmt") {
            if (argc < 3) {
                std::cerr << "Error: Please specify the source file to format." << std::endl;
                return 1;
            }
            const std::string filename = argv[2];
            std::ifstream in_file(filename, std::ios::binary);
            if (!in_file.good()) {
                std::cerr << "Error: Cannot open '" << filename << "'" << std::endl;
                return 1;
            }
            std::stringstream buf;
            buf << in_file.rdbuf();
            const std::string source = buf.str();
            in_file.close();

            const FormatResult result = format_wyrm_source(source);
            if (!result.ok) {
                std::cerr << "Error: Cannot format '" << filename << "': "
                          << result.error << ". File left unchanged." << std::endl;
                return 1;
            }

            const std::string backup = filename + ".bak";
            {
                std::ofstream bk(backup, std::ios::binary | std::ios::trunc);
                if (!bk.good()) {
                    std::cerr << "Error: Cannot create backup '" << backup
                              << "'. File left unchanged." << std::endl;
                    return 1;
                }
                bk << source;
                if (!bk.good()) {
                    std::cerr << "Error: Cannot write backup '" << backup
                              << "'. File left unchanged." << std::endl;
                    return 1;
                }
            }

            {
                std::ofstream out_file(filename, std::ios::binary | std::ios::trunc);
                if (!out_file.good()) {
                    std::cerr << "Error: Cannot open '" << filename
                              << "' for writing. File left unchanged." << std::endl;
                    return 1;
                }
                out_file << result.text;
                if (!out_file.good()) {
                    std::cerr << "Error: Cannot write '" << filename << "'." << std::endl;
                    return 1;
                }
            }

            std::cout << "[wyrmc " << WYRMC_VERSION << "] Formatted '" << filename
                      << "' (backup: " << backup << ")" << std::endl;
            return 0;
        }

        if (arg1 == "build") {
            if (argc < 3) {
                std::cerr << "Error: Please specify the source file to build." << std::endl;
                return 1;
            }
            std::string output_override;
            for (int i = 3; i + 1 < argc; ++i) {
                if (std::string(argv[i]) == "-o") {
                    output_override = argv[i + 1];
                    break;
                }
            }
            return run_native_compiler(argv[2], output_override);
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
