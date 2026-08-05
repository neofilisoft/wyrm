#include "transpiler.hpp"
#include "../lexer/lexer.hpp"
#include "../parser/parser.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>

namespace wyrm {

Transpiler::Transpiler(const std::string& src_dir)
    : in_function(false), has_main_def(false), in_unsafe(false), source_dir(src_dir) {}

std::string Transpiler::load_c_runtime() {
    std::string home_dir;
    if (const char* h = std::getenv("USERPROFILE")) {
        home_dir = h;
    } else if (const char* h = std::getenv("HOME")) {
        home_dir = h;
    }
    std::string lib_dir = home_dir + "/.wyrm/packages/wyrmlang/lib/";
    
    std::vector<std::string> files = {
        "wyrm_core.h", "wyrm_arena.h", "wyrm_str.h",
        "wyrm_core.c", "wyrm_arena.c", "wyrm_str.c"
    };

    std::string result;
    for (const auto& filename : files) {
        std::ifstream file(lib_dir + filename);
        if (!file.good()) {
            // Try fallback to workspace relative path if run during tests
            std::ifstream fallback_file("wyrm/lib/" + filename);
            if (fallback_file.good()) {
                file = std::move(fallback_file);
            } else {
                throw std::runtime_error("Transpiler Error: Cannot find C runtime file '" + filename + "'");
            }
        }

        std::string line;
        while (std::getline(file, line)) {
            // Strip include guards and local includes
            size_t idx = line.find_first_not_of(" \t");
            if (idx != std::string::npos) {
                std::string stripped = line.substr(idx);
                if (stripped.rfind("#include \"wyrm_", 0) == 0) continue;
                if (stripped.rfind("#ifndef WYRM_", 0) == 0) continue;
                if (stripped.rfind("#define WYRM_", 0) == 0) continue;
                if (stripped.rfind("#endif // WYRM_", 0) == 0) continue;
            }
            result += line + "\n";
        }
    }
    return result;
}

void Transpiler::push_scope() {
    scopes.push_back(TranspilerScope{});
}

std::string Transpiler::pop_scope() {
    if (scopes.empty()) return "";
    auto scope = scopes.back();
    scopes.pop_back();

    std::string cleanup;
    for (const auto& arena : scope.arenas) {
        cleanup += "arena_destroy(wyrm_arena_" + arena + ");\n";
    }
    for (const auto& alloc : scope.allocations) {
        cleanup += "wyrm_var_" + alloc + " = val_raw_free(wyrm_var_" + alloc + ");\n";
    }
    return cleanup;
}

std::string Transpiler::get_all_cleanups() {
    std::string cleanup;
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        for (const auto& arena : it->arenas) {
            cleanup += "arena_destroy(wyrm_arena_" + arena + ");\n";
        }
        for (const auto& alloc : it->allocations) {
            cleanup += "wyrm_var_" + alloc + " = val_raw_free(wyrm_var_" + alloc + ");\n";
        }
    }
    return cleanup;
}

void Transpiler::emit(const std::string& stmt) {
    if (in_function) {
        function_lines.push_back("    " + stmt);
    } else {
        top_level_lines.push_back("    " + stmt);
    }
}

void Transpiler::visit_statement(ASTNode* stmt) {
    std::string res = evaluate(stmt);
    if (!res.empty()) {
        if (dynamic_cast<FunctionCallNode*>(stmt) || 
            dynamic_cast<ArenaAllocNode*>(stmt) || 
            dynamic_cast<ArenaResetNode*>(stmt)) {
            emit(res + ";");
        }
    }
}

std::string Transpiler::evaluate(ASTNode* node) {
    if (!node) return "";
    node->accept(this);
    return last_result_;
}

