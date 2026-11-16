/*
 * wyrm_std_fs.c - Wyrm Standard Library: File System Module Implementation
 */
#include "wyrm_std_fs.h"
#include "../wyrm_core.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#ifdef _WIN32
#  include <windows.h>
#  include <direct.h>
#  define WYRM_MKDIR(p) _mkdir(p)
#  define WYRM_STAT_T   struct _stat
#  define WYRM_STAT     _stat
#else
#  include <dirent.h>
#  include <unistd.h>
#  include <sys/types.h>
#  define WYRM_MKDIR(p) mkdir((p), 0755)
#  define WYRM_STAT_T   struct stat
#  define WYRM_STAT     stat
#endif

/* ------------------------------------------------------------------ */

Value fs_read(Value path) {
    if (path.type != VAL_STRING || !path.as.string) {
        return val_error("fs_read: path must be a string");
    }
    FILE *f = fopen(path.as.string, "rb");
    if (!f) {
        char msg[512];
        snprintf(msg, sizeof(msg), "fs_read: cannot open '%s': %s", path.as.string, strerror(errno));
        return val_error(msg);
    }

    /* Determine file size */
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return val_error("fs_read: fseek failed");
    }
    long sz = ftell(f);
    if (sz < 0) {
        fclose(f);
        return val_error("fs_read: ftell failed");
    }
    rewind(f);

    char *buf = (char *)malloc((size_t)sz + 1);
    if (!buf) {
        fclose(f);
        return val_error("fs_read: out of memory");
    }
    size_t rd = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    buf[rd] = '\0';

    Value result = val_string(buf);
    free(buf);
    return result;
}

Value fs_write(Value path, Value content) {
    if (path.type != VAL_STRING || !path.as.string) {
        return val_error("fs_write: path must be a string");
    }
    const char *text = (content.type == VAL_STRING && content.as.string) ? content.as.string : "";

    FILE *f = fopen(path.as.string, "wb");
    if (!f) {
        char msg[512];
        snprintf(msg, sizeof(msg), "fs_write: cannot open '%s': %s", path.as.string, strerror(errno));
        return val_error(msg);
    }
    size_t len = strlen(text);
    size_t written = fwrite(text, 1, len, f);
    fclose(f);

    if (written != len) {
        return val_error("fs_write: write incomplete");
    }
    return val_bool(1);
}

Value fs_append(Value path, Value content) {
    if (path.type != VAL_STRING || !path.as.string) {
        return val_error("fs_append: path must be a string");
    }
    const char *text = (content.type == VAL_STRING && content.as.string) ? content.as.string : "";

    FILE *f = fopen(path.as.string, "ab");
    if (!f) {
        char msg[512];
        snprintf(msg, sizeof(msg), "fs_append: cannot open '%s': %s", path.as.string, strerror(errno));
        return val_error(msg);
    }
    size_t len = strlen(text);
    size_t written = fwrite(text, 1, len, f);
    fclose(f);

    if (written != len) {
        return val_error("fs_append: write incomplete");
    }
    return val_bool(1);
}

Value fs_exists(Value path) {
    if (path.type != VAL_STRING || !path.as.string) {
        return val_bool(0);
    }
    WYRM_STAT_T st;
    return val_bool(WYRM_STAT(path.as.string, &st) == 0);
}

Value fs_delete(Value path) {
    if (path.type != VAL_STRING || !path.as.string) {
        return val_bool(0);
    }
    return val_bool(remove(path.as.string) == 0);
}

/* Recursive mkdir helper */
static int make_dirs(const char *path) {
    char tmp[4096];
    size_t len;
    snprintf(tmp, sizeof(tmp), "%s", path);
    len = strlen(tmp);
    if (len == 0) return 0;
    if (tmp[len - 1] == '/' || tmp[len - 1] == '\\') {
        tmp[len - 1] = '\0';
    }
    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/' || *p == '\\') {
            char saved = *p;
            *p = '\0';
            WYRM_MKDIR(tmp); /* ignore errors - dir may already exist */
            *p = saved;
        }
    }
    return WYRM_MKDIR(tmp);
}

