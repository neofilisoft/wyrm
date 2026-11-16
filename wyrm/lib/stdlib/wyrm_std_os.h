/*
 * wyrm_std_os.h - Wyrm Standard Library: OS/Process Module
 *
 * Provides access to operating system environment variables, command-line
 * arguments, working directory, process exit, and shell command execution.
 *
 * All functions return Value types compatible with the Wyrm runtime.
 * Windows and POSIX are both supported via conditional compilation.
 */
#ifndef WYRM_STD_OS_H
#define WYRM_STD_OS_H

#include "../wyrm_core.h"

/* Returns an array of all command-line arguments as strings.
 * The array is populated by val_init_sys_args() at program startup. */
Value os_args(void);

/* Returns the value of an environment variable as a string Value.
 * Returns val_null() if the variable is not set. */
Value os_env(Value name);

/* Sets the environment variable 'name' to 'val'.
 * Returns val_bool(true) on success, val_bool(false) on failure. */
Value os_setenv(Value name, Value val);

/* Unsets (removes) an environment variable.
 * Returns val_bool(true) on success, val_bool(false) on failure. */
Value os_unsetenv(Value name);

/* Returns the current working directory as a string Value. */
Value os_cwd(void);

/* Changes the current working directory to the given path.
 * Returns val_bool(true) on success, val_bool(false) on failure. */
Value os_chdir(Value path);

/* Terminates the process with the given exit code.
 * Accepts a numeric Value; defaults to 0 if argument is null. */
Value os_exit(Value code);

/* Executes a shell command string and returns the exit code as a number.
 * The command is passed directly to the platform shell (cmd.exe / sh). */
Value os_exec(Value cmd);

/* Returns the platform name as a string: "windows", "linux", or "macos". */
Value os_platform(void);

/* Returns the path separator character string: "\" on Windows, "/" elsewhere. */
Value os_sep(void);

/* Returns a map of all environment variables: { name: value, ... }. */
Value os_environ(void);

#endif /* WYRM_STD_OS_H */
