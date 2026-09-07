/*
 * wyrm_std_yaml.c - Wyrm Standard Library: YAML Implementation
 *
 * Block-style YAML parser covering:
 *   - Mappings:  key: value
 *   - Sequences: - item
 *   - Scalars:   strings, numbers, booleans, null
 *   - Quoted strings: 'single' and "double"
 *   - Comments: # comment
 *   - Multi-level nesting via indentation
 */
#include "wyrm_std_yaml.h"
#include "wyrm_std_json.h"   /* for JSON_OBJ_SENTINEL and json_is_object */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>

/* --------------------------------------------------------------------------
 * Line-oriented YAML parser
 * -------------------------------------------------------------------------- */
typedef struct {
    char **lines;
    int    count;
    int    pos;       /* current line index */
    char   error[256];
} YamlParser;

/* Forward declarations */
static Value yaml_parse_node(YamlParser *p, int base_indent);
static Value yaml_parse_scalar(const char *s);

/* --------------------------------------------------------------------------
 * Split input into lines (no strdup of entire buffer - parse in-place)
 * -------------------------------------------------------------------------- */
static YamlParser *yaml_parser_create(const char *src) {
    /* Count newlines to allocate line array */
    int count = 1;
    for (const char *c = src; *c; c++) {
        if (*c == '\n') count++;
    }

    YamlParser *p = malloc(sizeof(YamlParser));
    if (!p) return NULL;
    p->lines = malloc(sizeof(char *) * (size_t)count);
    if (!p->lines) { free(p); return NULL; }
    p->count  = 0;
    p->pos    = 0;
    p->error[0] = '\0';

    /* Duplicate input and split on newlines */
    char *dup = strdup(src);
    if (!dup) { free(p->lines); free(p); return NULL; }

    char *cursor = dup;
    while (*cursor) {
        p->lines[p->count++] = cursor;
        char *nl = strchr(cursor, '\n');
        if (!nl) break;
        *nl   = '\0';
        cursor = nl + 1;
        /* Strip carriage return */
        if (cursor > dup && *(cursor - 2) == '\r') *(cursor - 2) = '\0';
    }

    /* Store dup pointer in lines[0] for freeing later (lines[0] is start of dup) */
    return p;
}

static void yaml_parser_free(YamlParser *p) {
    if (!p) return;
    if (p->count > 0) free(p->lines[0]); /* free the strdup */
    free(p->lines);
    free(p);
}

/* Count leading spaces (indentation) */
static int count_indent(const char *line) {
    int n = 0;
    while (line[n] == ' ') n++;
    return n;
}

/* Skip blank lines and comment lines; return current line or NULL at end */
static const char *current_line(YamlParser *p) {
    while (p->pos < p->count) {
        const char *ln = p->lines[p->pos];
        /* Skip blank/comment lines */
        int i = 0;
        while (ln[i] == ' ') i++;
        if (ln[i] == '\0' || ln[i] == '#') { p->pos++; continue; }
        return ln;
    }
    return NULL;
}

