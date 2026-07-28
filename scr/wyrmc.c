#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define WYRMPATH_SEP '\\'
#define mkdir_one(path) _mkdir(path)
#else
#include <unistd.h>
#define WYRMPATH_SEP '/'
#define mkdir_one(path) mkdir(path, 0755)
#endif

#define WYRMC_VERSION "2.2"
#define MAX_LINES 8192
#define MAX_VARS 512
#define MAX_NAME 96
#define MAX_TEXT 4096

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

typedef enum { VAL_NULL, VAL_BOOL, VAL_NUMBER, VAL_STRING } ValueType;

typedef struct {
    ValueType type;
    double number;
    bool boolean;
    char *string;
} Value;

typedef struct {
    char name[MAX_NAME];
    Value value;
} Var;

typedef struct {
    Var vars[MAX_VARS];
    int count;
} Env;

typedef struct {
    char **items;
    int count;
} Lines;

static char *xstrdup(const char *s) {
    size_t n = strlen(s) + 1;
    char *p = (char*)malloc(n);
    if (!p) { fprintf(stderr, "out of memory\n"); exit(1); }
    memcpy(p, s, n);
    return p;
}

static char *substr_dup(const char *s, size_t n) {
    char *p = (char*)malloc(n + 1);
    if (!p) { fprintf(stderr, "out of memory\n"); exit(1); }
    memcpy(p, s, n);
    p[n] = '\0';
    return p;
}

static char *trim(char *s) {
    while (isspace((unsigned char)*s)) s++;
    char *end = s + strlen(s);
    while (end > s && isspace((unsigned char)end[-1])) end--;
    *end = '\0';
    return s;
}

static bool starts_with(const char *s, const char *prefix) {
    return strncmp(s, prefix, strlen(prefix)) == 0;
}

static Value val_null(void) { Value v = { VAL_NULL, 0, false, NULL }; return v; }
static Value val_bool(bool b) { Value v = { VAL_BOOL, 0, b, NULL }; return v; }
static Value val_number(double n) { Value v = { VAL_NUMBER, n, false, NULL }; return v; }
static Value val_string(const char *s) { Value v = { VAL_STRING, 0, false, xstrdup(s ? s : "") }; return v; }

static char *value_to_string(Value v) {
    char buf[128];
    switch (v.type) {
        case VAL_NULL: return xstrdup("null");
        case VAL_BOOL: return xstrdup(v.boolean ? "true" : "false");
        case VAL_NUMBER:
            if (fabs(v.number - floor(v.number)) < 0.0000001) snprintf(buf, sizeof(buf), "%.0f", v.number);
            else snprintf(buf, sizeof(buf), "%g", v.number);
            return xstrdup(buf);
        case VAL_STRING: return xstrdup(v.string ? v.string : "");
    }
    return xstrdup("");
}

static bool value_truthy(Value v) {
    switch (v.type) {
        case VAL_NULL: return false;
        case VAL_BOOL: return v.boolean;
        case VAL_NUMBER: return v.number != 0.0;
        case VAL_STRING: return v.string && v.string[0] != '\0';
    }
    return false;
}

static Value env_get(Env *env, const char *name) {
    for (int i = env->count - 1; i >= 0; --i) {
        if (strcmp(env->vars[i].name, name) == 0) return env->vars[i].value;
    }
    if (strcmp(name, "true") == 0) return val_bool(true);
    if (strcmp(name, "false") == 0) return val_bool(false);
    if (strcmp(name, "null") == 0) return val_null();
    fprintf(stderr, "Runtime Error: undefined variable '%s'\n", name);
    exit(1);
}

static void env_set(Env *env, const char *name, Value value) {
    for (int i = env->count - 1; i >= 0; --i) {
        if (strcmp(env->vars[i].name, name) == 0) {
            env->vars[i].value = value;
            return;
        }
    }
    if (env->count >= MAX_VARS) { fprintf(stderr, "Runtime Error: too many variables\n"); exit(1); }
    snprintf(env->vars[env->count].name, sizeof(env->vars[env->count].name), "%s", name);
    env->vars[env->count].value = value;
    env->count++;
}

