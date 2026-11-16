/*
 * wyrm_std_path.c - Wyrm Standard Library: Path Manipulation Implementation
 */
#include "wyrm_std_path.h"
#include "../wyrm_core.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#  include <direct.h>
#  define WYRM_SEP_CHAR '\\'
#  define WYRM_SEP_STR  "\\"
#  define WYRM_GETCWD   _getcwd
static int wyrm_is_sep(char c) { return c == '/' || c == '\\'; }
#else
#  include <unistd.h>
#  define WYRM_SEP_CHAR '/'
#  define WYRM_SEP_STR  "/"
#  define WYRM_GETCWD   getcwd
static int wyrm_is_sep(char c) { return c == '/'; }
#endif

/* ------------------------------------------------------------------ */

Value path_join(Value a, Value b) {
    if (a.type != VAL_STRING || !a.as.string) return b.type == VAL_STRING ? val_copy(b) : val_null();
    if (b.type != VAL_STRING || !b.as.string) return val_copy(a);

    const char *pa = a.as.string;
    const char *pb = b.as.string;

    /* If b is absolute, return b unchanged */
#ifdef _WIN32
    if (wyrm_is_sep(pb[0]) || (strlen(pb) >= 2 && pb[1] == ':')) {
        return val_string(pb);
    }
#else
    if (pb[0] == '/') {
        return val_string(pb);
    }
#endif

    size_t la = strlen(pa);
    size_t lb = strlen(pb);
    /* Remove trailing separators from a */
    while (la > 1 && wyrm_is_sep(pa[la - 1])) la--;

    /* Build result */
    char *buf = (char *)malloc(la + lb + 2);
    if (!buf) return val_null();
    memcpy(buf, pa, la);
    buf[la] = WYRM_SEP_CHAR;
    memcpy(buf + la + 1, pb, lb);
    buf[la + lb + 1] = '\0';

    Value result = val_string(buf);
    free(buf);
    return result;
}

Value path_basename(Value p) {
    if (p.type != VAL_STRING || !p.as.string) return val_string("");
    const char *s = p.as.string;
    size_t len = strlen(s);
    if (len == 0) return val_string("");

    /* Strip trailing separators */
    size_t end = len;
    while (end > 1 && wyrm_is_sep(s[end - 1])) end--;

    /* Find last separator */
    size_t start = 0;
    for (size_t i = 0; i < end; i++) {
        if (wyrm_is_sep(s[i])) start = i + 1;
    }

    size_t blen = end - start;
    char *buf = (char *)malloc(blen + 1);
    if (!buf) return val_null();
    memcpy(buf, s + start, blen);
    buf[blen] = '\0';
    Value result = val_string(buf);
    free(buf);
    return result;
}

Value path_dirname(Value p) {
    if (p.type != VAL_STRING || !p.as.string) return val_string(".");
    const char *s = p.as.string;
    size_t len = strlen(s);
    if (len == 0) return val_string(".");

    /* Strip trailing separators */
    size_t end = len;
    while (end > 1 && wyrm_is_sep(s[end - 1])) end--;

    /* Find last separator */
    size_t last_sep = 0;
    int found = 0;
    for (size_t i = 0; i < end; i++) {
        if (wyrm_is_sep(s[i])) { last_sep = i; found = 1; }
    }

    if (!found) return val_string(".");

    /* Include the separator only if it is the root */
    size_t dlen = (last_sep == 0) ? 1 : last_sep;
    char *buf = (char *)malloc(dlen + 1);
    if (!buf) return val_null();
    memcpy(buf, s, dlen);
    buf[dlen] = '\0';
    Value result = val_string(buf);
    free(buf);
    return result;
}

Value path_ext(Value p) {
    if (p.type != VAL_STRING || !p.as.string) return val_string("");
    const char *s = p.as.string;

    /* Get basename first */
    Value base = path_basename(p);
    const char *b = (base.type == VAL_STRING && base.as.string) ? base.as.string : s;
    const char *dot = strrchr(b, '.');
    Value result;
    if (!dot || dot == b) {
        result = val_string("");
    } else {
        result = val_string(dot);
    }
    val_drop(base);
    return result;
}

Value path_abs(Value p) {
    if (p.type != VAL_STRING || !p.as.string) return val_null();
    char buf[4096];
#ifdef _WIN32
    if (_fullpath(buf, p.as.string, sizeof(buf))) {
        return val_string(buf);
    }
#else
    if (realpath(p.as.string, buf)) {
        return val_string(buf);
    }
    /* realpath may fail if the path does not exist yet - fall back to cwd join */
    char cwd[2048];
    if (!WYRM_GETCWD(cwd, sizeof(cwd))) return val_copy(p);
    Value base = val_string(cwd);
    Value result = path_join(base, p);
    val_drop(base);
    return result;
#endif
    return val_copy(p);
}

Value path_is_abs(Value p) {
    if (p.type != VAL_STRING || !p.as.string) return val_bool(0);
    const char *s = p.as.string;
#ifdef _WIN32
    if (wyrm_is_sep(s[0])) return val_bool(1);
    if (strlen(s) >= 2 && s[1] == ':') return val_bool(1);
    return val_bool(0);
#else
    return val_bool(s[0] == '/');
#endif
}

Value path_stem(Value p) {
    Value base = path_basename(p);
    if (base.type != VAL_STRING || !base.as.string) return base;
    const char *s = base.as.string;
    const char *dot = strrchr(s, '.');
    Value result;
    if (!dot || dot == s) {
        result = val_copy(base);
    } else {
        size_t len = (size_t)(dot - s);
        char *buf = (char *)malloc(len + 1);
        if (!buf) { val_drop(base); return val_null(); }
        memcpy(buf, s, len);
        buf[len] = '\0';
        result = val_string(buf);
        free(buf);
    }
    val_drop(base);
    return result;
}

Value path_with_ext(Value p, Value ext) {
    Value stem_dir = path_dirname(p);
    Value stem     = path_stem(path_basename(p));
    const char *e  = (ext.type == VAL_STRING && ext.as.string) ? ext.as.string : "";

    /* Ensure extension starts with a dot */
    char ext_buf[256];
    if (*e && *e != '.') {
        snprintf(ext_buf, sizeof(ext_buf), ".%s", e);
        e = ext_buf;
    }

    Value base_stem_ext;
    {
        const char *sn = (stem.type == VAL_STRING && stem.as.string) ? stem.as.string : "";
        size_t slen = strlen(sn);
        size_t elen = strlen(e);
        char *buf = (char *)malloc(slen + elen + 1);
        if (!buf) { val_drop(stem_dir); val_drop(stem); return val_null(); }
        memcpy(buf, sn, slen);
        memcpy(buf + slen, e, elen);
        buf[slen + elen] = '\0';
        base_stem_ext = val_string(buf);
        free(buf);
    }

    Value result;
    const char *dir = (stem_dir.type == VAL_STRING && stem_dir.as.string) ? stem_dir.as.string : ".";
    if (strcmp(dir, ".") == 0) {
        result = val_copy(base_stem_ext);
    } else {
        result = path_join(stem_dir, base_stem_ext);
    }
    val_drop(stem_dir);
    val_drop(stem);
    val_drop(base_stem_ext);
    return result;
}
