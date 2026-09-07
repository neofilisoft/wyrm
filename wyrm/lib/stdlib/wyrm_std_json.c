/*
 * wyrm_std_json.c - Wyrm Standard Library: JSON Implementation
 *
 * Recursive-descent JSON parser (RFC 8259 compliant) and encoder.
 * All parsing is done in a single pass over the input string.
 * Encodes Wyrm Values back to JSON text.
 */
#include "wyrm_std_json.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <errno.h>

/* --------------------------------------------------------------------------
 * JSON Object Sentinel
 * -------------------------------------------------------------------------- */
#define JSON_OBJ_SENTINEL "__json_obj__"

/* --------------------------------------------------------------------------
 * Parser State
 * -------------------------------------------------------------------------- */
typedef struct {
    const char *src;
    size_t      pos;
    size_t      len;
    char        error[256];
} JsonParser;

/* Forward declarations */
static Value parse_value(JsonParser *p);
static Value parse_string(JsonParser *p);
static Value parse_number(JsonParser *p);
static Value parse_array(JsonParser *p);
static Value parse_object(JsonParser *p);

/* --------------------------------------------------------------------------
 * Parser utilities
 * -------------------------------------------------------------------------- */
static void skip_whitespace(JsonParser *p) {
    while (p->pos < p->len && isspace((unsigned char)p->src[p->pos])) {
        p->pos++;
    }
}

static char peek(JsonParser *p) {
    if (p->pos >= p->len) return '\0';
    return p->src[p->pos];
}

static char advance(JsonParser *p) {
    if (p->pos >= p->len) return '\0';
    return p->src[p->pos++];
}

static int expect_char(JsonParser *p, char c) {
    skip_whitespace(p);
    if (peek(p) != c) {
        snprintf(p->error, sizeof(p->error),
                 "JSON parse error at position %zu: expected '%c', got '%c'",
                 p->pos, c, peek(p));
        return 0;
    }
    p->pos++;
    return 1;
}

/* --------------------------------------------------------------------------
 * Parse a JSON string literal (handles \n, \t, \", \\, \uXXXX)
 * -------------------------------------------------------------------------- */
static Value parse_string(JsonParser *p) {
    if (!expect_char(p, '"')) return val_error(p->error);

    size_t cap = 64;
    size_t sz  = 0;
    char  *buf = malloc(cap);
    if (!buf) { snprintf(p->error, sizeof(p->error), "JSON OOM in string"); return val_error(p->error); }

    while (p->pos < p->len) {
        char c = advance(p);
        if (c == '"') {
            buf[sz] = '\0';
            Value result = val_string(buf);
            free(buf);
            return result;
        }
        if (c == '\\') {
            if (p->pos >= p->len) break;
            char esc = advance(p);
            char decoded = '\0';
            if      (esc == '"')  decoded = '"';
            else if (esc == '\\') decoded = '\\';
            else if (esc == '/')  decoded = '/';
            else if (esc == 'b')  decoded = '\b';
            else if (esc == 'f')  decoded = '\f';
            else if (esc == 'n')  decoded = '\n';
            else if (esc == 'r')  decoded = '\r';
            else if (esc == 't')  decoded = '\t';
            else if (esc == 'u') {
                /* \uXXXX -> decode as UTF-8 (basic BMP only) */
                if (p->pos + 4 > p->len) break;
                unsigned int cp = 0;
                for (int i = 0; i < 4; i++) {
                    char h = advance(p);
                    cp <<= 4;
                    if (h >= '0' && h <= '9')      cp |= (unsigned)(h - '0');
                    else if (h >= 'a' && h <= 'f') cp |= (unsigned)(h - 'a' + 10);
                    else if (h >= 'A' && h <= 'F') cp |= (unsigned)(h - 'A' + 10);
                }
                /* Encode as UTF-8 */
                if (sz + 4 >= cap) { cap *= 2; buf = realloc(buf, cap); if (!buf) break; }
                if (cp < 0x80) {
                    buf[sz++] = (char)cp;
                } else if (cp < 0x800) {
                    buf[sz++] = (char)(0xC0 | (cp >> 6));
                    buf[sz++] = (char)(0x80 | (cp & 0x3F));
                } else {
                    buf[sz++] = (char)(0xE0 | (cp >> 12));
                    buf[sz++] = (char)(0x80 | ((cp >> 6) & 0x3F));
                    buf[sz++] = (char)(0x80 | (cp & 0x3F));
                }
                continue;
            }
            if (sz + 1 >= cap) { cap *= 2; buf = realloc(buf, cap); if (!buf) break; }
            buf[sz++] = decoded;
        } else {
            if (sz + 1 >= cap) { cap *= 2; buf = realloc(buf, cap); if (!buf) break; }
            buf[sz++] = c;
        }
    }

    free(buf);
    snprintf(p->error, sizeof(p->error), "JSON parse error: unterminated string");
    return val_error(p->error);
}