static char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "Error: cannot open '%s': %s\n", path, strerror(errno)); exit(1); }
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = (char*)malloc((size_t)n + 1);
    if (!buf) { fprintf(stderr, "out of memory\n"); exit(1); }
    if (n > 0 && fread(buf, 1, (size_t)n, f) != (size_t)n) { fprintf(stderr, "Error: failed reading '%s'\n", path); exit(1); }
    buf[n] = '\0';
    fclose(f);
    return buf;
}

static void lines_push(Lines *lines, const char *s) {
    if (lines->count >= MAX_LINES) { fprintf(stderr, "Parser Error: too many lines\n"); exit(1); }
    char *copy = xstrdup(s);
    char *t = trim(copy);
    if (*t) lines->items[lines->count++] = xstrdup(t);
    free(copy);
}

static Lines tokenize_lines(const char *source) {
    Lines lines;
    lines.items = (char**)calloc(MAX_LINES, sizeof(char*));
    lines.count = 0;
    char buf[MAX_TEXT];
    int bi = 0;
    bool in_string = false;
    char quote = 0;
    bool line_comment = false;

    for (const char *p = source; ; ++p) {
        char c = *p;
        char next = p[1];
        if (line_comment) {
            if (c == '\n' || c == '\0') line_comment = false;
            else continue;
        }
        if (!in_string && c == '/' && next == '/') { line_comment = true; ++p; continue; }
        if (c == '\0' || (!in_string && (c == '\n' || c == '\r'))) {
            buf[bi] = '\0';
            lines_push(&lines, buf);
            bi = 0;
            if (c == '\0') break;
            continue;
        }
        if (!in_string && (c == '{' || c == '}')) {
            buf[bi] = '\0';
            lines_push(&lines, buf);
            bi = 0;
            char brace[2] = { c, 0 };
            lines_push(&lines, brace);
            continue;
        }
        if ((c == '"' || c == '\'') && (p == source || p[-1] != '\\')) {
            if (!in_string) { in_string = true; quote = c; }
            else if (quote == c) in_string = false;
        }
        if (bi < MAX_TEXT - 1) buf[bi++] = c;
    }
    return lines;
}

static int find_top_operator(const char *expr, const char **ops, int op_count) {
    int paren = 0;
    bool in_string = false;
    char quote = 0;
    int best = -1;
    for (int i = (int)strlen(expr) - 1; i >= 0; --i) {
        char c = expr[i];
        if ((c == '"' || c == '\'') && (i == 0 || expr[i-1] != '\\')) {
            if (!in_string) { in_string = true; quote = c; }
            else if (quote == c) in_string = false;
        }
        if (in_string) continue;
        if (c == ')') paren++;
        else if (c == '(') paren--;
        if (paren != 0) continue;
        for (int oi = 0; oi < op_count; ++oi) {
            const char *op = ops[oi];
            int len = (int)strlen(op);
            if (i - len + 1 >= 0 && strncmp(expr + i - len + 1, op, len) == 0) {
                int pos = i - len + 1;
                if ((pos == 0 || isspace((unsigned char)expr[pos-1]) || strchr("()[]", expr[pos-1]) == NULL) || len <= 2) {
                    best = pos;
                    return best;
                }
            }
        }
    }
    return best;
}

static Value eval_expr(Env *env, const char *raw);
static int execute_lines(Env *env, Lines *lines, int start, int end);

static Value call_builtin(Env *env, const char *name, const char *arg_text) {
    if (strcmp(name, "str") == 0) {
        Value v = eval_expr(env, arg_text);
        char *s = value_to_string(v);
        Value out = val_string(s);
        free(s);
        return out;
    }
    if (strcmp(name, "int") == 0) return val_number((double)(int)eval_expr(env, arg_text).number);
    if (strcmp(name, "float") == 0) return val_number(eval_expr(env, arg_text).number);
    if (strcmp(name, "len") == 0) {
        Value v = eval_expr(env, arg_text);
        if (v.type == VAL_STRING) return val_number((double)strlen(v.string));
        return val_number(0);
    }
    fprintf(stderr, "Runtime Error: unsupported function '%s' in native C interpreter\n", name);
    exit(1);
}

