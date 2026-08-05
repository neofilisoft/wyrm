#ifndef WYRM_ENVIRONMENT_HPP
#define WYRM_ENVIRONMENT_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>

extern "C" {
#include "../../wyrm/lib/wyrm_core.h"
#include "../../wyrm/lib/wyrm_arena.h"
}

namespace wyrm {

struct Slot {
    Value value;
    bool is_constant;
    bool is_moved;
    bool is_owned;
};

class Environment {
public:
    Environment* outer;
    std::unordered_map<std::string, Slot> store;
    std::unordered_map<std::string, WyrmArena*> arenas;
    std::vector<void*> raw_allocations;

    explicit Environment(Environment* parent = nullptr) : outer(parent) {}

    ~Environment() {
        // RAII: Free raw allocations allocated in this scope
        for (void* ptr : raw_allocations) {
            if (ptr) {
                free(ptr);
            }
        }
        // RAII: Destroy arenas created in this scope
        for (auto& pair : arenas) {
            if (pair.second) {
                arena_destroy(pair.second);
            }
        }
    }

    // Disable copy constructors to ensure RAII runs cleanly once
    Environment(const Environment&) = delete;
    Environment& operator=(const Environment&) = delete;

    void define(const std::string& name, Value val, bool is_const = false, bool is_own = false) {
        store[name] = Slot{val, is_const, false, is_own};
    }

    Value get(const std::string& name) {
        auto it = store.find(name);
        if (it != store.end()) {
            if (it->second.is_moved) {
                throw std::runtime_error("Use-after-move: variable '" + name + "' has been moved and can no longer be used");
            }
            return it->second.value;
        }
        if (outer) {
            return outer->get(name);
        }
        throw std::runtime_error("Undefined variable: '" + name + "'");
    }

    bool is_owned(const std::string& name) {
        auto it = store.find(name);
        if (it != store.end()) {
            return it->second.is_owned;
        }
        if (outer) {
            return outer->is_owned(name);
        }
        return false;
    }

    void set(const std::string& name, Value val) {
        auto it = store.find(name);
        if (it != store.end()) {
            if (it->second.is_constant) {
                throw std::runtime_error("Cannot assign to constant '" + name + "'");
            }
            if (it->second.is_moved) {
                throw std::runtime_error("Cannot assign to moved variable '" + name + "'");
            }
            it->second.value = val;
            return;
        }
        if (outer) {
            outer->set(name, val);
            return;
        }
        // Var fallback: define as mutable in current scope
        define(name, val, false, false);
    }

    void mark_moved(const std::string& name) {
        auto it = store.find(name);
        if (it != store.end()) {
            it->second.is_moved = true;
            return;
        }
        if (outer) {
            outer->mark_moved(name);
        }
    }

    void define_arena(const std::string& name, WyrmArena* arena) {
        arenas[name] = arena;
    }

    WyrmArena* get_arena(const std::string& name) {
        auto it = arenas.find(name);
        if (it != arenas.end()) {
            return it->second;
        }
        if (outer) {
            return outer->get_arena(name);
        }
        throw std::runtime_error("Undefined arena: '" + name + "'");
    }

    void register_raw_allocation(void* ptr) {
        raw_allocations.push_back(ptr);
    }

    void unregister_raw_allocation(void* ptr) {
        for (auto it = raw_allocations.begin(); it != raw_allocations.end(); ++it) {
            if (*it == ptr) {
                raw_allocations.erase(it);
                return;
            }
        }
        if (outer) {
            outer->unregister_raw_allocation(ptr);
        }
    }
};

} // namespace wyrm

#endif // WYRM_ENVIRONMENT_HPP