/* Parse a scalar value from a string token */
static Value yaml_parse_scalar(const char *s) {
    if (!s || *s == '\0') return val_null();

    /* Strip inline comment */
    char buf[1024];
    strncpy(buf, s, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    char *hash = strchr(buf, '#');
    if (hash && (hash == buf || *(hash - 1) == ' ')) {
        *hash = '\0';
        /* Trim trailing space */
        size_t len = strlen(buf);
        while (len > 0 && buf[len - 1] == ' ') buf[--len] = '\0';
    }

    /* Quoted string: single quotes */
    if (buf[0] == '\'' && buf[strlen(buf) - 1] == '\'') {
        size_t len = strlen(buf) - 2;
        char *inner = malloc(len + 1);
        if (!inner) return val_null();
        memcpy(inner, buf + 1, len);
        inner[len] = '\0';
        Value v = val_string(inner);
        free(inner);
        return v;
    }

    /* Quoted string: double quotes */
    if (buf[0] == '"' && buf[strlen(buf) - 1] == '"') {
        size_t len = strlen(buf) - 2;
        char *inner = malloc(len + 1);
        if (!inner) return val_null();
        memcpy(inner, buf + 1, len);
        inner[len] = '\0';
        /* Process simple \n, \t escapes */
        char *out = inner;
        for (const char *in = inner; *in; ) {
            if (*in == '\\' && *(in + 1)) {
                in++;
                if (*in == 'n') { *out++ = '\n'; in++; }
                else if (*in == 't') { *out++ = '\t'; in++; }
                else { *out++ = *in++; }
            } else {
                *out++ = *in++;
            }
        }
        *out = '\0';
        Value v = val_string(inner);
        free(inner);
        return v;
    }

    /* Boolean */
    if (strcmp(buf, "true") == 0 || strcmp(buf, "True") == 0 || strcmp(buf, "TRUE") == 0)
        return val_bool(true);
    if (strcmp(buf, "false") == 0 || strcmp(buf, "False") == 0 || strcmp(buf, "FALSE") == 0)
        return val_bool(false);

    /* Null */
    if (strcmp(buf, "null") == 0 || strcmp(buf, "Null") == 0 ||
        strcmp(buf, "NULL") == 0 || strcmp(buf, "~") == 0 || buf[0] == '\0')
        return val_null();

    /* Number */
    char *end = NULL;
    errno = 0;
    double d = strtod(buf, &end);
    if (errno == 0 && end != buf && *end == '\0') {
        return val_number(d);
    }

    /* Plain string */
    return val_string(buf);
}

/* --------------------------------------------------------------------------
 * Recursive node parser
 * -------------------------------------------------------------------------- */
static Value yaml_parse_node(YamlParser *p, int base_indent) {
    const char *ln = current_line(p);
    if (!ln) return val_null();

    int this_indent = count_indent(ln);
    if (this_indent < base_indent) return val_null(); /* de-indented - caller handles */

    /* Detect sequence block: line starts with "- " at this_indent */
    if (ln[this_indent] == '-' && (ln[this_indent + 1] == ' ' || ln[this_indent + 1] == '\0')) {
        Value arr = val_array_create(0);
        while ((ln = current_line(p)) != NULL) {
            int ind = count_indent(ln);
            if (ind < this_indent) break;           /* de-indented: end of sequence */
            if (ind != this_indent) break;
            if (ln[ind] != '-') break;

            p->pos++; /* consume this "- " line */
            const char *item_str = ln + ind + 1;
            while (*item_str == ' ') item_str++;

            if (*item_str == '\0' || *item_str == '#') {
                /* Inline value is empty - next indented block is the value */
                Value child = yaml_parse_node(p, this_indent + 2);
                arr = val_array_append(arr, child);
            } else {
                /* Inline scalar value */
                Value item = yaml_parse_scalar(item_str);
                arr = val_array_append(arr, item);
            }
        }
        return arr;
    }

    /* Detect mapping: line contains ": " or ends with ":" */
    const char *colon = strchr(ln + this_indent, ':');
    if (colon && (colon[1] == ' ' || colon[1] == '\0' || colon[1] == '\n')) {
        Value obj = json_object();
        while ((ln = current_line(p)) != NULL) {
            int ind = count_indent(ln);
            if (ind < this_indent) break;           /* de-indented: end of mapping */
            if (ind != this_indent) break;

            colon = strchr(ln + ind, ':');
            if (!colon || (colon[1] != ' ' && colon[1] != '\0')) break;

            /* Extract key */
            size_t key_len = (size_t)(colon - (ln + ind));
            char key_buf[256];
            if (key_len >= sizeof(key_buf)) key_len = sizeof(key_buf) - 1;
            memcpy(key_buf, ln + ind, key_len);
            key_buf[key_len] = '\0';
            Value key = val_string(key_buf);

            p->pos++; /* consume key line */

            const char *value_str = colon + 1;
            while (*value_str == ' ') value_str++;

            Value val;
            if (*value_str == '\0' || *value_str == '#') {
                /* Value is on next indented block */
                val = yaml_parse_node(p, this_indent + 2);
            } else {
                val = yaml_parse_scalar(value_str);
            }

            obj = json_set(obj, key, val);
            val_drop(key);
            val_drop(val);
        }
        return obj;
    }

    /* Single scalar */
    p->pos++;
    return yaml_parse_scalar(ln + this_indent);
}

/* --------------------------------------------------------------------------
 * Public: yaml_parse
 * -------------------------------------------------------------------------- */
Value yaml_parse(Value yaml_str) {
    if (yaml_str.type != VAL_STRING || !yaml_str.as.string) {
        return val_error("yaml_parse: argument must be a string");
    }

    YamlParser *p = yaml_parser_create(yaml_str.as.string);
    if (!p) return val_error("yaml_parse: out of memory");

    /* Skip YAML document start marker "---" */
    const char *first = current_line(p);
    if (first && strncmp(first, "---", 3) == 0) p->pos++;

    Value result = yaml_parse_node(p, 0);
    yaml_parser_free(p);
    return result;
}

/* --------------------------------------------------------------------------
 * Encoder: write YAML output
 * -------------------------------------------------------------------------- */
typedef struct { char *buf; size_t sz; size_t cap; } YamlBuf;

static void yb_init(YamlBuf *b) {
    b->cap = 256; b->sz = 0; b->buf = malloc(b->cap);
    if (b->buf) b->buf[0] = '\0';
}

static void yb_append(YamlBuf *b, const char *s, size_t n) {
    if (!b->buf) return;
    while (b->sz + n + 1 > b->cap) { b->cap *= 2; b->buf = realloc(b->buf, b->cap); if (!b->buf) return; }
    memcpy(b->buf + b->sz, s, n);
    b->sz += n;
    b->buf[b->sz] = '\0';
}

static void yb_str(YamlBuf *b, const char *s) { if (s) yb_append(b, s, strlen(s)); }
static void yb_indent(YamlBuf *b, int depth) { for (int i = 0; i < depth * 2; i++) yb_append(b, " ", 1); }

static void encode_yaml(YamlBuf *b, Value v, int depth);

static void encode_yaml_scalar_str(YamlBuf *b, const char *s) {
    /* Quote strings that look like keywords or contain special characters */
    int needs_quote = 0;
    if (strcmp(s, "true") == 0 || strcmp(s, "false") == 0 ||
        strcmp(s, "null") == 0 || strcmp(s, "~") == 0) needs_quote = 1;
    if (!needs_quote) {
        for (const char *c = s; *c; c++) {
            if (*c == ':' || *c == '#' || *c == '\n' || *c == '\'') { needs_quote = 1; break; }
        }
    }
    if (needs_quote) {
        yb_append(b, "'", 1);
        for (const char *c = s; *c; c++) {
            if (*c == '\'') yb_append(b, "''", 2);
            else { char ch = *c; yb_append(b, &ch, 1); }
        }
        yb_append(b, "'", 1);
    } else {
        yb_str(b, s);
    }
}

static void encode_yaml(YamlBuf *b, Value v, int depth) {
    switch (v.type) {
        case VAL_NULL:    yb_str(b, "null"); break;
        case VAL_BOOL:    yb_str(b, v.as.boolean ? "true" : "false"); break;
        case VAL_NUMBER: {
            char num[64];
            double d = v.as.number;
            if (d == (long long)d && !isinf(d) && !isnan(d)) snprintf(num, sizeof(num), "%.0f", d);
            else if (isnan(d) || isinf(d))                    snprintf(num, sizeof(num), "null");
            else                                               snprintf(num, sizeof(num), "%.17g", d);
            yb_str(b, num);
            break;
        }
        case VAL_STRING:
            encode_yaml_scalar_str(b, v.as.string ? v.as.string : "");
            break;
        case VAL_ARRAY:
            if (!v.as.array || v.as.array->size == 0) {
                yb_str(b, json_is_object(v) ? "{}" : "[]");
                break;
            }
            if (json_is_object(v)) {
                /* Mapping */
                for (int i = 1; i + 1 < v.as.array->size; i += 2) {
                    yb_append(b, "\n", 1);
                    yb_indent(b, depth);
                    encode_yaml_scalar_str(b, v.as.array->data[i].as.string);
                    yb_str(b, ": ");
                    Value child = v.as.array->data[i + 1];
                    if (child.type == VAL_ARRAY && child.as.array && child.as.array->size > 0) {
                        encode_yaml(b, child, depth + 1);
                    } else {
                        encode_yaml(b, child, depth + 1);
                    }
                }
            } else {
                /* Sequence */
                for (int i = 0; i < v.as.array->size; i++) {
                    yb_append(b, "\n", 1);
                    yb_indent(b, depth);
                    yb_str(b, "- ");
                    Value elem = v.as.array->data[i];
                    if (elem.type == VAL_ARRAY && elem.as.array && elem.as.array->size > 0) {
                        encode_yaml(b, elem, depth + 1);
                    } else {
                        encode_yaml(b, elem, depth);
                    }
                }
            }
            break;
        default:
            yb_str(b, "null");
            break;
    }
}

/* --------------------------------------------------------------------------
 * Public: yaml_encode
 * -------------------------------------------------------------------------- */
Value yaml_encode(Value val) {
    YamlBuf b;
    yb_init(&b);
    if (!b.buf) return val_error("yaml_encode: out of memory");
    yb_str(&b, "---");
    encode_yaml(&b, val, 0);
    yb_append(&b, "\n", 1);
    Value result = val_string(b.buf);
    free(b.buf);
    return result;
}