static Value eval_expr(Env *env, const char *raw) {
    char tmp[MAX_TEXT];
    snprintf(tmp, sizeof(tmp), "%s", raw);
    char *expr = trim(tmp);
    size_t len = strlen(expr);
    if (len >= 2 && ((expr[0] == '"' && expr[len-1] == '"') || (expr[0] == '\'' && expr[len-1] == '\''))) {
        expr[len-1] = '\0';
        return val_string(expr + 1);
    }
    if (len >= 2 && expr[0] == '(' && expr[len-1] == ')') {
        expr[len-1] = '\0';
        return eval_expr(env, expr + 1);
    }

    const char *cmp_ops[] = {"==", "!=", ">=", "<=", ">", "<"};
    int pos = find_top_operator(expr, cmp_ops, 6);
    if (pos >= 0) {
        const char *op = NULL;
        for (int i = 0; i < 6; ++i) if (strncmp(expr + pos, cmp_ops[i], strlen(cmp_ops[i])) == 0) { op = cmp_ops[i]; break; }
        char *left = substr_dup(expr, (size_t)pos);
        char *right = xstrdup(expr + pos + strlen(op));
        Value a = eval_expr(env, left), b = eval_expr(env, right);
        free(left); free(right);
        if (strcmp(op, "==") == 0) {
            if (a.type == VAL_STRING || b.type == VAL_STRING) { char *as = value_to_string(a), *bs = value_to_string(b); bool r = strcmp(as, bs) == 0; free(as); free(bs); return val_bool(r); }
            return val_bool(a.number == b.number && a.type == b.type);
        }
        if (strcmp(op, "!=") == 0) {
            if (a.type == VAL_STRING || b.type == VAL_STRING) { char *as = value_to_string(a), *bs = value_to_string(b); bool r = strcmp(as, bs) != 0; free(as); free(bs); return val_bool(r); }
            return val_bool(!(a.number == b.number && a.type == b.type));
        }
        double an = a.type == VAL_NUMBER ? a.number : 0.0;
        double bn = b.type == VAL_NUMBER ? b.number : 0.0;
        if (strcmp(op, ">=") == 0) return val_bool(an >= bn);
        if (strcmp(op, "<=") == 0) return val_bool(an <= bn);
        if (strcmp(op, ">") == 0) return val_bool(an > bn);
        if (strcmp(op, "<") == 0) return val_bool(an < bn);
    }

    const char *add_ops[] = {"+", "-"};
    pos = find_top_operator(expr, add_ops, 2);
    if (pos > 0) {
        char op = expr[pos];
        char *left = substr_dup(expr, (size_t)pos);
        char *right = xstrdup(expr + pos + 1);
        Value a = eval_expr(env, left), b = eval_expr(env, right);
        free(left); free(right);
        if (op == '+') {
            if (a.type == VAL_STRING || b.type == VAL_STRING) {
                char *as = value_to_string(a), *bs = value_to_string(b);
                char *joined = (char*)malloc(strlen(as) + strlen(bs) + 1);
                strcpy(joined, as); strcat(joined, bs);
                Value out = val_string(joined);
                free(as); free(bs); free(joined);
                return out;
            }
            return val_number(a.number + b.number);
        }
        return val_number(a.number - b.number);
    }

    const char *mul_ops[] = {"*", "/", "%"};
    pos = find_top_operator(expr, mul_ops, 3);
    if (pos > 0) {
        char op = expr[pos];
        char *left = substr_dup(expr, (size_t)pos);
        char *right = xstrdup(expr + pos + 1);
        Value a = eval_expr(env, left), b = eval_expr(env, right);
        free(left); free(right);
        if (op == '*') return val_number(a.number * b.number);
        if (op == '/') return val_number(a.number / b.number);
        return val_number(fmod(a.number, b.number));
    }

    if (starts_with(expr, "not ")) return val_bool(!value_truthy(eval_expr(env, expr + 4)));
    if (expr[0] == '-') return val_number(-eval_expr(env, expr + 1).number);
    if (expr[0] == '!') return val_bool(!value_truthy(eval_expr(env, expr + 1)));

    char *paren = strchr(expr, '(');
    if (paren && expr[len-1] == ')') {
        *paren = '\0';
        expr[len-1] = '\0';
        return call_builtin(env, trim(expr), trim(paren + 1));
    }

    char *end = NULL;
    double n = strtod(expr, &end);
    if (end && *trim(end) == '\0' && end != expr) return val_number(n);
    return env_get(env, expr);
}