Value fs_mkdir(Value path) {
    if (path.type != VAL_STRING || !path.as.string) {
        return val_bool(0);
    }
    int r = make_dirs(path.as.string);
    /* mkdir returns 0 on success, EEXIST is also acceptable */
    return val_bool(r == 0 || errno == EEXIST);
}

Value fs_rmdir(Value path) {
    if (path.type != VAL_STRING || !path.as.string) {
        return val_bool(0);
    }
#ifdef _WIN32
    char cmd[4096];
    snprintf(cmd, sizeof(cmd), "rmdir /s /q \"%s\"", path.as.string);
    return val_bool(system(cmd) == 0);
#else
    char cmd[4096];
    snprintf(cmd, sizeof(cmd), "rm -rf \"%s\"", path.as.string);
    return val_bool(system(cmd) == 0);
#endif
}

Value fs_readdir(Value path) {
    if (path.type != VAL_STRING || !path.as.string) {
        return val_error("fs_readdir: path must be a string");
    }

    Value arr = val_array_init(0);

#ifdef _WIN32
    char pattern[4096];
    snprintf(pattern, sizeof(pattern), "%s\\*", path.as.string);
    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) {
        char msg[512];
        snprintf(msg, sizeof(msg), "fs_readdir: cannot open '%s'", path.as.string);
        val_drop(arr);
        return val_error(msg);
    }
    do {
        if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0) continue;
        Value s = val_string(fd.cFileName);
        arr = val_array_append(arr, s);
        val_drop(s);
    } while (FindNextFileA(h, &fd));
    FindClose(h);
#else
    DIR *dir = opendir(path.as.string);
    if (!dir) {
        char msg[512];
        snprintf(msg, sizeof(msg), "fs_readdir: cannot open '%s': %s", path.as.string, strerror(errno));
        val_drop(arr);
        return val_error(msg);
    }
    struct dirent *entry;
    while ((entry = readdir(dir))) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        Value s = val_string(entry->d_name);
        arr = val_array_append(arr, s);
        val_drop(s);
    }
    closedir(dir);
#endif
    return arr;
}

Value fs_stat(Value path) {
    if (path.type != VAL_STRING || !path.as.string) {
        return val_null();
    }
    WYRM_STAT_T st;
    if (WYRM_STAT(path.as.string, &st) != 0) {
        return val_null();
    }

    int is_dir = 0;
#ifdef _WIN32
    is_dir = (st.st_mode & _S_IFDIR) != 0;
#else
    is_dir = S_ISDIR(st.st_mode);
#endif

    const char *field_names[] = { "size", "is_dir", "exists" };
    Value field_vals[] = {
        val_number((double)st.st_size),
        val_bool(is_dir),
        val_bool(1)
    };
    return val_struct_create("StatResult", 3,
        (const char **)field_names, field_vals);
}

Value fs_copy(Value src, Value dst) {
    if (src.type != VAL_STRING || !src.as.string ||
        dst.type != VAL_STRING || !dst.as.string) {
        return val_bool(0);
    }
    FILE *in  = fopen(src.as.string, "rb");
    if (!in) return val_bool(0);
    FILE *out = fopen(dst.as.string, "wb");
    if (!out) { fclose(in); return val_bool(0); }

    char buf[8192];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
        if (fwrite(buf, 1, n, out) != n) {
            fclose(in); fclose(out);
            return val_bool(0);
        }
    }
    fclose(in);
    fclose(out);
    return val_bool(1);
}

Value fs_move(Value src, Value dst) {
    if (src.type != VAL_STRING || !src.as.string ||
        dst.type != VAL_STRING || !dst.as.string) {
        return val_bool(0);
    }
    return val_bool(rename(src.as.string, dst.as.string) == 0);
}