/* --------------------------------------------------------------------------
 * Parse a JSON number
 * -------------------------------------------------------------------------- */
static Value parse_number(JsonParser *p) {
    const char *start = p->src + p->pos;
    /* Consume optional sign, digits, decimal, exponent */
    if (peek(p) == '-') p->pos++;
    while (p->pos < p->len && isdigit((unsigned char)p->src[p->pos])) p->pos++;
    if (peek(p) == '.') {
        p->pos++;
        while (p->pos < p->len && isdigit((unsigned char)p->src[p->pos])) p->pos++;
    }
    if (peek(p) == 'e' || peek(p) == 'E') {
        p->pos++;
        if (peek(p) == '+' || peek(p) == '-') p->pos++;
        while (p->pos < p->len && isdigit((unsigned char)p->src[p->pos])) p->pos++;
    }
    char *end = NULL;
    errno = 0;
    double d = strtod(start, &end);
    if (errno != 0 || end == (char *)start) {
        snprintf(p->error, sizeof(p->error), "JSON parse error: invalid number at position %zu", p->pos);
        return val_error(p->error);
    }
    return val_number(d);
}

/* --------------------------------------------------------------------------
 * Parse a JSON array
 * -------------------------------------------------------------------------- */
static Value parse_array(JsonParser *p) {
    if (!expect_char(p, '[')) return val_error(p->error);
    skip_whitespace(p);

    Value arr = val_array_create(0);
    if (peek(p) == ']') { p->pos++; return arr; }

    while (p->pos < p->len) {
        skip_whitespace(p);
        Value elem = parse_value(p);
        if (elem.type == VAL_ERROR) { val_drop(arr); return elem; }
        arr = val_array_append(arr, elem);
        skip_whitespace(p);
        if (peek(p) == ']') { p->pos++; return arr; }
        if (!expect_char(p, ',')) { val_drop(arr); return val_error(p->error); }
    }

    val_drop(arr);
    snprintf(p->error, sizeof(p->error), "JSON parse error: unterminated array");
    return val_error(p->error);
}

/* --------------------------------------------------------------------------
 * Parse a JSON object -> tagged array [sentinel, k0, v0, k1, v1, ...]
 * -------------------------------------------------------------------------- */
static Value parse_object(JsonParser *p) {
    if (!expect_char(p, '{')) return val_error(p->error);
    skip_whitespace(p);

    /* Start with sentinel at index 0 */
    Value obj = val_array_create(1);
    Value sentinel = val_string(JSON_OBJ_SENTINEL);
    obj.as.array->data[0] = sentinel;
    obj.as.array->size = 1;

    if (peek(p) == '}') { p->pos++; return obj; }

    while (p->pos < p->len) {
        skip_whitespace(p);
        Value key = parse_string(p);
        if (key.type == VAL_ERROR) { val_drop(obj); return key; }
        if (!expect_char(p, ':')) { val_drop(key); val_drop(obj); return val_error(p->error); }
        skip_whitespace(p);
        Value val = parse_value(p);
        if (val.type == VAL_ERROR) { val_drop(key); val_drop(obj); return val; }

        obj = val_array_append(obj, key);
        obj = val_array_append(obj, val);

        skip_whitespace(p);
        if (peek(p) == '}') { p->pos++; return obj; }
        if (!expect_char(p, ',')) { val_drop(obj); return val_error(p->error); }
    }

    val_drop(obj);
    snprintf(p->error, sizeof(p->error), "JSON parse error: unterminated object");
    return val_error(p->error);
}

/* --------------------------------------------------------------------------
 * Top-level value dispatch
 * -------------------------------------------------------------------------- */
static Value parse_value(JsonParser *p) {
    skip_whitespace(p);
    char c = peek(p);

    if (c == '"') return parse_string(p);
    if (c == '[') return parse_array(p);
    if (c == '{') return parse_object(p);

    if (c == 't') {
        if (p->pos + 4 <= p->len && strncmp(p->src + p->pos, "true", 4) == 0) {
            p->pos += 4;
            return val_bool(true);
        }
    }
    if (c == 'f') {
        if (p->pos + 5 <= p->len && strncmp(p->src + p->pos, "false", 5) == 0) {
            p->pos += 5;
            return val_bool(false);
        }
    }
    if (c == 'n') {
        if (p->pos + 4 <= p->len && strncmp(p->src + p->pos, "null", 4) == 0) {
            p->pos += 4;
            return val_null();
        }
    }
    if (c == '-' || isdigit((unsigned char)c)) return parse_number(p);

    snprintf(p->error, sizeof(p->error),
             "JSON parse error at position %zu: unexpected character '%c'", p->pos, c);
    return val_error(p->error);
}