static int collect_block(Lines *lines, int open_index, Lines *out) {
    int depth = 0;
    out->items = (char**)calloc(MAX_LINES, sizeof(char*));
    out->count = 0;
    for (int i = open_index; i < lines->count; ++i) {
        if (strcmp(lines->items[i], "{") == 0) { depth++; if (depth == 1) continue; }
        if (strcmp(lines->items[i], "}") == 0) { depth--; if (depth == 0) return i + 1; }
        if (depth >= 1) lines_push(out, lines->items[i]);
    }
    fprintf(stderr, "Parser Error: missing closing brace\n");
    exit(1);
}

static char *parse_use_path(const char *line) {
    char tmp[MAX_TEXT];
    snprintf(tmp, sizeof(tmp), "%s", line + 3);
    char *p = trim(tmp);
    size_t len = strlen(p);
    if (len > 0 && p[len - 1] == ';') p[--len] = '\0';
    p = trim(p);
    len = strlen(p);
    if (len >= 2 && ((p[0] == '"' && p[len - 1] == '"') || (p[0] == '\'' && p[len - 1] == '\''))) {
        p[len - 1] = '\0';
        p++;
    }
    return xstrdup(p);
}

static void run_use_module(Env *env, const char *line) {
    char *module = parse_use_path(line);
    char candidate[PATH_MAX];
    snprintf(candidate, sizeof(candidate), "%s", module);
    if (!strstr(candidate, ".wyr")) strncat(candidate, ".wyr", sizeof(candidate) - strlen(candidate) - 1);
    char *source = read_file(candidate);
    Lines module_lines = tokenize_lines(source);
    execute_lines(env, &module_lines, 0, module_lines.count);
    free(source);
    free(module);
}


static void exec_print(Env *env, const char *args) {
    char tmp[MAX_TEXT];
    snprintf(tmp, sizeof(tmp), "%s", args);
    char *p = trim(tmp);
    size_t len = strlen(p);
    if (len >= 2 && p[0] == '(' && p[len-1] == ')') { p[len-1] = '\0'; p++; }
    Value v = eval_expr(env, p);
    char *s = value_to_string(v);
    printf("%s\n", s);
    free(s);
}

static int execute_lines(Env *env, Lines *lines, int start, int end) {
    for (int i = start; i < end; ++i) {
        char *line = lines->items[i];
        if (strcmp(line, "{") == 0 || strcmp(line, "}") == 0) continue;
        if (starts_with(line, "use ")) { run_use_module(env, line); continue; }
        if (starts_with(line, "fn ")) {
            if (i + 1 < end && strcmp(lines->items[i+1], "{") == 0) { Lines skip; i = collect_block(lines, i + 1, &skip) - 1; }
            continue;
        }
        if (starts_with(line, "if ")) {
            if (i + 1 >= end || strcmp(lines->items[i+1], "{") != 0) { fprintf(stderr, "Parser Error: if requires block\n"); exit(1); }
            Lines block;
            int next = collect_block(lines, i + 1, &block);
            if (value_truthy(eval_expr(env, line + 3))) execute_lines(env, &block, 0, block.count);
            i = next - 1;
            continue;
        }
        if (starts_with(line, "repeat") || starts_with(line, "do")) {
            if (i + 1 >= end || strcmp(lines->items[i+1], "{") != 0) { fprintf(stderr, "Parser Error: repeat/do requires block\n"); exit(1); }
            Lines block;
            int next = collect_block(lines, i + 1, &block);
            if (next >= end || !starts_with(lines->items[next], "til")) { fprintf(stderr, "Parser Error: loop requires til(condition)\n"); exit(1); }
            const char *cond = lines->items[next] + 3;
            while (true) {
                int signal = execute_lines(env, &block, 0, block.count);
                if (signal == 1) break;
                if (value_truthy(eval_expr(env, cond))) break;
            }
            i = next;
            continue;
        }
        if (strcmp(line, "break") == 0) return 1;
        if (strcmp(line, "continue") == 0) return 2;
        if (starts_with(line, "print")) { exec_print(env, line + 5); continue; }
        if (starts_with(line, "var ")) line += 4;
        if (starts_with(line, "dec ")) line += 4;
        if (starts_with(line, "owned ")) line += 6;
        char *eq = strchr(line, '=');
        if (eq) {
            char *name = substr_dup(line, (size_t)(eq - line));
            char *value = xstrdup(eq + 1);
            env_set(env, trim(name), eval_expr(env, value));
            free(name); free(value);
            continue;
        }
        if (*line) fprintf(stderr, "Native C wyrmc warning: skipped unsupported statement: %s\n", line);
    }
    return 0;
}

