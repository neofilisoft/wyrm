/*
 * wyrm_std_io.c - Wyrm Standard Library: I/O Module Implementation
 */
#include "wyrm_std_io.h"
#include "../wyrm_core.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Value io_stdin(void) {
    size_t cap  = 4096;
    size_t len  = 0;
    char  *buf  = (char *)malloc(cap);
    if (!buf) return val_error("io_stdin: out of memory");

    int c;
    while ((c = fgetc(stdin)) != EOF) {
        if (len + 1 >= cap) {
            cap *= 2;
            char *tmp = (char *)realloc(buf, cap);
            if (!tmp) { free(buf); return val_error("io_stdin: out of memory"); }
            buf = tmp;
        }
        buf[len++] = (char)c;
    }
    buf[len] = '\0';

    Value result = val_string(buf);
    free(buf);
    return result;
}

Value io_readline(void) {
    size_t cap  = 256;
    size_t len  = 0;
    char  *buf  = (char *)malloc(cap);
    if (!buf) return val_error("io_readline: out of memory");

    int c;
    while ((c = fgetc(stdin)) != EOF) {
        if (c == '\n') break;
        if (len + 1 >= cap) {
            cap *= 2;
            char *tmp = (char *)realloc(buf, cap);
            if (!tmp) { free(buf); return val_error("io_readline: out of memory"); }
            buf = tmp;
        }
        buf[len++] = (char)c;
    }

    if (c == EOF && len == 0) {
        free(buf);
        return val_null();
    }

    /* Strip trailing \r for Windows line endings */
    if (len > 0 && buf[len - 1] == '\r') len--;
    buf[len] = '\0';

    Value result = val_string(buf);
    free(buf);
    return result;
}

Value io_write(Value text) {
    if (text.type == VAL_STRING && text.as.string) {
        fputs(text.as.string, stdout);
    }
    return val_null();
}

Value io_writeln(Value text) {
    if (text.type == VAL_STRING && text.as.string) {
        fputs(text.as.string, stdout);
    }
    fputc('\n', stdout);
    return val_null();
}

Value io_flush(void) {
    fflush(stdout);
    return val_null();
}

Value io_stderr(Value msg) {
    if (msg.type == VAL_STRING && msg.as.string) {
        fputs(msg.as.string, stderr);
    }
    return val_null();
}

Value io_stderrln(Value msg) {
    if (msg.type == VAL_STRING && msg.as.string) {
        fputs(msg.as.string, stderr);
    }
    fputc('\n', stderr);
    return val_null();
}