std::string Transpiler::transpile(std::vector<ASTNodePtr>& ast) {
    header_lines.push_back(load_c_runtime());

    // Check if main function is defined in Wyrm
    for (const auto& stmt : ast) {
        auto* fn_def = dynamic_cast<FunctionDefNode*>(stmt.get());
        if (fn_def && fn_def->name->name == "main") {
            has_main_def = true;
        }
    }

    header_lines.push_back("\n// Forward declarations of user functions");
    for (const auto& stmt : ast) {
        auto* fn_def = dynamic_cast<FunctionDefNode*>(stmt.get());
        if (fn_def) {
            if (fn_def->name->name == "main") continue;
            std::string params_str;
            for (size_t i = 0; i < fn_def->params.size(); ++i) {
                params_str += "Value wyrm_var_" + fn_def->params[i]->name;
                if (i + 1 < fn_def->params.size()) params_str += ", ";
            }
            header_lines.push_back("Value wyrm_fn_" + fn_def->name->name + "(" + params_str + ");");
        }
    }

    header_lines.push_back("\n// Global variable declarations");

    for (const auto& stmt : ast) {
        if (has_main_def) {
            auto* call = dynamic_cast<FunctionCallNode*>(stmt.get());
            if (call) {
                auto* id = dynamic_cast<IdentifierNode*>(call->name.get());
                if (id && id->name == "main") {
                    continue;
                }
            }
        }
        visit_statement(stmt.get());
    }

    std::vector<std::string> final_code;
    final_code.insert(final_code.end(), header_lines.begin(), header_lines.end());
    final_code.push_back("\n// Function implementations");
    final_code.insert(final_code.end(), function_lines.begin(), function_lines.end());

    if (has_main_def) {
        final_code.push_back("\nint main(int argc, char *argv[]) {");
        final_code.push_back("    val_init_sys_args(argc, argv);");
        final_code.insert(final_code.end(), top_level_lines.begin(), top_level_lines.end());
        final_code.push_back("    wyrm_fn_main();");
        final_code.push_back("    return 0;");
        final_code.push_back("}");
    } else {
        final_code.push_back("\nint main(int argc, char *argv[]) {");
        final_code.push_back("    val_init_sys_args(argc, argv);");
        final_code.insert(final_code.end(), top_level_lines.begin(), top_level_lines.end());
        final_code.push_back("    return 0;");
        final_code.push_back("}");
    }

    std::string output;
    for (const auto& line : final_code) {
        output += line + "\n";
    }
    return output;
}

void Transpiler::visit(NumberNode* node) {
    std::string val_str = std::to_string(node->value);
    // Remove trailing zeros for clean output
    val_str.erase(val_str.find_last_not_of('0') + 1, std::string::npos);
    if (val_str.back() == '.') {
        val_str.pop_back();
    }
    last_result_ = "val_number(" + val_str + ")";
}

void Transpiler::visit(StringNode* node) {
    std::string escaped;
    for (char c : node->value) {
        if (c == '"') escaped += "\\\"";
        else if (c == '\\') escaped += "\\\\";
        else if (c == '\n') escaped += "\\n";
        else if (c == '\t') escaped += "\\t";
        else if (c == '\r') escaped += "\\r";
        else escaped += c;
    }
    last_result_ = "val_string(\"" + escaped + "\")";
}

void Transpiler::visit(BooleanNode* node) {
    last_result_ = std::string("val_bool(") + (node->value ? "true" : "false") + ")";
}

void Transpiler::visit(NoneNode* node) {
    (void)node;
    last_result_ = "val_null()";
}

void Transpiler::visit(IdentifierNode* node) {
    last_result_ = "wyrm_var_" + node->name;
}

void Transpiler::visit(BinaryOpNode* node) {
    std::string left = evaluate(node->left.get());
    std::string right = evaluate(node->right.get());
    std::string op = node->op.value;

    std::string c_func;
    if (op == "+") c_func = "val_add";
    else if (op == "-") c_func = "val_sub";
    else if (op == "*") c_func = "val_mul";
    else if (op == "/") c_func = "val_div";
    else if (op == "//") c_func = "val_floordiv";
    else if (op == "%") c_func = "val_mod";
    else if (op == "==") c_func = "val_eq";
    else if (op == "!=") c_func = "val_ne";
    else if (op == "<") c_func = "val_lt";
    else if (op == ">") c_func = "val_gt";
    else if (op == "<=") c_func = "val_le";
    else if (op == ">=") c_func = "val_ge";
    else if (op == "and" || op == "&&") c_func = "val_and";
    else if (op == "or" || op == "||") c_func = "val_or";
    else if (op == "**") c_func = "val_pow";
    else throw std::runtime_error("Unknown operator '" + op + "'");

    last_result_ = c_func + "(" + left + ", " + right + ")";
}

