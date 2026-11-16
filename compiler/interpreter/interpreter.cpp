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
#include "../../wyrm/lib/stdlib/wyrm_std_json.h"
#include "../../wyrm/lib/stdlib/wyrm_std_collections.h"
}
#include "stdlib_setup.hpp"

namespace wyrm {

// Implementation of Function::call
Value Function::call(const std::vector<Value>& args, Interpreter* interpreter) {
    if (interpreter->call_depth_ >= Interpreter::MAX_CALL_DEPTH) {
        throw std::runtime_error(
            "RuntimeError: Maximum recursion depth exceeded in function '" + name +
            "' (limit: " + std::to_string(Interpreter::MAX_CALL_DEPTH) + ")"
        );
    }

    if (args.size() != params.size()) {
        throw std::runtime_error("Function '" + name + "' expected " + 
                                 std::to_string(params.size()) + " arguments, got " + 
                                 std::to_string(args.size()));
    }

    auto env = std::make_shared<Environment>(closure.get());
    for (size_t i = 0; i < params.size(); ++i) {
        env->define(params[i], args[i]);
    }

    interpreter->call_depth_++;
    try {
        interpreter->execute_block(body, env);
    } catch (const ReturnSignal& sig) {
        interpreter->call_depth_--;
        return sig.value;
    } catch (...) {
        interpreter->call_depth_--;
        throw;
    }
    interpreter->call_depth_--;

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

    // Look up 'main' without silently swallowing unrelated errors
    Value main_val;
    bool has_main = false;
    try {
        main_val = global_env->get("main");
        has_main = true;
    } catch (const std::runtime_error& e) {
        std::string msg = e.what();
        if (msg.find("Undefined variable: 'main'") == std::string::npos) {
            throw; // re-throw: not a missing-main error
        }
        // No main function defined: that is allowed, just skip.
    }

    if (has_main && main_val.type == VAL_RAW_PTR && main_val.as.raw_ptr) {
        auto* func = static_cast<Callable*>(main_val.as.raw_ptr);
        func->call({}, this);
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
        if (idx.type == VAL_STRING) {
            if (json_is_object(obj)) {
                last_value_ = json_get(obj, idx);
                return;
            } else if (obj.type == VAL_RAW_PTR && obj.as.raw_ptr) {
                last_value_ = map_get(obj, idx);
                return;
            }
        }
        last_value_ = val_array_get(obj, idx);
    }
}

void Interpreter::visit(IndexAssignNode* node) {
    Value obj = evaluate(node->obj.get());
    Value idx = evaluate(node->index.get());
    Value val = evaluate(node->value.get());
    if (idx.type == VAL_STRING) {
        if (json_is_object(obj)) {
            last_value_ = json_set(obj, idx, val);
            return;
        } else if (obj.type == VAL_RAW_PTR && obj.as.raw_ptr) {
            last_value_ = map_set(obj, idx, val);
            return;
        }
    }
    last_value_ = val_array_set(obj, idx, val);
}

void Interpreter::visit(ReturnNode* node) {
    Value val = val_null();
    if (node->expr) {
        val = evaluate(node->expr.get());
        if (auto* id = dynamic_cast<IdentifierNode*>(node->expr.get())) {
            if (current_env->is_owned(id->name)) {
                current_env->mark_moved(id->name);
            } else {
                val = val_copy(val);
            }
        }
    }
    throw ReturnSignal{val};
}

void Interpreter::visit(UseNode* node) {
    std::string raw_path = node->module_path;

    // Check if it's a built-in standard library module
    if (stdlib::try_register(raw_path, *current_env)) {
        return;
    }

    // Resolve search paths in priority order:
    // 1. Relative to source_dir
    // 2. Global installed packages (~/.wyrm/packages)
    // 3. Local packages folder (CWD/packages)
    std::string dot_path = raw_path;
    for (char& c : dot_path) {
        if (c == '.') c = '/';
    }

    std::vector<std::string> possible_paths;
    auto add_candidates = [&](const std::string& prefix, const std::string& p) {
        possible_paths.push_back(prefix + p);
        possible_paths.push_back(prefix + p + ".wyr");
        possible_paths.push_back(prefix + p + "/mod.wyr");
        possible_paths.push_back(prefix + p + "/main.wyr");
    };

    add_candidates(source_dir + "/", raw_path);
    if (dot_path != raw_path) add_candidates(source_dir + "/", dot_path);
    add_candidates(source_dir + "/library/", raw_path);
    if (dot_path != raw_path) add_candidates(source_dir + "/library/", dot_path);
    add_candidates(source_dir + "/packages/", raw_path);
    if (dot_path != raw_path) add_candidates(source_dir + "/packages/", dot_path);
    add_candidates("library/", raw_path);
    if (dot_path != raw_path) add_candidates("library/", dot_path);
    add_candidates("packages/", raw_path);
    if (dot_path != raw_path) add_candidates("packages/", dot_path);

    std::string home_dir;
    if (const char* h = std::getenv("USERPROFILE")) {
        home_dir = h;
    } else if (const char* h = std::getenv("HOME")) {
        home_dir = h;
    }
    if (!home_dir.empty()) {
        add_candidates(home_dir + "/.wyrm/packages/", raw_path);
        add_candidates(home_dir + "/.wyrm/library/", raw_path);
        if (dot_path != raw_path) {
            add_candidates(home_dir + "/.wyrm/packages/", dot_path);
            add_candidates(home_dir + "/.wyrm/library/", dot_path);
        }
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

    std::ifstream file(target_path);
    if (!file.is_open()) {
        throw std::runtime_error("Compilation Error: Cannot open module file '" + target_path + "'");
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    file.close();

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

Value StructType::call(const std::vector<Value>& args, Interpreter* /*interpreter*/) {
    std::vector<const char*> field_name_ptrs;
    for (const auto& f : fields) {
        field_name_ptrs.push_back(f.c_str());
    }

    std::vector<Value> initial_vals(fields.size(), val_null());
    for (size_t i = 0; i < args.size() && i < fields.size(); ++i) {
        initial_vals[i] = val_copy(args[i]);
    }

    Value s = val_struct_create(name.c_str(), (int)fields.size(),
                                field_name_ptrs.empty() ? NULL : field_name_ptrs.data(),
                                initial_vals.empty() ? NULL : initial_vals.data());
    for (auto& v : initial_vals) {
        val_drop(v);
    }
    return s;
}

void Interpreter::visit(StructDefNode* node) {
    std::unordered_map<std::string, std::shared_ptr<Function>> methods;
    for (auto& m : node->methods) {
        std::vector<std::string> params;
        for (auto& p : m->params) {
            params.push_back(p->name);
        }
        std::vector<ASTNode*> body;
        for (auto& stmt : m->body) {
            body.push_back(stmt.get());
        }
        auto func = std::make_shared<Function>(m->name->name, params, body, current_env);
        methods[m->name->name] = func;
    }

    auto struct_type = std::make_shared<StructType>(node->name, node->fields, methods);
    struct_types_[node->name] = struct_type;

    // Define constructor callable in current environment
    Value ctor_val = val_raw_ptr(struct_type.get());
    current_env->define(node->name, ctor_val, false, false);

    last_value_ = val_null();
}

void Interpreter::visit(MemberAccessNode* node) {
    Value obj = evaluate(node->obj.get());
    if (obj.type == VAL_STRUCT && obj.as.structure) {
        last_value_ = val_struct_get(obj, node->member.c_str());
        return;
    }
    if (json_is_object(obj)) {
        Value key = val_string(node->member.c_str());
        last_value_ = json_get(obj, key);
        val_drop(key);
        return;
    }
    if (obj.type == VAL_RAW_PTR && obj.as.raw_ptr) {
        Value key = val_string(node->member.c_str());
        last_value_ = map_get(obj, key);
        val_drop(key);
        return;
    }
    last_value_ = val_null();
}

void Interpreter::visit(MemberAssignNode* node) {
    Value obj = evaluate(node->obj.get());
    Value val = evaluate(node->value.get());
    if (obj.type == VAL_STRUCT && obj.as.structure) {
        last_value_ = val_struct_set(obj, node->member.c_str(), val);
        return;
    }
    if (json_is_object(obj)) {
        Value key = val_string(node->member.c_str());
        last_value_ = json_set(obj, key, val);
        val_drop(key);
        return;
    }
    if (obj.type == VAL_RAW_PTR && obj.as.raw_ptr) {
        Value key = val_string(node->member.c_str());
        last_value_ = map_set(obj, key, val);
        val_drop(key);
        return;
    }
    last_value_ = val_null();
}

void Interpreter::visit(MethodCallNode* node) {
    Value obj = evaluate(node->obj.get());
    std::vector<Value> args;
    for (auto& arg : node->args) {
        args.push_back(evaluate(arg.get()));
    }

    if (obj.type == VAL_STRUCT && obj.as.structure) {
        std::string tname = obj.as.structure->type_name;
        auto it = struct_types_.find(tname);
        if (it != struct_types_.end()) {
            auto mit = it->second->methods.find(node->method_name);
            if (mit != it->second->methods.end()) {
                std::vector<Value> call_args;
                call_args.push_back(val_copy(obj));
                for (auto& a : args) call_args.push_back(val_copy(a));
                last_value_ = mit->second->call(call_args, this);
                return;
            }
        }
        throw std::runtime_error("Method '" + node->method_name + "' not found on struct '" + tname + "'");
    }

    throw std::runtime_error("Cannot call method '" + node->method_name + "' on non-struct value");
}

// ===========================================================================
// M2: EnumVariantConstructor::call
// Creates a struct-like Value with type_name = "EnumName::VariantName"
// ===========================================================================
Value EnumVariantConstructor::call(const std::vector<Value>& args, Interpreter* /*interp*/) {
    std::string tag = enum_name + "::" + variant_name;
    if (fields.empty()) {
        // Unit variant - create a struct with just a tag field
        const char* fnames[] = { "__tag__" };
        Value fvals[] = { val_string(tag.c_str()) };
        Value result = val_struct_create(tag.c_str(), 1, fnames, fvals);
        val_drop(fvals[0]);
        return result;
    }
    // Payload variant - bind positional args to field names
    size_t count = std::min(fields.size(), args.size());
    std::vector<const char*> fnames_raw;
    std::vector<Value> fvals;
    fnames_raw.push_back("__tag__");
    fvals.push_back(val_string(tag.c_str()));
    for (size_t i = 0; i < count; i++) {
        fnames_raw.push_back(fields[i].c_str());
        fvals.push_back(val_copy(args[i]));
    }
    Value result = val_struct_create(tag.c_str(),
        (int)fnames_raw.size(),
        fnames_raw.data(),
        fvals.data());
    for (auto& value : fvals) val_drop(value);
    return result;
}

// ===========================================================================
// M2: visit(EnumDefNode*)
// Registers each variant as a callable constructor in the environment
// ===========================================================================
void Interpreter::visit(EnumDefNode* node) {
    for (const auto& variant : node->variants) {
        auto* ctor = new EnumVariantConstructor(node->name, variant.name, variant.fields);
        callables_alive_.emplace_back(ctor);
        // Register as enum_name::VariantName and VariantName (short form)
        std::string full_name = node->name + "::" + variant.name;
        current_env->define(full_name, val_raw_ptr(ctor));
        current_env->define(variant.name, val_raw_ptr(ctor));
    }
    last_value_ = val_null();
}

// ===========================================================================
// M2: visit(MatchNode*)
// Evaluates subject, then iterates arms looking for variant or wildcard match
// ===========================================================================
void Interpreter::visit(MatchNode* node) {
    Value subject = evaluate(node->subject.get());
    // Identifier evaluation borrows the value stored in its environment. A
    // match consumes its temporary subject at the end, so retain structs and
    // other reference-counted values before that cleanup happens.
    if (dynamic_cast<IdentifierNode*>(node->subject.get())) {
        subject = val_copy(subject);
    }

    // Extract the __tag__ field from the subject (if it is a struct with a tag)
    std::string subject_tag;
    if (subject.type == VAL_STRUCT && subject.as.structure) {
        Value tag_val = val_struct_get(subject, "__tag__");
        if (tag_val.type == VAL_STRING && tag_val.as.string) {
            subject_tag = tag_val.as.string;
        }
        val_drop(tag_val);
    } else if (subject.type == VAL_STRING && subject.as.string) {
        subject_tag = subject.as.string;
    }

    bool matched = false;
    for (auto& arm : node->arms) {
        bool arm_matches = false;
        if (arm.variant_name == "_") {
            // Wildcard
            arm_matches = true;
        } else {
            // Match by exact variant name or full tag (EnumName::VariantName)
            arm_matches = (arm.variant_name == subject_tag) ||
                          (subject_tag.size() >= arm.variant_name.size() &&
                           subject_tag.substr(subject_tag.size() - arm.variant_name.size()) == arm.variant_name);
        }

        if (arm_matches) {
            // Create new scope for this arm
            auto arm_env = std::make_shared<Environment>(current_env.get());
            // Bind fields to binding names
            for (size_t i = 0; i < arm.bindings.size(); i++) {
                if (!arm.bindings[i].empty()) {
                    // Try to get field by index from the struct
                    if (subject.type == VAL_STRUCT && subject.as.structure) {
                        // Fields: index 0 = __tag__, index 1+ = payload fields
                        Value field_val = val_null();
                        int field_idx = (int)(i + 1);
                        if (subject.as.structure->field_count > field_idx) {
                            field_val = val_copy(subject.as.structure->fields[field_idx]);
                        }
                        arm_env->define(arm.bindings[i], field_val);
                        val_drop(field_val);
                    }
                }
            }
            auto saved_env = current_env;
            current_env = arm_env;
            try {
                for (auto& stmt : arm.body) {
                    execute_statement(stmt.get());
                }
            } catch (...) {
                current_env = saved_env;
                val_drop(subject);
                throw;
            }
            current_env = saved_env;
            matched = true;
            break;
        }
    }
    if (!matched) {
        // Non-exhaustive match is a runtime error
        throw std::runtime_error("RuntimeError: Non-exhaustive match - no arm matched value '" + subject_tag + "'");
    }
    val_drop(subject);
    last_value_ = val_null();
}

// ===========================================================================
// M3: visit(ThrowNode*)
// Evaluates the expression and throws a WyrmThrowSignal
// ===========================================================================
void Interpreter::visit(ThrowNode* node) {
    Value v = evaluate(node->expr.get());
    throw WyrmThrowSignal(v);
}

// ===========================================================================
// M3: visit(TryCatchNode*)
// Executes try body; catches WyrmThrowSignal or std::exception in catch block
// ===========================================================================
void Interpreter::visit(TryCatchNode* node) {
    bool caught = false;
    try {
        auto try_env = std::make_shared<Environment>(current_env.get());
        auto saved = current_env;
        current_env = try_env;
        try {
            for (auto& stmt : node->try_body) {
                execute_statement(stmt.get());
            }
        } catch (...) {
            current_env = saved;
            throw;
        }
        current_env = saved;
    } catch (const WyrmThrowSignal& sig) {
        caught = true;
        auto catch_env = std::make_shared<Environment>(current_env.get());
        if (!node->catch_var.empty() && node->catch_var != "_err") {
            catch_env->define(node->catch_var, val_copy(sig.value));
        } else {
            catch_env->define(node->catch_var, val_copy(sig.value));
        }
        auto saved = current_env;
        current_env = catch_env;
        try {
            for (auto& stmt : node->catch_body) {
                execute_statement(stmt.get());
            }
        } catch (...) {
            current_env = saved;
            // Run finally even when catch throws
            if (!node->finally_body.empty()) {
                auto saved2 = current_env;
                for (auto& stmt : node->finally_body) {
                    try { execute_statement(stmt.get()); } catch (...) {}
                }
                current_env = saved2;
            }
            throw;
        }
        current_env = saved;
    } catch (const std::exception& ex) {
        caught = true;
        auto catch_env = std::make_shared<Environment>(current_env.get());
        catch_env->define(node->catch_var, val_string(ex.what()));
        auto saved = current_env;
        current_env = catch_env;
        try {
            for (auto& stmt : node->catch_body) {
                execute_statement(stmt.get());
            }
        } catch (...) {
            current_env = saved;
            if (!node->finally_body.empty()) {
                for (auto& stmt : node->finally_body) {
                    try { execute_statement(stmt.get()); } catch (...) {}
                }
            }
            throw;
        }
        current_env = saved;
    }
    (void)caught;
    // Run finally block unconditionally
    if (!node->finally_body.empty()) {
        auto saved = current_env;
        for (auto& stmt : node->finally_body) {
            execute_statement(stmt.get());
        }
    }
    last_value_ = val_null();
}

// ===========================================================================
// M5: visit(LambdaNode*)
// Creates a closure (Function) capturing the current environment
// ===========================================================================
void Interpreter::visit(LambdaNode* node) {
    std::vector<std::string> param_names;
    param_names.reserve(node->params.size());
    for (auto& p : node->params) {
        param_names.push_back(p->name);
    }
    std::vector<ASTNode*> body_ptrs;
    body_ptrs.reserve(node->body.size());
    for (auto& s : node->body) {
        body_ptrs.push_back(s.get());
    }
    // Closure captures the current environment
    auto closure_fn = std::make_unique<Function>(
        "<lambda>", std::move(param_names), std::move(body_ptrs), current_env
    );
    Function* raw = closure_fn.get();
    callables_alive_.push_back(std::move(closure_fn));
    last_value_ = val_raw_ptr(raw);
}

// ===========================================================================
// M6: visit(AwaitNode*)
// Sequential execution: await evaluates expression immediately (no suspension)
// ===========================================================================
void Interpreter::visit(AwaitNode* node) {
    // Sequential semantics: evaluate the expression synchronously
    last_value_ = evaluate(node->expr.get());
}

} // namespace wyrm