/* --------------------------------------------------------------------------
 * Public: json_parse
 * -------------------------------------------------------------------------- */
Value json_parse(Value json_str) {
    if (json_str.type != VAL_STRING || !json_str.as.string) {
        return val_error("json_parse: argument must be a string");
    }

    JsonParser p;
    p.src = json_str.as.string;
    p.pos = 0;
    p.len = strlen(p.src);
    p.error[0] = '\0';

    Value result = parse_value(&p);
    if (result.type == VAL_ERROR) return result;

    skip_whitespace(&p);
    if (p.pos < p.len) {
        val_drop(result);
        snprintf(p.error, sizeof(p.error),
                 "JSON parse error: trailing content at position %zu", p.pos);
        return val_error(p.error);
    }
    return result;
}

/* --------------------------------------------------------------------------
 * Encoder: dynamic string buffer
 * -------------------------------------------------------------------------- */
typedef struct {
    char  *buf;
    size_t sz;
    size_t cap;
} EncBuf;

static void enc_init(EncBuf *b) {
    b->cap = 256;
    b->sz  = 0;
    b->buf = malloc(b->cap);
    if (b->buf) b->buf[0] = '\0';
}

static void enc_append(EncBuf *b, const char *s, size_t n) {
    if (!b->buf) return;
    while (b->sz + n + 1 > b->cap) {
        b->cap *= 2;
        b->buf = realloc(b->buf, b->cap);
        if (!b->buf) return;
    }
    memcpy(b->buf + b->sz, s, n);
    b->sz += n;
    b->buf[b->sz] = '\0';
}

static void enc_char(EncBuf *b, char c) { enc_append(b, &c, 1); }

static void enc_str_escaped(EncBuf *b, const char *s) {
    enc_char(b, '"');
    if (s) {
        for (size_t i = 0; s[i]; i++) {
            unsigned char uc = (unsigned char)s[i];
            if      (uc == '"')  enc_append(b, "\\\"", 2);
            else if (uc == '\\') enc_append(b, "\\\\", 2);
            else if (uc == '\n') enc_append(b, "\\n", 2);
            else if (uc == '\r') enc_append(b, "\\r", 2);
            else if (uc == '\t') enc_append(b, "\\t", 2);
            else if (uc < 0x20) {
                char esc[7];
                snprintf(esc, sizeof(esc), "\\u%04x", uc);
                enc_append(b, esc, 6);
            } else {
                enc_char(b, (char)uc);
            }
        }
    }
    enc_char(b, '"');
}

static void encode_value(EncBuf *b, Value v, int pretty, int indent_size, int depth);

static void enc_indent(EncBuf *b, int indent_size, int depth) {
    for (int i = 0; i < indent_size * depth; i++) enc_char(b, ' ');
}

static void encode_value(EncBuf *b, Value v, int pretty, int indent_size, int depth) {
    switch (v.type) {
        case VAL_NULL:
            enc_append(b, "null", 4);
            break;
        case VAL_BOOL:
            enc_append(b, v.as.boolean ? "true" : "false", v.as.boolean ? 4 : 5);
            break;
        case VAL_NUMBER: {
            char num[64];
            double d = v.as.number;
            if (d == (long long)d && !isinf(d) && !isnan(d)) {
                snprintf(num, sizeof(num), "%.0f", d);
            } else if (isnan(d) || isinf(d)) {
                snprintf(num, sizeof(num), "null"); /* JSON has no NaN/Inf */
            } else {
                snprintf(num, sizeof(num), "%.17g", d);
            }
            enc_append(b, num, strlen(num));
            break;
        }
        case VAL_STRING:
            enc_str_escaped(b, v.as.string);
            break;
        case VAL_ARRAY:
            if (!v.as.array || v.as.array->size == 0) {
                enc_append(b, "[]", 2);
                break;
            }
            if (json_is_object(v)) {
                /* JSON object: sentinel, k0, v0, k1, v1, ... */
                enc_char(b, '{');
                int first = 1;
                for (int i = 1; i + 1 < v.as.array->size; i += 2) {
                    if (!first) { enc_char(b, ','); if (pretty) enc_char(b, '\n'); }
                    else if (pretty) enc_char(b, '\n');
                    first = 0;
                    if (pretty) enc_indent(b, indent_size, depth + 1);
                    enc_str_escaped(b, v.as.array->data[i].as.string);
                    enc_char(b, ':');
                    if (pretty) enc_char(b, ' ');
                    encode_value(b, v.as.array->data[i + 1], pretty, indent_size, depth + 1);
                }
                if (pretty) { enc_char(b, '\n'); enc_indent(b, indent_size, depth); }
                enc_char(b, '}');
            } else {
                /* JSON array */
                enc_char(b, '[');
                for (int i = 0; i < v.as.array->size; i++) {
                    if (i > 0) { enc_char(b, ','); if (pretty) enc_char(b, '\n'); }
                    else if (pretty) enc_char(b, '\n');
                    if (pretty) enc_indent(b, indent_size, depth + 1);
                    encode_value(b, v.as.array->data[i], pretty, indent_size, depth + 1);
                }
                if (pretty) { enc_char(b, '\n'); enc_indent(b, indent_size, depth); }
                enc_char(b, ']');
            }
            break;
        default:
            enc_append(b, "null", 4);
            break;
    }
}

