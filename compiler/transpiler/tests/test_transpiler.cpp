#include "../../lexer/lexer.hpp"
#include "../../parser/parser.hpp"
#include "../../interpreter/interpreter.hpp"
#include "../transpiler.hpp"

#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace wyrm;

namespace {

struct TestContext {
    int passed = 0;
    int failed = 0;

    void expect(bool condition, const std::string& label) {
        if (condition) {
            ++passed;
            std::cout << "PASS: " << label << "\n";
        } else {
            ++failed;
            std::cerr << "FAIL: " << label << "\n";
        }
    }
};

std::string read_file(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    std::ostringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

void write_file(const std::string& path, const std::string& content) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out << content;
}

int shell(const std::string& command) {
    return std::system(command.c_str());
}

// The upstream stdlib random module contains inline x86 cpuid assembly that
// this aarch64 host cannot assemble. Only the emitted scratch C is adapted;
// repository sources are never modified by the test.
void adapt_generated_c_for_host(std::string& code) {
    const std::string marker = "static inline void wyrm_cpuid(";
    const std::size_t begin = code.find(marker);
    if (begin == std::string::npos) return;
    const std::size_t end = code.find("\n}\n", begin);
    if (end == std::string::npos) return;

    const std::string replacement =
        "static inline void wyrm_cpuid(unsigned int leaf, unsigned int *eax, unsigned int *ebx, unsigned int *ecx, unsigned int *edx) {\n"
        "    (void)leaf;\n"
        "    *eax = *ebx = *ecx = *edx = 0;\n"
        "}\n";
    code.replace(begin, end + 3 - begin, replacement);
}

std::vector<ASTNodePtr> parse_source(const std::string& source) {
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    return parser.parse();
}

int run_interpreter_file(const std::string& source_path) {
    try {
        auto ast = parse_source(read_file(source_path));
        Interpreter interpreter;
        interpreter.interpret(ast);
    } catch (const std::exception& error) {
        std::cerr << error.what() << "\n";
        return 1;
    }
    return 0;
}

struct ProgramResult {
    int exit_code = 0;
    std::string output;
};

ProgramResult run_transpiled_program(const std::string& name,
                                     const std::string& source,
                                     const std::string& scratch,
                                     const std::string& repo_root,
                                     TestContext& context) {
    const std::string source_path = scratch + "/" + name + ".wyr";
    const std::string c_path = scratch + "/" + name + ".c";
    const std::string binary_path = scratch + "/" + name + "_bin";
    const std::string stdout_path = scratch + "/" + name + "_native.out";
    const std::string stderr_path = scratch + "/" + name + "_native.err";
    const std::string build_log = scratch + "/" + name + "_build.log";
    write_file(source_path, source);

    try {
        auto ast = parse_source(source);
        Transpiler transpiler(".");
        std::string generated = transpiler.transpile(ast);
        adapt_generated_c_for_host(generated);
        write_file(c_path, generated);
    } catch (const std::exception& error) {
        context.expect(false, name + ": transpilation (" + std::string(error.what()) + ")");
        return {};
    }

    const std::string compile_command =
        "gcc -O2 -std=c11 -DWYRM_NO_CURL -I\"" + repo_root + "/wyrm/lib\" \"" +
        c_path + "\" -lm -o \"" + binary_path + "\" > \"" + build_log + "\" 2>&1";
    if (shell(compile_command) != 0) {
        context.expect(false, name + ": generated C compiles (" + read_file(build_log).substr(0, 200) + ")");
        return {};
    }

    const std::string run_command = "\"" + binary_path + "\" > \"" + stdout_path + "\" 2> \"" + stderr_path + "\"";
    const int raw_status = shell(run_command);
    ProgramResult result;
    result.exit_code = raw_status;
    result.output = read_file(stdout_path);
    return result;
}

std::string run_interpreter_program(const std::string& self,
                                    const std::string& name,
                                    const std::string& source,
                                    const std::string& scratch) {
    const std::string source_path = scratch + "/" + name + ".wyr";
    write_file(source_path, source);
    const std::string stdout_path = scratch + "/" + name + "_interp.out";
    const std::string stderr_path = scratch + "/" + name + "_interp.err";
    shell("\"" + self + "\" --interpreter \"" + source_path + "\" > \"" + stdout_path + "\" 2> \"" + stderr_path + "\"");
    return read_file(stdout_path);
}

void check_program(const std::string& name,
                   const std::string& source,
                   const std::string& expected,
                   const std::string& self,
                   const std::string& scratch,
                   const std::string& repo_root,
                   TestContext& context) {
    const ProgramResult native = run_transpiled_program(name, source, scratch, repo_root, context);
    const std::string interpreted = run_interpreter_program(self, name, source, scratch);

    context.expect(native.exit_code == 0, name + ": native program exits successfully");
    context.expect(native.output == expected, name + ": native output matches expected");
    context.expect(interpreted == expected, name + ": interpreter output matches expected");
    context.expect(native.output == interpreted, name + ": native output matches interpreter output");
}

void check_number_literal(TestContext& context, double value, const std::string& label) {
    Token token(TokenType::FLOAT, "0", 1, 1);
    NumberNode node(token);
    node.value = value;

    Transpiler transpiler;
    const std::string expression = transpiler.evaluate(&node);
    const std::string prefix = "val_number(";
    bool shape_ok = expression.rfind(prefix, 0) == 0 && !expression.empty() && expression.back() == ')';
    if (!shape_ok) {
        context.expect(false, label + ": literal has val_number(...) shape");
        return;
    }
    const std::string literal = expression.substr(prefix.size(), expression.size() - prefix.size() - 1);
    char* parse_end = nullptr;
    const double round_trip = std::strtod(literal.c_str(), &parse_end);
    const bool complete = parse_end == literal.c_str() + literal.size();
    const bool identical = std::memcmp(&round_trip, &value, sizeof(double)) == 0;
    context.expect(complete && identical, label + ": emitted literal '" + literal + "' round-trips exactly");
}

} // namespace

