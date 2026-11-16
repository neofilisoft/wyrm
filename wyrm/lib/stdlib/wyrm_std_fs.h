/*
 * wyrm_std_fs.h - Wyrm Standard Library: File System Module
 *
 * Provides file read/write, directory listing, stat, and path operations.
 * All paths accept both forward and backward slashes on Windows.
 */
#ifndef WYRM_STD_FS_H
#define WYRM_STD_FS_H

#include "../wyrm_core.h"

/* Read the entire contents of a file, return as a string Value.
 * Returns val_error("message") on failure. */
Value fs_read(Value path);

/* Write a string to a file, overwriting its contents.
 * Returns val_bool(true) on success, val_error("message") on failure. */
Value fs_write(Value path, Value content);

/* Append a string to a file (creates the file if it does not exist).
 * Returns val_bool(true) on success, val_error("message") on failure. */
Value fs_append(Value path, Value content);

/* Returns val_bool(true) if path exists (file or directory). */
Value fs_exists(Value path);

/* Deletes a file. Returns val_bool(true) on success, false on failure. */
Value fs_delete(Value path);

/* Creates a directory (and any missing parent directories).
 * Returns val_bool(true) on success, false on failure. */
Value fs_mkdir(Value path);

/* Removes a directory and all its contents recursively.
 * Returns val_bool(true) on success, false on failure. */
Value fs_rmdir(Value path);

/* Returns an array of filenames (strings) in the given directory.
 * Returns val_error("message") on failure. */
Value fs_readdir(Value path);

/* Returns a map with file metadata for the given path:
 *   { "size": number, "is_dir": bool, "exists": bool }
 * Returns val_null() if the path does not exist. */
Value fs_stat(Value path);

/* Copy src to dst. Returns val_bool(true) on success. */
Value fs_copy(Value src, Value dst);

/* Move (rename) src to dst. Returns val_bool(true) on success. */
Value fs_move(Value src, Value dst);

#endif /* WYRM_STD_FS_H */
