/*
 * stdlib_setup.cpp - Wyrm Standard Library: Module Registration
 *
 * Implements the C++ registration layer. Each register_X function wraps
 * C backend functions from the wyrm/lib/stdlib/ directory into callable
 * builtins added to the interpreter's Environment.
 *
 * Threading uses std::thread (C++11) with a shared Interpreter instance
 * protected by a per-spawned-thread mutex so thread functions can safely
 * call back into the interpreter.
 */
#include "stdlib_setup.hpp"
#include "interpreter.hpp"

#include <stdexcept>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <memory>
#include <vector>

extern "C" {
#include "../../wyrm/lib/wyrm_ffi.h"
#include "../../wyrm/lib/stdlib/wyrm_std_json.h"
#include "../../wyrm/lib/stdlib/wyrm_std_yaml.h"
#include "../../wyrm/lib/stdlib/wyrm_std_sdl.h"
#include "../../wyrm/lib/stdlib/wyrm_std_collections.h"
#include "../../wyrm/lib/stdlib/wyrm_std_random.h"
}

namespace wyrm {
namespace stdlib {

/* ============================================================
 * Helpers
 * ============================================================ */

// Wrap a C builtin that takes 1 argument
static auto wrap1(Value (*fn)(Value)) {
    return [fn](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) throw std::runtime_error("expected 1 argument");
        return fn(args[0]);
    };
}

// Wrap a C builtin that takes 2 arguments
static auto wrap2(Value (*fn)(Value, Value)) {
    return [fn](const std::vector<Value>& args) -> Value {
        if (args.size() < 2) throw std::runtime_error("expected 2 arguments");
        return fn(args[0], args[1]);
    };
}

// Wrap a C builtin that takes 3 arguments
static auto wrap3(Value (*fn)(Value, Value, Value)) {
    return [fn](const std::vector<Value>& args) -> Value {
        if (args.size() < 3) throw std::runtime_error("expected 3 arguments");
        return fn(args[0], args[1], args[2]);
    };
}

// Wrap a C builtin that takes 0 arguments
static auto wrap0(Value (*fn)(void)) {
    return [fn](const std::vector<Value>&) -> Value { return fn(); };
}

// Register a lambda builtin into the Environment as a BuiltinCallable
static void reg(Environment& env, const std::string& name,
                std::function<Value(const std::vector<Value>&)> fn);

/* ============================================================
 * FFI module: std.ffi
 * ============================================================ */
void register_ffi(Environment& env) {
    reg(env, "ffi_open",  wrap1(ffi_open));
    reg(env, "ffi_sym",   wrap2(ffi_sym));
    reg(env, "ffi_close", wrap1(ffi_close));
    reg(env, "ffi_call",  [](const std::vector<Value>& args) -> Value {
        if (args.size() < 2) throw std::runtime_error("ffi_call: expected (func, args_array)");
        return ffi_call_fn(args[0], args[1]);
    });
}

/* ============================================================
 * JSON module: std.json
 * ============================================================ */
void register_json(Environment& env) {
    reg(env, "json_parse",   wrap1(json_parse));
    reg(env, "json_encode",  wrap1(json_encode));
    reg(env, "json_pretty",  wrap2(json_pretty));
    reg(env, "json_get",     wrap2(json_get));
    reg(env, "json_has",     wrap2(json_has));
    reg(env, "json_set",     wrap3(json_set));
    reg(env, "json_object",  wrap0(json_object));
}

/* ============================================================
 * YAML module: std.yaml
 * ============================================================ */
void register_yaml(Environment& env) {
    reg(env, "yaml_parse",  wrap1(yaml_parse));
    reg(env, "yaml_encode", wrap1(yaml_encode));
}

/* ============================================================
 * SDL2 module: std.sdl
 * ============================================================ */
void register_sdl(Environment& env) {
    reg(env, "sdl_init",           wrap0(sdl_init));
    reg(env, "sdl_quit",           wrap0(sdl_quit));
    reg(env, "sdl_ticks",          wrap0(sdl_ticks));
    reg(env, "sdl_poll_event",     wrap0(sdl_poll_event));
    reg(env, "sdl_delay",          wrap1(sdl_delay));
    reg(env, "sdl_destroy_window", wrap1(sdl_destroy_window));
    reg(env, "sdl_window", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 3) throw std::runtime_error("sdl_window: expected (title, width, height)");
        return sdl_window(args[0], args[1], args[2]);
    });
    reg(env, "sdl_clear", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 4) throw std::runtime_error("sdl_clear: expected (win, r, g, b)");
        return sdl_clear(args[0], args[1], args[2], args[3]);
    });
    reg(env, "sdl_present", wrap1(sdl_present));
    reg(env, "sdl_draw_rect", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 8) throw std::runtime_error("sdl_draw_rect: expected (win, x, y, w, h, r, g, b)");
        return sdl_draw_rect(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]);
    });
    reg(env, "sdl_draw_line", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 8) throw std::runtime_error("sdl_draw_line: expected (win, x1, y1, x2, y2, r, g, b)");
        return sdl_draw_line(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]);
    });
    // Register SDL event type string constants
    env.define("SDL_QUIT",        val_string(WYRM_SDL_EVT_QUIT));
    env.define("SDL_KEYDOWN",     val_string(WYRM_SDL_EVT_KEYDOWN));
    env.define("SDL_KEYUP",       val_string(WYRM_SDL_EVT_KEYUP));
    env.define("SDL_MOUSEMOTION", val_string(WYRM_SDL_EVT_MOUSEMOTION));
    env.define("SDL_MOUSEDOWN",   val_string(WYRM_SDL_EVT_MOUSEDOWN));
    env.define("SDL_MOUSEUP",     val_string(WYRM_SDL_EVT_MOUSEUP));
}

