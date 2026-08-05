#include "interpreter.hpp"
#include "../lexer/lexer.hpp"
#include "../parser/parser.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <cstdlib>

extern "C" {
#include "../../wyrm/lib/wyrm_str.h"
}

namespace wyrm {

// Implementation of Function::call
Value Function::call(const std::vector<Value>& args, Interpreter* interpreter) {
    if (args.size() != params.size()) {
        throw std::runtime_error("Function '" + name + "' expected " + 
                                 std::to_string(params.size()) + " arguments, got " + 
                                 std::to_string(args.size()));
    }

    auto env = std::make_shared<Environment>(closure.get());
    for (size_t i = 0; i < params.size(); ++i) {
        env->define(params[i], args[i]);
    }

    try {
        interpreter->execute_block(body, env);
    } catch (const ReturnSignal& sig) {
        return sig.value;
    }

    return val_null();
}

// Implementation of BuiltinFunction::call
Value BuiltinFunction::call(const std::vector<Value>& args, Interpreter* interpreter) {
    // For raw malloc/free, we need interpreter context to track allocations
    if (name == "malloc") {
        if (!interpreter->in_unsafe) {
            throw std::runtime_error("malloc() can only be called inside an unsafe block");
        }
        if (args.size() != 1) {
            throw std::runtime_error("malloc() expects exactly 1 argument");
        }
        Value ptr = val_raw_malloc(args[0]);
        if (ptr.type == VAL_RAW_PTR && ptr.as.raw_ptr) {
            interpreter->current_env->register_raw_allocation(ptr.as.raw_ptr);
        }
        return ptr;
    } else if (name == "free") {
        if (!interpreter->in_unsafe) {
            throw std::runtime_error("free() can only be called inside an unsafe block");
        }
        if (args.size() != 1) {
            throw std::runtime_error("free() expects exactly 1 argument");
        }
        if (args[0].type == VAL_RAW_PTR && args[0].as.raw_ptr) {
            interpreter->current_env->unregister_raw_allocation(args[0].as.raw_ptr);
        }
        return val_raw_free(args[0]);
    } else if (name == "realloc") {
        if (!interpreter->in_unsafe) {
            throw std::runtime_error("realloc() can only be called inside an unsafe block");
        }
        if (args.size() != 2) {
            throw std::runtime_error("realloc() expects exactly 2 arguments");
        }
        if (args[0].type == VAL_RAW_PTR && args[0].as.raw_ptr) {
            interpreter->current_env->unregister_raw_allocation(args[0].as.raw_ptr);
        }
        Value ptr = val_raw_realloc(args[0], args[1]);
        if (ptr.type == VAL_RAW_PTR && ptr.as.raw_ptr) {
            interpreter->current_env->register_raw_allocation(ptr.as.raw_ptr);
        }
        return ptr;
    }

    return func(args);
}

Interpreter::Interpreter(const std::string& src_dir, std::vector<std::string> args)
    : global_env(std::make_shared<Environment>()), current_env(global_env), source_dir(src_dir), in_unsafe(false), cli_args(std::move(args)) {
    setup_builtins();
}

Value Interpreter::evaluate(ASTNode* node) {
    if (!node) return val_null();
    node->accept(this);
    return last_value_;
}

void Interpreter::execute_statement(ASTNode* node) {
    if (!node) return;
    node->accept(this);
}

void Interpreter::execute(std::vector<ASTNodePtr>& statements) {
    for (auto& stmt : statements) {
        execute_statement(stmt.get());
    }
}

void Interpreter::execute_block(std::vector<ASTNodePtr>& statements, std::shared_ptr<Environment> env) {
    auto old_env = current_env;
    current_env = env;
    try {
        for (auto& stmt : statements) {
            execute_statement(stmt.get());
        }
    } catch (...) {
        current_env = old_env;
        throw;
    }
    current_env = old_env;
}

void Interpreter::execute_block(std::vector<ASTNode*>& statements, std::shared_ptr<Environment> env) {
    auto old_env = current_env;
    current_env = env;
    try {
        for (auto* stmt : statements) {
            execute_statement(stmt);
        }
    } catch (...) {
        current_env = old_env;
        throw;
    }
    current_env = old_env;
}

void Interpreter::interpret(std::vector<ASTNodePtr>& statements) {
    execute(statements);
    try {
        Value main_val = global_env->get("main");
        if (main_val.type == VAL_RAW_PTR && main_val.as.raw_ptr) {
            auto* func = static_cast<Callable*>(main_val.as.raw_ptr);
            if (func) {
                func->call({}, this);
            }
        }
    } catch (const std::runtime_error&) {
        // No main function defined; ignore.
    }
}

// Visitor implementation
void Interpreter::visit(NumberNode* node) {
    last_value_ = val_number(node->value);
}

void Interpreter::visit(StringNode* node) {
    last_value_ = val_string(node->value.c_str());
}

void Interpreter::visit(BooleanNode* node) {
    last_value_ = val_bool(node->value);
}

void Interpreter::visit(NoneNode* node) {
    (void)node;
    last_value_ = val_null();
}

void Interpreter::visit(IdentifierNode* node) {
    last_value_ = current_env->get(node->name);
}

void Interpreter::visit(BinaryOpNode* node) {
    std::string op = node->op.value;
    
    // Short-circuiting logical operations
    if (op == "and" || op == "&&") {
        Value left = evaluate(node->left.get());
        if (!val_to_bool(left)) {
            last_value_ = left;
        } else {
            last_value_ = evaluate(node->right.get());
        }
        return;
    }
    if (op == "or" || op == "||") {
        Value left = evaluate(node->left.get());
        if (val_to_bool(left)) {
            last_value_ = left;
        } else {
            last_value_ = evaluate(node->right.get());
        }
        return;
    }

    Value left = evaluate(node->left.get());
    Value right = evaluate(node->right.get());

    if (op == "+") {
        last_value_ = val_add(left, right);
    } else if (op == "-") {
        last_value_ = val_sub(left, right);
    } else if (op == "*") {
        last_value_ = val_mul(left, right);
    } else if (op == "/") {
        last_value_ = val_div(left, right);
    } else if (op == "%") {
        last_value_ = val_mod(left, right);
    } else if (op == "//") {
        last_value_ = val_floordiv(left, right);
    } else if (op == "==") {
        last_value_ = val_eq(left, right);
    } else if (op == "!=") {
        last_value_ = val_ne(left, right);
    } else if (op == "<") {
        last_value_ = val_lt(left, right);
    } else if (op == ">") {
        last_value_ = val_gt(left, right);
    } else if (op == "<=") {
        last_value_ = val_le(left, right);
    } else if (op == ">=") {
        last_value_ = val_ge(left, right);
    } else if (op == "**") {
        last_value_ = val_pow(left, right);
    } else {
        throw std::runtime_error("Unknown operator '" + op + "'");
    }
}

void Interpreter::visit(UnaryOpNode* node) {
    std::string op = node->op.value;
    Value val = evaluate(node->expr.get());

    if (op == "not" || op == "!") {
        last_value_ = val_not(val);
    } else if (op == "-") {
        last_value_ = val_sub(val_number(0.0), val);
    } else if (op == "+") {
        last_value_ = val;
    } else {
        throw std::runtime_error("Unknown unary operator '" + op + "'");
    }
}

void Interpreter::visit(AssignmentNode* node) {
    Value val = evaluate(node->value.get());
    if (node->is_declaration) {
        current_env->define(node->var_name->name, val, node->declaration_type == "dec", false);
    } else {
        current_env->set(node->var_name->name, val);
    }
    last_value_ = val;
}

void Interpreter::visit(PrintNode* node) {
    for (size_t i = 0; i < node->expressions.size(); ++i) {
        Value val = evaluate(node->expressions[i].get());
        char* s = val_to_str_ptr(val);
        std::cout << s;
        free(s);
        if (i + 1 < node->expressions.size()) {
            std::cout << " ";
        }
    }
    std::cout << std::endl;
    last_value_ = val_null();
}

void Interpreter::visit(IfNode* node) {
    Value cond = evaluate(node->condition.get());
    if (val_to_bool(cond)) {
        for (auto& stmt : node->then_body) {
            execute_statement(stmt.get());
        }
        return;
    }

    for (auto& clause : node->elif_clauses) {
        Value elif_cond = evaluate(clause.first.get());
        if (val_to_bool(elif_cond)) {
            for (auto& stmt : clause.second) {
                execute_statement(stmt.get());
            }
            return;
        }
    }

    for (auto& stmt : node->else_body) {
        execute_statement(stmt.get());
    }
}

void Interpreter::visit(RepeatNode* node) {
    while (true) {
        try {
            for (auto& stmt : node->body) {
                execute_statement(stmt.get());
            }
        } catch (const BreakSignal&) {
            break;
        } catch (const ContinueSignal&) {
            // fall through to condition check
        }

        Value cond = evaluate(node->condition.get());
        if (val_to_bool(cond)) {
            break;
        }
    }
}

void Interpreter::visit(FunctionDefNode* node) {
    std::vector<std::string> params;
    for (auto& p : node->params) {
        params.push_back(p->name);
    }

    std::vector<ASTNode*> body;
    for (auto& stmt : node->body) {
        body.push_back(stmt.get());
    }

    auto func = std::make_unique<Function>(node->name->name, params, body, current_env);
    Value func_val = val_raw_ptr(func.get());
    current_env->define(node->name->name, func_val, false, false);
    callables_alive_.push_back(std::move(func));
}

void Interpreter::visit(FunctionCallNode* node) {
    Value func_val = current_env->get(node->name->name);
    if (func_val.type != VAL_RAW_PTR || !func_val.as.raw_ptr) {
        throw std::runtime_error("'" + node->name->name + "' is not a callable function");
    }

    auto* callable = static_cast<Callable*>(func_val.as.raw_ptr);
    std::vector<Value> args;
    for (auto& arg : node->args) {
        args.push_back(evaluate(arg.get()));
    }

    last_value_ = callable->call(args, this);
}

void Interpreter::visit(ListNode* node) {
    Value arr = val_array_create((int)node->elements.size());
    for (size_t i = 0; i < node->elements.size(); ++i) {
        arr.as.array->data[i] = evaluate(node->elements[i].get());
    }
    last_value_ = arr;
}

void Interpreter::visit(IndexNode* node) {
    Value obj = evaluate(node->obj.get());
    auto* slice = dynamic_cast<SliceNode*>(node->index.get());
    if (slice) {
        Value start = slice->start ? evaluate(slice->start.get()) : val_null();
        Value end = slice->end ? evaluate(slice->end.get()) : val_null();
        last_value_ = val_array_slice(obj, start, end);
    } else {
        Value idx = evaluate(node->index.get());
        last_value_ = val_array_get(obj, idx);
    }
}

void Interpreter::visit(IndexAssignNode* node) {
    Value obj = evaluate(node->obj.get());
    Value idx = evaluate(node->index.get());
    Value val = evaluate(node->value.get());
    last_value_ = val_array_set(obj, idx, val);
}

void Interpreter::visit(ReturnNode* node) {
    Value val = node->expr ? evaluate(node->expr.get()) : val_null();
    throw ReturnSignal{val};
}

void Interpreter::visit(UseNode* node) {
    std::string raw_path = node->module_path;

    // Resolve search paths in priority order:
    // 1. Relative to source_dir
    // 2. Global installed packages (~/.wyrm/packages)
    // 3. Local packages folder (CWD/packages)
    std::vector<std::string> possible_paths;
    possible_paths.push_back(source_dir + "/" + raw_path);
    possible_paths.push_back(source_dir + "/" + raw_path + ".wyr");
    possible_paths.push_back(source_dir + "/packages/" + raw_path);
    possible_paths.push_back(source_dir + "/packages/" + raw_path + ".wyr");
    possible_paths.push_back(source_dir + "/packages/" + raw_path + "/mod.wyr");
    possible_paths.push_back(source_dir + "/packages/" + raw_path + "/main.wyr");

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

    // Save previous source dir
    std::string old_source_dir = source_dir;
    // Determine new source dir relative to target path
    size_t last_slash = target_path.find_last_of("/\\");
    if (last_slash != std::string::npos) {
        source_dir = target_path.substr(0, last_slash);
    } else {
        source_dir = ".";
    }

    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();

    // Execute in current environment
    for (auto& stmt : ast) {
        // Skip main function definition in imported modules
        auto* func_def = dynamic_cast<FunctionDefNode*>(stmt.get());
        if (func_def && func_def->name->name == "main") {
            continue;
        }
        execute_statement(stmt.get());
    }

    source_dir = old_source_dir;
    last_value_ = val_null();
}

void Interpreter::visit(BreakNode* node) {
    (void)node;
    throw BreakSignal{};
}

void Interpreter::visit(ContinueNode* node) {
    (void)node;
    throw ContinueSignal{};
}

void Interpreter::visit(SliceNode* node) {
    (void)node;
    // SliceNode visited directly is a syntax error
    throw std::runtime_error("Compilation Error: SliceNode can only be used directly as an index");
}

void Interpreter::visit(UnsafeBlockNode* node) {
    bool old_unsafe = in_unsafe;
    in_unsafe = true;
    
    auto sub_env = std::make_shared<Environment>(current_env.get());
    
    auto old_env = current_env;
    current_env = sub_env;
    try {
        for (auto& stmt : node->body) {
            execute_statement(stmt.get());
        }
    } catch (...) {
        current_env = old_env;
        in_unsafe = old_unsafe;
        throw;
    }
    current_env = old_env;
    in_unsafe = old_unsafe;
}

void Interpreter::visit(OwnedDeclNode* node) {
    Value val = evaluate(node->value.get());
    current_env->define(node->var_name->name, val, node->declaration_type == "dec", true);
    last_value_ = val;
}

void Interpreter::visit(ArenaNode* node) {
    Value size = evaluate(node->size_expr.get());
    if (size.type != VAL_NUMBER) {
        throw std::runtime_error("Arena size must be a number");
    }
    WyrmArena* arena = arena_create((size_t)size.as.number);
    current_env->define_arena(node->name, arena);
    last_value_ = val_null();
}

void Interpreter::visit(ArenaAllocNode* node) {
    WyrmArena* arena = current_env->get_arena(node->arena_name);
    Value size = evaluate(node->size_expr.get());
    if (size.type != VAL_NUMBER) {
        throw std::runtime_error("Arena allocation size must be a number");
    }
    last_value_ = val_raw_ptr(arena_alloc(arena, (size_t)size.as.number));
}

void Interpreter::visit(ArenaResetNode* node) {
    WyrmArena* arena = current_env->get_arena(node->arena_name);
    arena_reset(arena);
    last_value_ = val_null();
}

} // namespace wyrm
