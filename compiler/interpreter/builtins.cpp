#include "interpreter.hpp"
#include <iostream>
#include <stdexcept>

extern "C" {
#include "../../wyrm/lib/wyrm_core.h"
#include "../../wyrm/lib/wyrm_str.h"
}

namespace wyrm {

namespace {

Value builtin_print(const std::vector<Value>& args) {
    for (size_t i = 0; i < args.size(); ++i) {
        char* s = val_to_str_ptr(args[i]);
        std::cout << s;
        free(s);
        if (i + 1 < args.size()) {
            std::cout << " ";
        }
    }
    std::cout << std::endl;
    return val_null();
}

Value builtin_input(const std::vector<Value>& args) {
    Value prompt = args.empty() ? val_string("") : args[0];
    return val_input(prompt);
}

Value builtin_int(const std::vector<Value>& args) {
    if (args.empty()) return val_number(0.0);
    return val_int(args[0]);
}

Value builtin_float(const std::vector<Value>& args) {
    if (args.empty()) return val_number(0.0);
    return val_float(args[0]);
}

Value builtin_str(const std::vector<Value>& args) {
    if (args.empty()) return val_string("");
    return val_str(args[0]);
}

Value builtin_len(const std::vector<Value>& args) {
    if (args.empty()) return val_number(0.0);
    return val_len(args[0]);
}

Value builtin_type(const std::vector<Value>& args) {
    if (args.empty()) return val_string("null");
    return val_type(args[0]);
}

Value builtin_abs(const std::vector<Value>& args) {
    if (args.empty()) return val_number(0.0);
    return val_abs(args[0]);
}

Value builtin_min(const std::vector<Value>& args) {
    if (args.empty()) return val_null();
    Value lowest = args[0];
    for (size_t i = 1; i < args.size(); ++i) {
        Value cmp = val_lt(args[i], lowest);
        if (val_to_bool(cmp)) {
            lowest = args[i];
        }
    }
    return lowest;
}

Value builtin_max(const std::vector<Value>& args) {
    if (args.empty()) return val_null();
    Value highest = args[0];
    for (size_t i = 1; i < args.size(); ++i) {
        Value cmp = val_gt(args[i], highest);
        if (val_to_bool(cmp)) {
            highest = args[i];
        }
    }
    return highest;
}

Value builtin_round(const std::vector<Value>& args) {
    if (args.empty()) return val_number(0.0);
    return val_round(args[0]);
}

Value builtin_pow(const std::vector<Value>& args) {
    if (args.size() < 2) return val_number(0.0);
    return val_pow(args[0], args[1]);
}

Value builtin_append(const std::vector<Value>& args) {
    if (args.size() < 2) throw std::runtime_error("append() expects 2 arguments");
    return val_array_append(args[0], args[1]);
}

Value builtin_pop(const std::vector<Value>& args) {
    if (args.empty()) throw std::runtime_error("pop() expects 1 argument");
    return val_array_pop(args[0]);
}

Value builtin_split(const std::vector<Value>& args) {
    if (args.size() < 2) throw std::runtime_error("split() expects 2 arguments");
    return val_split(args[0], args[1]);
}

Value builtin_join(const std::vector<Value>& args) {
    if (args.size() < 2) throw std::runtime_error("join() expects 2 arguments");
    return val_join(args[0], args[1]);
}

Value builtin_trim(const std::vector<Value>& args) {
    if (args.empty()) throw std::runtime_error("trim() expects 1 argument");
    return val_trim(args[0]);
}

Value builtin_upper(const std::vector<Value>& args) {
    if (args.empty()) throw std::runtime_error("upper() expects 1 argument");
    return val_upper(args[0]);
}

Value builtin_lower(const std::vector<Value>& args) {
    if (args.empty()) throw std::runtime_error("lower() expects 1 argument");
    return val_lower(args[0]);
}

Value builtin_contains(const std::vector<Value>& args) {
    if (args.size() < 2) throw std::runtime_error("contains() expects 2 arguments");
    return val_contains(args[0], args[1]);
}

Value builtin_replace(const std::vector<Value>& args) {
    if (args.size() < 3) throw std::runtime_error("replace() expects 3 arguments");
    return val_replace(args[0], args[1], args[2]);
}

Value builtin_starts_with(const std::vector<Value>& args) {
    if (args.size() < 2) throw std::runtime_error("starts_with() expects 2 arguments");
    return val_starts_with(args[0], args[1]);
}

Value builtin_ends_with(const std::vector<Value>& args) {
    if (args.size() < 2) throw std::runtime_error("ends_with() expects 2 arguments");
    return val_ends_with(args[0], args[1]);
}

Value builtin_char_at(const std::vector<Value>& args) {
    if (args.size() < 2) throw std::runtime_error("char_at() expects 2 arguments");
    return val_char_at(args[0], args[1]);
}

Value builtin_ord_val(const std::vector<Value>& args) {
    if (args.empty()) throw std::runtime_error("ord_val() expects 1 argument");
    return val_ord_val(args[0]);
}

Value builtin_chr_val(const std::vector<Value>& args) {
    if (args.empty()) throw std::runtime_error("chr_val() expects 1 argument");
    return val_chr_val(args[0]);
}

Value builtin_to_bytes(const std::vector<Value>& args) {
    if (args.empty()) throw std::runtime_error("to_bytes() expects 1 argument");
    return val_to_bytes(args[0]);
}

Value builtin_from_bytes(const std::vector<Value>& args) {
    if (args.empty()) throw std::runtime_error("from_bytes() expects 1 argument");
    return val_from_bytes(args[0]);
}

Value builtin_read_file(const std::vector<Value>& args) {
    if (args.empty()) throw std::runtime_error("read_file() expects 1 argument");
    return val_read_file(args[0]);
}

Value builtin_write_file(const std::vector<Value>& args) {
    if (args.size() < 2) throw std::runtime_error("write_file() expects 2 arguments");
    return val_write_file(args[0], args[1]);
}

Value builtin_exit(const std::vector<Value>& args) {
    Value code = args.empty() ? val_number(0.0) : args[0];
    return val_exit(code);
}

Value builtin_getenv(const std::vector<Value>& args) {
    if (args.empty()) throw std::runtime_error("getenv() expects 1 argument");
    return val_getenv(args[0]);
}

Value builtin_system(const std::vector<Value>& args) {
    if (args.empty()) throw std::runtime_error("system() expects 1 argument");
    return val_system(args[0]);
}

} // namespace

class SysArgsFunction : public Callable {
public:
    Value call(const std::vector<Value>& args, Interpreter* interpreter) override {
        Value arr = val_array_create(interpreter->cli_args.size());
        for (size_t i = 0; i < interpreter->cli_args.size(); ++i) {
            arr.as.array->data[i] = val_string(interpreter->cli_args[i].c_str());
        }
        return arr;
    }
    bool is_builtin() const override { return true; }
};

void Interpreter::setup_builtins() {
    auto sys_args_fn = std::make_unique<SysArgsFunction>();
    global_env->define("sys_args", val_raw_ptr(sys_args_fn.get()), true, false);
    callables_alive_.push_back(std::move(sys_args_fn));

    auto register_fn = [this](const std::string& name, BuiltinFnPtr f) {
        auto fn = std::make_unique<BuiltinFunction>(name, f);
        global_env->define(name, val_raw_ptr(fn.get()), true, false);
        callables_alive_.push_back(std::move(fn));
    };

    register_fn("print", builtin_print);
    register_fn("input", builtin_input);
    register_fn("int", builtin_int);
    register_fn("float", builtin_float);
    register_fn("str", builtin_str);
    register_fn("len", builtin_len);
    register_fn("type", builtin_type);
    register_fn("abs", builtin_abs);
    register_fn("min", builtin_min);
    register_fn("max", builtin_max);
    register_fn("round", builtin_round);
    register_fn("pow", builtin_pow);
    register_fn("append", builtin_append);
    register_fn("pop", builtin_pop);

    // Spezialized builtins intercepting interpreter context
    auto register_special_fn = [this](const std::string& name) {
        auto fn = std::make_unique<BuiltinFunction>(name, nullptr);
        global_env->define(name, val_raw_ptr(fn.get()), true, false);
        callables_alive_.push_back(std::move(fn));
    };

    register_special_fn("malloc");
    register_special_fn("free");
    register_special_fn("realloc");

    // 14 string operations
    register_fn("split", builtin_split);
    register_fn("join", builtin_join);
    register_fn("trim", builtin_trim);
    register_fn("upper", builtin_upper);
    register_fn("lower", builtin_lower);
    register_fn("contains", builtin_contains);
    register_fn("replace", builtin_replace);
    register_fn("starts_with", builtin_starts_with);
    register_fn("ends_with", builtin_ends_with);
    register_fn("char_at", builtin_char_at);
    register_fn("ord_val", builtin_ord_val);
    register_fn("chr_val", builtin_chr_val);
    register_fn("to_bytes", builtin_to_bytes);
    register_fn("from_bytes", builtin_from_bytes);
    register_fn("read_file", builtin_read_file);
    register_fn("write_file", builtin_write_file);
    register_fn("exit", builtin_exit);
    register_fn("getenv", builtin_getenv);
    register_fn("system", builtin_system);
}

} // namespace wyrm