/* ============================================================
 * Threading module: std.thread
 * ============================================================ */

// Thread record: keeps the thread object and its done state alive
struct WyrmThread {
    std::thread     thr;
    std::mutex      mtx;
    bool            done = false;
    Value           result;

    WyrmThread() : result(val_null()) {}
    WyrmThread(const WyrmThread&) = delete;
    WyrmThread& operator=(const WyrmThread&) = delete;
};

void register_thread(Environment& env) {
    // thread_spawn(fn_value, args_array) -> thread_handle
    reg(env, "thread_spawn", [](const std::vector<Value>& args) -> Value {
        if (args.empty()) throw std::runtime_error("thread_spawn: expected (fn, args_array)");
        if (args[0].type != VAL_RAW_PTR || !args[0].as.raw_ptr) {
            throw std::runtime_error("thread_spawn: first argument must be a function");
        }

        // Capture callable pointer and argument copies
        Callable* callable = static_cast<Callable*>(args[0].as.raw_ptr);
        std::vector<Value> thread_args;
        if (args.size() >= 2 && args[1].type == VAL_ARRAY && args[1].as.array) {
            for (int i = 0; i < args[1].as.array->size; i++) {
                thread_args.push_back(val_copy(args[1].as.array->data[i]));
            }
        }

        auto record = std::make_shared<WyrmThread>();

        record->thr = std::thread([callable, thread_args = std::move(thread_args), record]() mutable {
            try {
                // Each thread creates its own Interpreter instance
                Interpreter interp;
                Value res = callable->call(thread_args, &interp);
                std::lock_guard<std::mutex> lk(record->mtx);
                record->result = res;
                record->done   = true;
            } catch (const std::exception& ex) {
                std::lock_guard<std::mutex> lk(record->mtx);
                record->result = val_error(ex.what());
                record->done   = true;
            }
            for (auto& a : thread_args) val_drop(a);
        });

        // Box the shared_ptr into a raw ptr stored in a Value
        // Use heap-allocated shared_ptr so it survives across calls
        auto *ptr = new std::shared_ptr<WyrmThread>(record);
        return val_raw_ptr(ptr);
    });

    // thread_join(handle) -> return_value
    reg(env, "thread_join", [](const std::vector<Value>& args) -> Value {
        if (args.empty() || args[0].type != VAL_RAW_PTR || !args[0].as.raw_ptr)
            throw std::runtime_error("thread_join: expected a thread handle");
        auto *ptr = static_cast<std::shared_ptr<WyrmThread>*>(args[0].as.raw_ptr);
        WyrmThread& rec = **ptr;
        if (rec.thr.joinable()) rec.thr.join();
        std::lock_guard<std::mutex> lk(rec.mtx);
        Value res = val_copy(rec.result);
        delete ptr; // free the boxed shared_ptr
        return res;
    });

    // thread_detach(handle)
    reg(env, "thread_detach", [](const std::vector<Value>& args) -> Value {
        if (args.empty() || args[0].type != VAL_RAW_PTR || !args[0].as.raw_ptr)
            throw std::runtime_error("thread_detach: expected a thread handle");
        auto *ptr = static_cast<std::shared_ptr<WyrmThread>*>(args[0].as.raw_ptr);
        if ((*ptr)->thr.joinable()) (*ptr)->thr.detach();
        delete ptr;
        return val_null();
    });

    // thread_sleep(ms)
    reg(env, "thread_sleep", [](const std::vector<Value>& args) -> Value {
        if (args.empty() || args[0].type != VAL_NUMBER)
            throw std::runtime_error("thread_sleep: expected milliseconds (number)");
        std::this_thread::sleep_for(
            std::chrono::milliseconds(static_cast<long long>(args[0].as.number)));
        return val_null();
    });

    // thread_id() -> number (hash of current thread id)
    reg(env, "thread_id", [](const std::vector<Value>&) -> Value {
        auto id = std::this_thread::get_id();
        size_t h = std::hash<std::thread::id>{}(id);
        return val_number(static_cast<double>(h));
    });

    // mutex_new() -> mutex_handle
    reg(env, "mutex_new", [](const std::vector<Value>&) -> Value {
        auto *m = new std::mutex();
        return val_raw_ptr(m);
    });

    // mutex_lock(handle)
    reg(env, "mutex_lock", [](const std::vector<Value>& args) -> Value {
        if (args.empty() || args[0].type != VAL_RAW_PTR || !args[0].as.raw_ptr)
            throw std::runtime_error("mutex_lock: expected a mutex handle");
        static_cast<std::mutex*>(args[0].as.raw_ptr)->lock();
        return val_null();
    });

    // mutex_unlock(handle)
    reg(env, "mutex_unlock", [](const std::vector<Value>& args) -> Value {
        if (args.empty() || args[0].type != VAL_RAW_PTR || !args[0].as.raw_ptr)
            throw std::runtime_error("mutex_unlock: expected a mutex handle");
        static_cast<std::mutex*>(args[0].as.raw_ptr)->unlock();
        return val_null();
    });

    // mutex_destroy(handle)
    reg(env, "mutex_destroy", [](const std::vector<Value>& args) -> Value {
        if (args.empty() || args[0].type != VAL_RAW_PTR || !args[0].as.raw_ptr)
            throw std::runtime_error("mutex_destroy: expected a mutex handle");
        delete static_cast<std::mutex*>(args[0].as.raw_ptr);
        return val_null();
    });
}