void Transpiler::visit(UnaryOpNode* node) {
    std::string val = evaluate(node->expr.get());
    std::string op = node->op.value;
    if (op == "-") {
        last_result_ = "val_sub(val_number(0.0), " + val + ")";
    } else if (op == "!" || op == "not") {
        last_result_ = "val_not(" + val + ")";
    } else {
        throw std::runtime_error("Unknown unary operator '" + op + "'");
    }
}

void Transpiler::visit(AssignmentNode* node) {
    std::string var_name = node->var_name->name;
    std::string val_expr = evaluate(node->value.get());
    std::string c_var = "wyrm_var_" + var_name;

    bool is_malloc = false;
    auto* call = dynamic_cast<FunctionCallNode*>(node->value.get());
    if (call && call->name->name == "malloc") {
        is_malloc = true;
    }

    if (in_function) {
        if (constants.count(var_name)) {
            throw std::runtime_error("Compilation Error: Cannot assign to constant '" + var_name + "'");
        }
        if (node->is_declaration) {
            if (node->declaration_type == "dec") {
                constants.insert(var_name);
            }
            declared_locals.insert(var_name);
            emit("Value " + c_var + " = " + val_expr + ";");
            if (is_malloc && !scopes.empty()) {
                scopes.back().allocations.push_back(var_name);
            }
        } else {
            if (!declared_locals.count(var_name)) {
                declared_locals.insert(var_name);
                emit("Value " + c_var + " = " + val_expr + ";");
                if (is_malloc && !scopes.empty()) {
                    scopes.back().allocations.push_back(var_name);
                }
            } else {
                emit(c_var + " = " + val_expr + ";");
            }
        }
    } else {
        if (constants.count(var_name)) {
            throw std::runtime_error("Compilation Error: Cannot assign to constant '" + var_name + "'");
        }
        if (!declared_globals.count(var_name)) {
            declared_globals.insert(var_name);
            header_lines.push_back("Value " + c_var + ";");
        }
        if (node->is_declaration && node->declaration_type == "dec") {
            constants.insert(var_name);
        }
        emit(c_var + " = " + val_expr + ";");
    }
    last_result_ = c_var;
}

void Transpiler::visit(IndexAssignNode* node) {
    std::string obj = evaluate(node->obj.get());
    std::string index = evaluate(node->index.get());
    std::string value = evaluate(node->value.get());
    emit("val_array_set(" + obj + ", " + index + ", " + value + ");");
    last_result_ = "";
}

void Transpiler::visit(PrintNode* node) {
    std::string args_str;
    for (size_t i = 0; i < node->expressions.size(); ++i) {
        args_str += evaluate(node->expressions[i].get());
        if (i + 1 < node->expressions.size()) args_str += ", ";
    }
    emit("val_print(" + std::to_string(node->expressions.size()) + (args_str.empty() ? "" : ", " + args_str) + ");");
    last_result_ = "";
}

