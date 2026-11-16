/*
 * wyrm_std_os.c - Wyrm Standard Library: OS/Process Module Implementation
 */
#include "wyrm_std_os.h"
#include "../wyrm_core.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#  include <direct.h>
#  include <windows.h>
#  define WYRM_GETCWD _getcwd
#  define WYRM_CHDIR  _chdir
#  define WYRM_PLATFORM "windows"
#  define WYRM_SEP "\\"
#else
#  include <unistd.h>
#  include <errno.h>
#  ifdef __APPLE__
#    define WYRM_PLATFORM "macos"
#  else
#    define WYRM_PLATFORM "linux"
#  endif
#  define WYRM_GETCWD getcwd
#  define WYRM_CHDIR  chdir
#  define WYRM_SEP "/"
extern char **environ;
#endif

Value os_args(void) {
    return val_copy(val_sys_args());
}

Value os_env(Value name) {
    if (name.type != VAL_STRING || !name.as.string) {
        return val_null();
    }
    const char *v = getenv(name.as.string);
    if (!v) return val_null();
    return val_string(v);
}

Value os_setenv(Value name, Value val) {
    if (name.type != VAL_STRING || !name.as.string) {
        return val_bool(0);
    }
    const char *n = name.as.string;
    const char *v = (val.type == VAL_STRING && val.as.string) ? val.as.string : "";

#ifdef _WIN32
    char buf[8192];
    snprintf(buf, sizeof(buf), "%s=%s", n, v);
    int r = _putenv(buf);
    return val_bool(r == 0);
#else
    int r = setenv(n, v, 1);
    return val_bool(r == 0);
#endif
}

Value os_unsetenv(Value name) {
    if (name.type != VAL_STRING || !name.as.string) {
        return val_bool(0);
    }
#ifdef _WIN32
    char buf[4096];
    snprintf(buf, sizeof(buf), "%s=", name.as.string);
    int r = _putenv(buf);
    return val_bool(r == 0);
#else
    int r = unsetenv(name.as.string);
    return val_bool(r == 0);
#endif
}

Value os_cwd(void) {
    char buf[4096];
    if (!WYRM_GETCWD(buf, sizeof(buf))) {
        return val_null();
    }
    return val_string(buf);
}

Value os_chdir(Value path) {
    if (path.type != VAL_STRING || !path.as.string) {
        return val_bool(0);
    }
    int r = WYRM_CHDIR(path.as.string);
    return val_bool(r == 0);
}

Value os_exit(Value code) {
    int exit_code = 0;
    if (code.type == VAL_NUMBER) {
        exit_code = (int)code.as.number;
    }
    exit(exit_code);
    /* unreachable - satisfies compiler */
    return val_null();
}

Value os_exec(Value cmd) {
    if (cmd.type != VAL_STRING || !cmd.as.string) {
        return val_number(-1.0);
    }
    int r = system(cmd.as.string);
    return val_number((double)r);
}

Value os_platform(void) {
    return val_string(WYRM_PLATFORM);
}

Value os_sep(void) {
    return val_string(WYRM_SEP);
}

Value os_environ(void) {
    Value map = val_array_init(0);

#ifdef _WIN32
    LPCH env_str = GetEnvironmentStrings();
    if (!env_str) return map;
    val_drop(map);
    map = val_array_init(0);

    LPCH p = env_str;
    while (*p) {
        const char *eq = strchr(p, '=');
        if (eq && eq != p) {
            size_t key_len = (size_t)(eq - p);
            char *key = (char *)malloc(key_len + 1);
            if (key) {
                memcpy(key, p, key_len);
                key[key_len] = '\0';

                /* Build a 2-element array [key, value] */
                Value kv = val_array_init(2, val_string(key), val_string(eq + 1));
                map = val_array_append(map, kv);
                val_drop(kv);
                free(key);
            }
        }
        p += strlen(p) + 1;
    }
    FreeEnvironmentStrings(env_str);
#else
    char **e = environ;
    val_drop(map);
    map = val_array_init(0);
    if (!e) return map;
    while (*e) {
        const char *eq = strchr(*e, '=');
        if (eq) {
            size_t key_len = (size_t)(eq - *e);
            char *key = (char *)malloc(key_len + 1);
            if (key) {
                memcpy(key, *e, key_len);
                key[key_len] = '\0';
                Value kv = val_array_init(2, val_string(key), val_string(eq + 1));
                map = val_array_append(map, kv);
                val_drop(kv);
                free(key);
            }
        }
        e++;
    }
#endif
    return map;
}