static void run_source(const char *source) {
    Lines lines = tokenize_lines(source);
    Env env = {0};
    Lines main_body = {0};
    bool has_main = false;
    Lines top;
    top.items = (char**)calloc(MAX_LINES, sizeof(char*));
    top.count = 0;

    for (int i = 0; i < lines.count; ++i) {
        if (starts_with(lines.items[i], "fn main")) {
            if (i + 1 < lines.count && strcmp(lines.items[i+1], "{") == 0) {
                int next = collect_block(&lines, i + 1, &main_body);
                has_main = true;
                i = next - 1;
                continue;
            }
        }
        lines_push(&top, lines.items[i]);
    }
    execute_lines(&env, &top, 0, top.count);
    if (has_main) execute_lines(&env, &main_body, 0, main_body.count);
}

static char *c_escape(const char *s) {
    size_t cap = strlen(s) * 4 + 1;
    char *out = (char*)malloc(cap);
    size_t j = 0;
    for (size_t i = 0; s[i]; ++i) {
        unsigned char c = (unsigned char)s[i];
        if (c == '\\' || c == '"') { out[j++] = '\\'; out[j++] = (char)c; }
        else if (c == '\n') { out[j++] = '\\'; out[j++] = 'n'; }
        else if (c == '\r') { out[j++] = '\\'; out[j++] = 'r'; }
        else if (c == '\t') { out[j++] = '\\'; out[j++] = 't'; }
        else out[j++] = (char)c;
    }
    out[j] = '\0';
    return out;
}

static void default_output_path(const char *input, char *out, size_t out_size) {
    snprintf(out, out_size, "%s", input);
    char *dot = strrchr(out, '.');
    if (dot) *dot = '\0';
#ifdef _WIN32
    strncat(out, ".exe", out_size - strlen(out) - 1);
#endif
}

static int build_file(const char *input, const char *output) {
    char *source = read_file(input);
    char *escaped = c_escape(source);
    char temp_c[PATH_MAX];
    snprintf(temp_c, sizeof(temp_c), "%s.native.c", output);
    FILE *f = fopen(temp_c, "wb");
    if (!f) { fprintf(stderr, "Error: cannot write '%s'\n", temp_c); return 1; }
    fprintf(f, "#define WYRMC_EMBEDDED_SOURCE \"%s\"\n", escaped);
    fprintf(f, "#define WYRMC_EMBEDDED_MAIN 1\n");
    fprintf(f, "#include \"scr/wyrmc.c\"\n");
    fclose(f);
    char cmd[PATH_MAX * 2];
    snprintf(cmd, sizeof(cmd), "gcc -std=c11 -O2 -I . \"%s\" -o \"%s\" -lm", temp_c, output);
    int rc = system(cmd);
    if (rc == 0) printf("Built native C Wyrm executable: %s\n", output);
    free(source); free(escaped);
    return rc;
}

#ifndef WYRMC_EMBEDDED_MAIN
int main(int argc, char **argv) {
    if (argc <= 1 || strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "help") == 0) {
        printf("Wyrm native C AOT/dev tool (wyrm/wyrmc) v%s\n", WYRMC_VERSION);
        printf("Usage:\n  wyrm run <file.wyr>\n  wyrm build <file.wyr> [-o output.exe]\n  wyrm <file.wyr>\n  wyrmc remains as a compiler alias\n");
        return argc <= 1 ? 0 : 0;
    }
    if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "version") == 0) {
        printf("wyrm/wyrmc version %s (native C11 AOT primary, interpreter dev mode)\n", WYRMC_VERSION);
        return 0;
    }
    if (strcmp(argv[1], "build") == 0) {
        if (argc < 3) { fprintf(stderr, "Error: build requires <file.wyr>\n"); return 1; }
        char out[PATH_MAX];
        default_output_path(argv[2], out, sizeof(out));
        for (int i = 3; i < argc; ++i) if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) snprintf(out, sizeof(out), "%s", argv[++i]);
        return build_file(argv[2], out);
    }
    const char *file = (strcmp(argv[1], "run") == 0 && argc >= 3) ? argv[2] : argv[1];
    char *source = read_file(file);
    run_source(source);
    free(source);
    return 0;
}
#else
int main(void) {
    run_source(WYRMC_EMBEDDED_SOURCE);
    return 0;
}
#endif