void Transpiler::visit(IfNode* node) {
    std::string cond = evaluate(node->condition.get());
    emit("if (val_to_bool(" + cond + ")) {");
    push_scope();
    for (auto& stmt : node->then_body) {
        visit_statement(stmt.get());
    }
    std::string cleanup = pop_scope();
    if (!cleanup.empty()) {
        std::stringstream ss(cleanup);
        std::string line;
        while (std::getline(ss, line)) {
            if (!line.empty()) emit(line);
        }
    }
    emit("}");

    for (auto& clause : node->elif_clauses) {
        std::string elif_cond = evaluate(clause.first.get());
        emit("else if (val_to_bool(" + elif_cond + ")) {");
        push_scope();
        for (auto& stmt : clause.second) {
            visit_statement(stmt.get());
        }
        cleanup = pop_scope();
        if (!cleanup.empty()) {
            std::stringstream ss(cleanup);
            std::string line;
            while (std::getline(ss, line)) {
                if (!line.empty()) emit(line);
            }
        }
        emit("}");
    }

    if (!node->else_body.empty()) {
        emit("else {");
        push_scope();
        for (auto& stmt : node->else_body) {
            visit_statement(stmt.get());
        }
        cleanup = pop_scope();
        if (!cleanup.empty()) {
            std::stringstream ss(cleanup);
            std::string line;
            while (std::getline(ss, line)) {
                if (!line.empty()) emit(line);
            }
        }
        emit("}");
    }
    last_result_ = "";
}

void Transpiler::visit(RepeatNode* node) {
    emit("do {");
    push_scope();
    for (auto& stmt : node->body) {
        visit_statement(stmt.get());
    }
    std::string cleanup = pop_scope();
    if (!cleanup.empty()) {
        std::stringstream ss(cleanup);
        std::string line;
        while (std::getline(ss, line)) {
            if (!line.empty()) emit(line);
        }
    }
    std::string cond = evaluate(node->condition.get());
    emit("} while (!val_to_bool(" + cond + "));");
    last_result_ = "";
}

void Transpiler::visit(FunctionDefNode* node) {
    in_function = true;
    std::string func_name = node->name->name;
    std::string c_func_name = (func_name == "main") ? "wyrm_fn_main" : "wyrm_fn_" + func_name;
    
    std::string params_str;
    for (size_t i = 0; i < node->params.size(); ++i) {
        params_str += "Value wyrm_var_" + node->params[i]->name;
        if (i + 1 < node->params.size()) params_str += ", ";
    }

    auto old_locals = declared_locals;
    declared_locals.clear();
    for (auto& p : node->params) {
        declared_locals.insert(p->name);
    }

    auto old_scopes = scopes;
    scopes.clear();

    function_lines.push_back("\nValue " + c_func_name + "(" + params_str + ") {");
    push_scope();

    for (auto& stmt : node->body) {
        visit_statement(stmt.get());
    }

    std::string cleanup = pop_scope();
    if (!cleanup.empty()) {
        std::stringstream ss(cleanup);
        std::string line;
        while (std::getline(ss, line)) {
            if (!line.empty()) function_lines.push_back("    " + line);
        }
    }

    function_lines.push_back("    return val_null();");
    function_lines.push_back("}");

    scopes = old_scopes;
    declared_locals = old_locals;
    in_function = false;
    last_result_ = "";
}

void Transpiler::visit(FunctionCallNode* node) {
    std::string func_name = node->name->name;

    if (func_name == "malloc" || func_name == "free" || func_name == "realloc") {
        if (!in_unsafe) {
            throw std::runtime_error("Compilation Error: Raw memory operations (malloc, free, realloc) are only allowed inside unsafe blocks");
        }
    }

    std::string args_str;
    for (size_t i = 0; i < node->args.size(); ++i) {
        args_str += evaluate(node->args[i].get());
        if (i + 1 < node->args.size()) args_str += ", ";
    }

    static const std::unordered_set<std::string> builtins = {
        "input", "len", "type", "int", "float", "str", "abs", "round", "pow",
        "append", "pop", "malloc", "free", "realloc", "sys_args", "read_file", "write_file", "exit", "system", "getenv",
        "split", "join", "trim", "upper", "lower", "contains", "replace",
        "starts_with", "ends_with", "char_at", "ord_val", "chr_val", "to_bytes", "from_bytes"
    };

    if (builtins.count(func_name)) {
        if (func_name == "input") {
            std::string prompt_arg = node->args.empty() ? "val_string(\"\")" : evaluate(node->args[0].get());
            last_result_ = "val_input(" + prompt_arg + ")";
        } else if (func_name == "free") {
            if (node->args.size() == 1 && dynamic_cast<IdentifierNode*>(node->args[0].get())) {
                std::string c_var = "wyrm_var_" + dynamic_cast<IdentifierNode*>(node->args[0].get())->name;
                last_result_ = c_var + " = val_raw_free(" + c_var + ")";
            } else {
                last_result_ = "val_raw_free(" + evaluate(node->args[0].get()) + ")";
            }
        } else {
            std::string c_fn = "val_" + func_name;
            if (func_name == "malloc") c_fn = "val_raw_malloc";
            else if (func_name == "realloc") c_fn = "val_raw_realloc";
            else if (func_name == "append") c_fn = "val_array_append";
            else if (func_name == "pop") c_fn = "val_array_pop";
            last_result_ = c_fn + "(" + args_str + ")";
        }
    } else {
        std::string c_fn = (func_name == "main") ? "wyrm_fn_main" : "wyrm_fn_" + func_name;
        last_result_ = c_fn + "(" + args_str + ")";
    }
}

