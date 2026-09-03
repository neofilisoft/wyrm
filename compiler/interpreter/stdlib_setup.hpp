/*
 * stdlib_setup.hpp - Wyrm Standard Library: C++ Registration Layer
 *
 * This header declares the C++ functions that register each stdlib module's
 * native builtins into the interpreter's Environment. The registration
 * happens when the interpreter processes a "use std.X" statement.
 *
 * Architecture:
 *   - Each stdlib module has a C backend (wyrm_std_X.h/.c)
 *   - This C++ layer wraps the C functions and registers them as builtins
 *   - The interpreter's visit(UseNode*) calls stdlib::try_register() first
 */
#pragma once

#include <string>
#include <functional>
#include <vector>
#include "environment.hpp"

namespace wyrm {
namespace stdlib {

/* --------------------------------------------------------------------------
 * try_register: attempt to register a stdlib module.
 * Returns true if module_path matched a known stdlib module and was
 * registered. Returns false if it's not a stdlib path (caller should
 * fall through to filesystem loading).
 * -------------------------------------------------------------------------- */
bool try_register(const std::string& module_path, Environment& env);

/* --------------------------------------------------------------------------
 * Individual module registration functions.
 * Called by try_register for each matching "std.X" path.
 * -------------------------------------------------------------------------- */
void register_ffi(Environment& env);
void register_json(Environment& env);
void register_yaml(Environment& env);
void register_sdl(Environment& env);
void register_thread(Environment& env);
void register_collections(Environment& env);

} // namespace stdlib
} // namespace wyrm