int main(int argc, char** argv) {
    if (argc >= 3 && std::string(argv[1]) == "--interpreter") {
        return run_interpreter_file(argv[2]);
    }

    const std::string scratch = argc > 1 ? argv[1] : "wyrm_transpiler_test_scratch";
    const std::string repo_root = argc > 2 ? argv[2] : ".";
    const std::string self = argc > 0 ? argv[0] : "";
    std::error_code ec;
    fs::create_directories(scratch, ec);

    TestContext context;

    const std::string and_or_program =
        "fn mark() {\n"
        "    print(\"RHS_RAN\")\n"
        "    return 99\n"
        "}\n"
        "fn main() {\n"
        "    var a = false and mark()\n"
        "    var b = true and mark()\n"
        "    var c = true or mark()\n"
        "    var d = false or mark()\n"
        "    print(a)\n"
        "    print(b)\n"
        "    print(c)\n"
        "    print(d)\n"
        "}\n";
    const std::string and_or_expected =
        "RHS_RAN\nRHS_RAN\nfalse\n99\ntrue\n99\n";
    check_program("short_circuit_and_or", and_or_program, and_or_expected, self, scratch, repo_root, context);

    const std::string skipped_division_program =
        "fn main() {\n"
        "    var a = false and (1 / 0)\n"
        "    var b = true or (1 / 0)\n"
        "    print(a)\n"
        "    print(b)\n"
        "}\n";
    check_program("short_circuit_skips_division", skipped_division_program,
                  "false\ntrue\n", self, scratch, repo_root, context);

    const std::string nested_program =
        "fn mark() {\n"
        "    print(\"RHS_RAN\")\n"
        "    return 2\n"
        "}\n"
        "fn main() {\n"
        "    var x = false and (true and mark())\n"
        "    var y = true or (false or mark())\n"
        "    var z = false or (true and mark())\n"
        "    print(x)\n"
        "    print(y)\n"
        "    print(z)\n"
        "}\n";
    check_program("short_circuit_nested", nested_program,
                  "RHS_RAN\nfalse\ntrue\n2\n", self, scratch, repo_root, context);

    const std::string precision_program =
        "fn main() {\n"
        "    var a = 1.0000000000000002\n"
        "    var b = 1.0\n"
        "    print(a == b)\n"
        "    var tiny = 0.00000000000000000001\n"
        "    print(tiny > 0.0)\n"
        "    var precise = 123.456789012345\n"
        "    var coarse = 123.456789\n"
        "    print(precise == coarse)\n"
        "}\n";
    check_program("number_precision", precision_program,
                  "false\ntrue\nfalse\n", self, scratch, repo_root, context);

    check_number_literal(context, 1.0, "integer-valued double");
    check_number_literal(context, -0.0, "negative zero");
    check_number_literal(context, 0.1234567891234567, "high precision fraction");
    check_number_literal(context, 1e-20, "small magnitude");
    check_number_literal(context, std::numeric_limits<double>::denorm_min(), "smallest subnormal");
    check_number_literal(context, std::numeric_limits<double>::min(), "smallest normalized");
    check_number_literal(context, std::numeric_limits<double>::max(), "largest finite");

    std::cout << "\nTranspiler tests: " << context.passed << " passed, "
              << context.failed << " failed\n";
    return context.failed == 0 ? 0 : 1;
}
