#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace {

int failures = 0;

void expect(bool condition, const std::string& label) {
    if (condition) {
        std::cout << "PASS: " << label << "\n";
    } else {
        std::cerr << "FAIL: " << label << "\n";
        ++failures;
    }
}

std::string read_file(const std::string& path) {
    std::ifstream in(path);
    std::ostringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

std::size_t count_of(const std::string& text, const std::string& needle) {
    std::size_t count = 0;
    std::size_t at = text.find(needle);
    while (at != std::string::npos) {
        ++count;
        at = text.find(needle, at + needle.size());
    }
    return count;
}

bool has_call_to(const std::string& text, const std::string& name) {
    return text.find("call void @" + name + "(") != std::string::npos ||
           text.find("call " + name + "(") != std::string::npos;
}

} // namespace

// Usage: test_llvm_short_circuit <generated.ll>
// The IR fixture is produced by the self-hosted compiler from a program whose
// short-circuit RHS calls @wyrm_fn_side. It is checked structurally because
// linking LLVM output against the C runtime is driver-dependent.
int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: test_llvm_short_circuit <generated.ll>\n";
        return 2;
    }

    const std::string ir = read_file(argv[1]);
    expect(!ir.empty(), "generated LLVM IR is readable");

    expect(count_of(ir, "br i1 %sc_cond_") >= 2,
           "and/or emit conditional branches on the left operand");
    expect(count_of(ir, "call i1 @llvm_val_to_bool(ptr %tmp_") >= 2,
           "left operands are tested with llvm_val_to_bool");
    expect(count_of(ir, "sc_rhs_") >= 2, "RHS blocks are emitted");
    expect(count_of(ir, "sc_short_") >= 2, "short-circuit result blocks are emitted");
    expect(count_of(ir, "sc_merge_") >= 2, "merge blocks are emitted");
    expect(count_of(ir, "br label %sc_merge_") >= 4,
           "both RHS and short-circuit blocks branch to the merge block");

    expect(!has_call_to(ir, "llvm_val_and"), "no llvm_val_and call remains in emitted IR");
    expect(!has_call_to(ir, "llvm_val_or"), "no llvm_val_or call remains in emitted IR");
    expect(ir.find(" = and i1 ") == std::string::npos &&
           ir.find(" = or i1 ") == std::string::npos,
           "no eager unboxed and/or instruction remains in emitted IR");

    bool inside_rhs = false;
    bool rhs_call_outside_block = false;
    std::size_t rhs_calls = 0;
    std::istringstream lines(ir);
    std::string line;
    while (std::getline(lines, line)) {
        if (!line.empty() && line[0] != ' ' && line[0] != ';' && line.back() == ':') {
            inside_rhs = line.rfind("sc_rhs_", 0) == 0;
        }
        if (line.find("call void @wyrm_fn_side(") != std::string::npos) {
            ++rhs_calls;
            if (!inside_rhs) rhs_call_outside_block = true;
        }
    }
    expect(rhs_calls >= 2, "program contains two RHS side-effect calls");
    expect(!rhs_call_outside_block, "every RHS side-effect call is inside an sc_rhs_ block");

    std::cout << "\nLLVM short-circuit structural tests: "
              << (failures == 0 ? 0 : failures) << " failure(s)\n";
    return failures == 0 ? 0 : 1;
}
