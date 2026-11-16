/*
 * wyrm_std_io.h - Wyrm Standard Library: I/O Module
 *
 * Buffered stdin/stdout/stderr access beyond the basic print() builtin.
 */
#ifndef WYRM_STD_IO_H
#define WYRM_STD_IO_H

#include "../wyrm_core.h"

/* Reads all of stdin until EOF, returns as a string Value. */
Value io_stdin(void);

/* Reads a single line from stdin (strips trailing newline).
 * Returns val_null() on EOF. */
Value io_readline(void);

/* Writes a string to stdout without a trailing newline. */
Value io_write(Value text);

/* Writes a string to stdout with a trailing newline. */
Value io_writeln(Value text);

/* Flushes stdout. */
Value io_flush(void);

/* Writes a string to stderr (no newline). */
Value io_stderr(Value msg);

/* Writes a string to stderr with a trailing newline. */
Value io_stderrln(Value msg);

#endif /* WYRM_STD_IO_H */