/* ============================================================
 * Collections module: std.collections
 * ============================================================ */
void register_collections(Environment& env) {
    reg(env, "map_new",    [](const std::vector<Value>&) -> Value { return map_new(); });
    reg(env, "map_set",    wrap3(map_set));
    reg(env, "map_get",    wrap2(map_get));
    reg(env, "map_has",    wrap2(map_has));
    reg(env, "map_del",    wrap2(map_del));
    reg(env, "map_keys",   wrap1(map_keys));
    reg(env, "map_values", wrap1(map_values));
    reg(env, "map_len",    wrap1(map_len));
    reg(env, "set_new",      [](const std::vector<Value>&) -> Value { return set_new(); });
    reg(env, "set_add",      wrap2(set_add));
    reg(env, "set_has",      wrap2(set_has));
    reg(env, "set_del",      wrap2(set_del));
    reg(env, "set_union",    wrap2(set_union_fn));
    reg(env, "set_intersect",wrap2(set_intersect));
    reg(env, "set_to_array", wrap1(set_to_array));
}

void register_random(Environment& env) {
    reg(env, "rand_seed",        wrap1(rand_seed));
    reg(env, "rand",             wrap0(rand_val));
    reg(env, "rand_int",         wrap2(rand_int));
    reg(env, "rand_range",       wrap2(rand_range));
    reg(env, "rand_choice",      wrap1(rand_choice));
    reg(env, "rand_shuffle",     wrap1(rand_shuffle));
    reg(env, "rand_secure",      wrap0(rand_secure));
    reg(env, "rand_secure_int",  wrap2(rand_secure_int));
    reg(env, "rand_bytes_hex",   wrap1(rand_bytes_hex));
    reg(env, "rand_has_trng",    wrap0(rand_has_trng));
    reg(env, "rand_trng",        wrap0(rand_trng));
    reg(env, "rand_trng_int",    wrap2(rand_trng_int));
    reg(env, "rand_reseed_trng", wrap0(rand_reseed_trng));
}

/* ============================================================
 * try_register: public dispatch function
 * ============================================================ */
bool try_register(const std::string& module_path, Environment& env) {
    static const std::unordered_map<std::string,
        void(*)(Environment&)> dispatch_table = {
        { "std.ffi",         register_ffi         },
        { "std.json",        register_json        },
        { "std.yaml",        register_yaml        },
        { "std.sdl",         register_sdl         },
        { "std.thread",      register_thread      },
        { "std.collections", register_collections },
        { "std.random",      register_random      },
    };
    auto it = dispatch_table.find(module_path);
    if (it == dispatch_table.end()) return false;
    it->second(env);
    return true;
}

/* ============================================================
 * reg helper: wraps a std::function as a BuiltinCallable in env
 * Must be defined after BuiltinCallable is visible from interpreter.hpp
 * ============================================================ */
static void reg(Environment& env, const std::string& name,
                std::function<Value(const std::vector<Value>&)> fn) {
    struct StdBuiltin : public Callable {
        std::string name_;
        std::function<Value(const std::vector<Value>&)> fn_;
        int depth_ = 0;

        StdBuiltin(std::string n, std::function<Value(const std::vector<Value>&)> f)
            : name_(std::move(n)), fn_(std::move(f)) {}

        Value call(const std::vector<Value>& args, Interpreter* /*interp*/) override {
            return fn_(args);
        }
        bool is_builtin() const override { return true; }
    };

    // Allocate on heap; lifetime managed by the Interpreter's callables_alive_ set
    auto *builtin = new StdBuiltin(name, std::move(fn));
    env.define(name, val_raw_ptr(builtin));
}

} // namespace stdlib
} // namespace wyrm