void Transpiler::visit(ListNode* node) {
    std::string args;
    for (size_t i = 0; i < node->elements.size(); ++i) {
        args += evaluate(node->elements[i].get());
        if (i + 1 < node->elements.size()) args += ", ";
    }
    last_result_ = "val_array_init(" + std::to_string(node->elements.size()) + (args.empty() ? "" : ", " + args) + ")";
}

void Transpiler::visit(IndexNode* node) {
    std::string obj = evaluate(node->obj.get());
    auto* slice = dynamic_cast<SliceNode*>(node->index.get());
    if (slice) {
        std::string start = slice->start ? evaluate(slice->start.get()) : "val_null()";
        std::string end = slice->end ? evaluate(slice->end.get()) : "val_null()";
        last_result_ = "val_array_slice(" + obj + ", " + start + ", " + end + ")";
    } else {
        std::string idx = evaluate(node->index.get());
        last_result_ = "val_array_get(" + obj + ", " + idx + ")";
    }
}

void Transpiler::visit(SliceNode* node) {
    (void)node;
    throw std::runtime_error("Compilation Error: SliceNode can only be used directly as an index");
}

void Transpiler::visit(ReturnNode* node) {
    std::string expr = node->expr ? evaluate(node->expr.get()) : "val_null()";
    std::string cleanups = get_all_cleanups();
    if (!cleanups.empty()) {
        std::stringstream ss(cleanups);
        std::string line;
        while (std::getline(ss, line)) {
            if (!line.empty()) emit(line);
        }
    }
    emit("return " + expr + ";");
    last_result_ = "";
}

void Transpiler::visit(UseNode* node) {
    std::string raw_path = node->module_path;
    std::vector<std::string> possible_paths;
    possible_paths.push_back(raw_path);
    possible_paths.push_back(raw_path + ".wyr");
    possible_paths.push_back("packages/" + raw_path);
    possible_paths.push_back("packages/" + raw_path + ".wyr");
    possible_paths.push_back("packages/" + raw_path + "/mod.wyr");
    possible_paths.push_back("packages/" + raw_path + "/main.wyr");

    std::string home_dir;
    if (const char* h = std::getenv("USERPROFILE")) {
        home_dir = h;
    } else if (const char* h = std::getenv("HOME")) {
        home_dir = h;
    }
    if (!home_dir.empty()) {
        std::string global_pkg = home_dir + "/.wyrm/packages/";
        possible_paths.push_back(global_pkg + raw_path);
        possible_paths.push_back(global_pkg + raw_path + ".wyr");
        possible_paths.push_back(global_pkg + raw_path + "/mod.wyr");
        possible_paths.push_back(global_pkg + raw_path + "/main.wyr");
    }

    std::string target_path;
    for (const auto& path : possible_paths) {
        std::ifstream f(path);
        if (f.good()) {
            target_path = path;
            break;
        }
    }

    if (target_path.empty()) {
        throw std::runtime_error("Compilation Error: Cannot find module '" + raw_path + "'");
    }

    std::ifstream f(target_path);
    std::stringstream buffer;
    buffer << f.rdbuf();
    std::string source = buffer.str();

    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();

    header_lines.push_back("\n// --- Module: " + raw_path + " ---");
    for (auto& stmt : ast) {
        auto* func_def = dynamic_cast<FunctionDefNode*>(stmt.get());
        if (func_def) {
            if (func_def->name->name == "main") continue;
            std::string params_str;
            for (size_t i = 0; i < func_def->params.size(); ++i) {
                params_str += "Value wyrm_var_" + func_def->params[i]->name;
                if (i + 1 < func_def->params.size()) params_str += ", ";
            }
            header_lines.push_back("Value wyrm_fn_" + func_def->name->name + "(" + params_str + ");");
        }
        visit_statement(stmt.get());
    }
    header_lines.push_back("// --- End of Module: " + raw_path + " ---\n");
    last_result_ = "";
}