/* --------------------------------------------------------------------------
 * Public: json_encode
 * -------------------------------------------------------------------------- */
Value json_encode(Value val) {
    EncBuf b;
    enc_init(&b);
    if (!b.buf) return val_error("json_encode: out of memory");
    encode_value(&b, val, 0, 0, 0);
    Value result = val_string(b.buf);
    free(b.buf);
    return result;
}

/* --------------------------------------------------------------------------
 * Public: json_pretty
 * -------------------------------------------------------------------------- */
Value json_pretty(Value val, Value indent_size) {
    int indent = 2;
    if (indent_size.type == VAL_NUMBER) {
        indent = (int)indent_size.as.number;
        if (indent < 0) indent = 0;
        if (indent > 16) indent = 16;
    }
    EncBuf b;
    enc_init(&b);
    if (!b.buf) return val_error("json_pretty: out of memory");
    encode_value(&b, val, 1, indent, 0);
    Value result = val_string(b.buf);
    free(b.buf);
    return result;
}

/* --------------------------------------------------------------------------
 * Public: json_is_object
 * -------------------------------------------------------------------------- */
int json_is_object(Value v) {
    if (v.type != VAL_ARRAY || !v.as.array || v.as.array->size < 1) return 0;
    Value first = v.as.array->data[0];
    if (first.type != VAL_STRING || !first.as.string) return 0;
    return strcmp(first.as.string, JSON_OBJ_SENTINEL) == 0;
}

/* --------------------------------------------------------------------------
 * Public: json_get
 * -------------------------------------------------------------------------- */
Value json_get(Value obj, Value key) {
    if (!json_is_object(obj)) return val_error("json_get: not a JSON object");
    if (key.type != VAL_STRING || !key.as.string) return val_error("json_get: key must be a string");
    for (int i = 1; i + 1 < obj.as.array->size; i += 2) {
        Value k = obj.as.array->data[i];
        if (k.type == VAL_STRING && k.as.string && strcmp(k.as.string, key.as.string) == 0) {
            return val_copy(obj.as.array->data[i + 1]);
        }
    }
    return val_null();
}

/* --------------------------------------------------------------------------
 * Public: json_has
 * -------------------------------------------------------------------------- */
Value json_has(Value obj, Value key) {
    if (!json_is_object(obj)) return val_bool(false);
    if (key.type != VAL_STRING || !key.as.string) return val_bool(false);
    for (int i = 1; i + 1 < obj.as.array->size; i += 2) {
        Value k = obj.as.array->data[i];
        if (k.type == VAL_STRING && k.as.string && strcmp(k.as.string, key.as.string) == 0) {
            return val_bool(true);
        }
    }
    return val_bool(false);
}

/* --------------------------------------------------------------------------
 * Public: json_set
 * -------------------------------------------------------------------------- */
Value json_set(Value obj, Value key, Value val) {
    if (!json_is_object(obj)) return val_error("json_set: not a JSON object");
    if (key.type != VAL_STRING || !key.as.string) return val_error("json_set: key must be a string");

    /* Update existing key */
    for (int i = 1; i + 1 < obj.as.array->size; i += 2) {
        Value k = obj.as.array->data[i];
        if (k.type == VAL_STRING && k.as.string && strcmp(k.as.string, key.as.string) == 0) {
            val_drop(obj.as.array->data[i + 1]);
            obj.as.array->data[i + 1] = val_copy(val);
            return obj;
        }
    }
    /* Append new key-value pair */
    obj = val_array_append(obj, val_copy(key));
    obj = val_array_append(obj, val_copy(val));
    return obj;
}

/* --------------------------------------------------------------------------
 * Public: json_object
 * -------------------------------------------------------------------------- */
Value json_object(void) {
    Value obj = val_array_create(1);
    Value sentinel = val_string(JSON_OBJ_SENTINEL);
    val_drop(obj.as.array->data[0]);
    obj.as.array->data[0] = sentinel;
    obj.as.array->size = 1;
    return obj;
}
