/*
 * wyrm_std_path.h - Wyrm Standard Library: Path Manipulation Module
 *
 * Pure string-based path operations. No filesystem access is performed;
 * all operations work on the path string only.
 */
#ifndef WYRM_STD_PATH_H
#define WYRM_STD_PATH_H

#include "../wyrm_core.h"

/* Joins two path components with the platform separator.
 * path_join("a/b", "c") -> "a/b/c" */
Value path_join(Value a, Value b);

/* Returns the final filename component of a path.
 * path_basename("/a/b/c.txt") -> "c.txt" */
Value path_basename(Value p);

/* Returns the directory component of a path.
 * path_dirname("/a/b/c.txt") -> "/a/b" */
Value path_dirname(Value p);

/* Returns the file extension including the dot, or "" if none.
 * path_ext("file.txt") -> ".txt" */
Value path_ext(Value p);

/* Returns the absolute (resolved) path.
 * Uses OS getcwd() to resolve relative segments. */
Value path_abs(Value p);

/* Returns val_bool(true) if the path is absolute. */
Value path_is_abs(Value p);

/* Strips the extension from a path.
 * path_stem("file.txt") -> "file" */
Value path_stem(Value p);

/* Replaces or appends a file extension.
 * path_with_ext("file.txt", ".md") -> "file.md" */
Value path_with_ext(Value p, Value ext);

#endif /* WYRM_STD_PATH_H */