void Transpiler::visit(BreakNode* node) {
    (void)node;
    std::string cleanups = get_all_cleanups();
    if (!cleanups.empty()) {
        std::stringstream ss(cleanups);
        std::string line;
        while (std::getline(ss, line)) {
            if (!line.empty()) emit(line);
        }
    }
    emit("break;");
    last_result_ = "";
}

void Transpiler::visit(ContinueNode* node) {
    (void)node;
    std::string cleanups = get_all_cleanups();
    if (!cleanups.empty()) {
        std::stringstream ss(cleanups);
        std::string line;
        while (std::getline(ss, line)) {
            if (!line.empty()) emit(line);
        }
    }
    emit("continue;");
    last_result_ = "";
}

void Transpiler::visit(UnsafeBlockNode* node) {
    emit("/* unsafe block begin */");
    auto old_locals = declared_locals;
    bool old_unsafe = in_unsafe;
    in_unsafe = true;
    
    push_scope();
    for (auto& stmt : node->body) {
        visit_statement(stmt.get());
    }
    std::string cleanup = pop_scope();
    if (!cleanup.empty()) {
        std::stringstream ss(cleanup);
        std::string line;
        while (std::getline(ss, line)) {
            if (!line.empty()) emit(line);
        }
    }
    in_unsafe = old_unsafe;
    declared_locals = old_locals;
    emit("/* unsafe block end */");
    last_result_ = "";
}

void Transpiler::visit(OwnedDeclNode* node) {
    std::string var_name = node->var_name->name;
    std::string val_expr = evaluate(node->value.get());
    std::string c_var = "wyrm_var_" + var_name;
    bool is_const = (node->declaration_type == "dec");

    if (in_function) {
        if (!declared_locals.count(var_name)) {
            declared_locals.insert(var_name);
            emit("Value " + c_var + " = " + val_expr + ";  /* owned */");
        } else {
            emit(c_var + " = " + val_expr + ";  /* owned reassign */");
        }
    } else {
        if (!declared_globals.count(var_name)) {
            declared_globals.insert(var_name);
            header_lines.push_back("Value " + c_var + ";  /* owned global */");
        }
        emit(c_var + " = " + val_expr + ";");
    }

    if (is_const) {
        constants.insert(var_name);
    }
    last_result_ = c_var;
}

void Transpiler::visit(ArenaNode* node) {
    std::string size_expr = evaluate(node->size_expr.get());
    std::string c_var = "wyrm_arena_" + node->name;
    emit("WyrmArena *" + c_var + " = arena_create((size_t)(" + size_expr + ").as.number);");
    if (!scopes.empty()) {
        scopes.back().arenas.push_back(node->name);
    }
    last_result_ = "";
}

void Transpiler::visit(ArenaAllocNode* node) {
    std::string size_expr = evaluate(node->size_expr.get());
    std::string c_var = "wyrm_arena_" + node->arena_name;
    last_result_ = "val_raw_ptr(arena_alloc(" + c_var + ", (size_t)(" + size_expr + ").as.number))";
}

void Transpiler::visit(ArenaResetNode* node) {
    std::string c_var = "wyrm_arena_" + node->arena_name;
    last_result_ = "val_arena_reset(" + c_var + ")";
}

} // namespace wyrm
