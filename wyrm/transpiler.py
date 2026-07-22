import os
import sys
import subprocess
from .ast import *
from .lexer import Lexer
from .parser import Parser

# C runtime embedded as a string
C_RUNTIME = """#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <stdarg.h>

typedef enum {
    VAL_NULL,
    VAL_BOOL,
    VAL_NUMBER,
    VAL_STRING,
    VAL_ARRAY
} ValueType;

struct Value;

typedef struct {
    struct Value *data;
    int size;
    int capacity;
} ValArray;

typedef struct Value {
    ValueType type;
    union {
        bool boolean;
        double number;
        char *string;
        ValArray *array;
    } as;
} Value;

// Function declarations
Value val_null();
Value val_bool(bool b);
Value val_number(double n);
Value val_string(const char *s);
Value val_array_init(int count, ...);
bool val_to_bool(Value v);
char* val_to_str_ptr(Value v);
void val_print(int count, ...);
Value val_input(Value prompt);
Value val_len(Value v);
Value val_type(Value v);
Value val_int(Value v);
Value val_float(Value v);
Value val_str(Value v);
Value val_abs(Value v);
Value val_min(int count, ...);
Value val_max(int count, ...);
Value val_round(Value v);
Value val_pow(Value x, Value y);

Value val_add(Value a, Value b);
Value val_sub(Value a, Value b);
Value val_mul(Value a, Value b);
Value val_div(Value a, Value b);
Value val_mod(Value a, Value b);
Value val_eq(Value a, Value b);
Value val_ne(Value a, Value b);
Value val_lt(Value a, Value b);
Value val_gt(Value a, Value b);
Value val_le(Value a, Value b);
Value val_ge(Value a, Value b);
Value val_and(Value a, Value b);
Value val_or(Value a, Value b);
Value val_not(Value a);

Value val_array_get(Value arr, Value index);
Value val_array_set(Value arr, Value index, Value val);
Value val_floordiv(Value a, Value b);
Value val_array_append(Value arr, Value item);
Value val_array_pop(Value arr);


// Implementations
Value val_null() {
    Value v;
    v.type = VAL_NULL;
    return v;
}

Value val_bool(bool b) {
    Value v;
    v.type = VAL_BOOL;
    v.as.boolean = b;
    return v;
}

Value val_number(double n) {
    Value v;
    v.type = VAL_NUMBER;
    v.as.number = n;
    return v;
}

Value val_string(const char *s) {
    Value v;
    v.type = VAL_STRING;
    v.as.string = strdup(s ? s : "");
    return v;
}

Value val_array_init(int count, ...) {
    Value v;
    v.type = VAL_ARRAY;
    v.as.array = malloc(sizeof(ValArray));
    v.as.array->size = count;
    v.as.array->capacity = count;
    if (count > 0) {
        v.as.array->data = malloc(count * sizeof(Value));
        va_list args;
        va_start(args, count);
        for (int i = 0; i < count; i++) {
            v.as.array->data[i] = va_arg(args, Value);
        }
        va_end(args);
    } else {
        v.as.array->data = NULL;
    }
    return v;
}

bool val_to_bool(Value v) {
    switch (v.type) {
        case VAL_NULL: return false;
        case VAL_BOOL: return v.as.boolean;
        case VAL_NUMBER: return v.as.number != 0.0;
        case VAL_STRING: return strlen(v.as.string) > 0;
        case VAL_ARRAY: return v.as.array->size > 0;
    }
    return false;
}

char* val_to_str_ptr(Value v) {
    char buf[128];
    switch (v.type) {
        case VAL_NULL: return strdup("null");
        case VAL_BOOL: return strdup(v.as.boolean ? "true" : "false");
        case VAL_NUMBER:
            if (v.as.number == floor(v.as.number)) {
                snprintf(buf, sizeof(buf), "%.0f", v.as.number);
            } else {
                snprintf(buf, sizeof(buf), "%g", v.as.number);
            }
            return strdup(buf);
        case VAL_STRING: return strdup(v.as.string);
        case VAL_ARRAY: {
            int cap = 256;
            char *res = malloc(cap);
            strcpy(res, "[");
            for (int i = 0; i < v.as.array->size; i++) {
                char *item_str = val_to_str_ptr(v.as.array->data[i]);
                if (strlen(res) + strlen(item_str) + 4 > cap) {
                    cap = cap * 2 + strlen(item_str);
                    res = realloc(res, cap);
                }
                strcat(res, item_str);
                free(item_str);
                if (i < v.as.array->size - 1) {
                    strcat(res, ", ");
                }
            }
            strcat(res, "]");
            return res;
        }
    }
    return strdup("");
}

void val_print(int count, ...) {
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; i++) {
        Value val = va_arg(args, Value);
        char *s = val_to_str_ptr(val);
        printf("%s", s);
        free(s);
        if (i < count - 1) {
            printf(" ");
        }
    }
    va_end(args);
    printf("\\n");
}

Value val_input(Value prompt) {
    char *prompt_str = val_to_str_ptr(prompt);
    printf("%s", prompt_str);
    free(prompt_str);
    fflush(stdout);

    char buf[1024];
    if (fgets(buf, sizeof(buf), stdin)) {
        size_t len = strlen(buf);
        if (len > 0 && buf[len - 1] == '\\n') {
            buf[len - 1] = '\\0';
        }
        return val_string(buf);
    }
    return val_string("");
}

Value val_len(Value v) {
    if (v.type == VAL_STRING) {
        return val_number((double)strlen(v.as.string));
    } else if (v.type == VAL_ARRAY) {
        return val_number((double)v.as.array->size);
    }
    return val_number(0.0);
}

Value val_type(Value v) {
    switch (v.type) {
        case VAL_NULL: return val_string("null");
        case VAL_BOOL: return val_string("bool");
        case VAL_NUMBER: return val_string("number");
        case VAL_STRING: return val_string("string");
        case VAL_ARRAY: return val_string("array");
    }
    return val_string("unknown");
}

Value val_int(Value v) {
    if (v.type == VAL_NUMBER) {
        return val_number(floor(v.as.number));
    } else if (v.type == VAL_STRING) {
        return val_number(atof(v.as.string));
    }
    return val_number(0.0);
}

Value val_float(Value v) {
    if (v.type == VAL_NUMBER) {
        return v;
    } else if (v.type == VAL_STRING) {
        return val_number(atof(v.as.string));
    }
    return val_number(0.0);
}

Value val_str(Value v) {
    char *s = val_to_str_ptr(v);
    Value res = val_string(s);
    free(s);
    return res;
}

Value val_abs(Value v) {
    if (v.type == VAL_NUMBER) {
        return val_number(fabs(v.as.number));
    }
    return val_number(0.0);
}

Value val_round(Value v) {
    if (v.type == VAL_NUMBER) {
        return val_number(round(v.as.number));
    }
    return val_number(0.0);
}

Value val_pow(Value x, Value y) {
    if (x.type == VAL_NUMBER && y.type == VAL_NUMBER) {
        return val_number(pow(x.as.number, y.as.number));
    }
    return val_number(0.0);
}

Value val_add(Value a, Value b) {
    if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
        return val_number(a.as.number + b.as.number);
    }
    char *s1 = val_to_str_ptr(a);
    char *s2 = val_to_str_ptr(b);
    char *res = malloc(strlen(s1) + strlen(s2) + 1);
    strcpy(res, s1);
    strcat(res, s2);
    Value val = val_string(res);
    free(s1);
    free(s2);
    free(res);
    return val;
}

Value val_sub(Value a, Value b) {
    if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
        return val_number(a.as.number - b.as.number);
    }
    return val_number(0.0);
}

Value val_mul(Value a, Value b) {
    if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
        return val_number(a.as.number * b.as.number);
    }
    return val_number(0.0);
}

Value val_div(Value a, Value b) {
    if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
        if (b.as.number == 0.0) {
            fprintf(stderr, "Runtime Error: Division by zero\\n");
            exit(1);
        }
        return val_number(a.as.number / b.as.number);
    }
    return val_number(0.0);
}

Value val_mod(Value a, Value b) {
    if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
        if (b.as.number == 0.0) {
            fprintf(stderr, "Runtime Error: Modulo by zero\\n");
            exit(1);
        }
        return val_number(fmod(a.as.number, b.as.number));
    }
    return val_number(0.0);
}

Value val_eq(Value a, Value b) {
    if (a.type != b.type) return val_bool(false);
    switch (a.type) {
        case VAL_NULL: return val_bool(true);
        case VAL_BOOL: return val_bool(a.as.boolean == b.as.boolean);
        case VAL_NUMBER: return val_bool(a.as.number == b.as.number);
        case VAL_STRING: return val_bool(strcmp(a.as.string, b.as.string) == 0);
        case VAL_ARRAY: return val_bool(a.as.array == b.as.array);
    }
    return val_bool(false);
}

Value val_ne(Value a, Value b) {
    return val_bool(!val_to_bool(val_eq(a, b)));
}

Value val_lt(Value a, Value b) {
    if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
        return val_bool(a.as.number < b.as.number);
    }
    return val_bool(false);
}

Value val_gt(Value a, Value b) {
    if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
        return val_bool(a.as.number > b.as.number);
    }
    return val_bool(false);
}

Value val_le(Value a, Value b) {
    if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
        return val_bool(a.as.number <= b.as.number);
    }
    return val_bool(false);
}

Value val_ge(Value a, Value b) {
    if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
        return val_bool(a.as.number >= b.as.number);
    }
    return val_bool(false);
}

Value val_and(Value a, Value b) {
    return val_bool(val_to_bool(a) && val_to_bool(b));
}

Value val_or(Value a, Value b) {
    return val_bool(val_to_bool(a) || val_to_bool(b));
}

Value val_not(Value a) {
    return val_bool(!val_to_bool(a));
}

Value val_array_get(Value arr, Value index) {
    int idx = (int)index.as.number;
    if (arr.type == VAL_STRING) {
        int len = strlen(arr.as.string);
        if (idx < 0 || idx >= len) {
            fprintf(stderr, "Runtime Error: String index out of bounds: %d\\n", idx);
            exit(1);
        }
        char char_str[2] = { arr.as.string[idx], '\\0' };
        return val_string(char_str);
    }
    if (arr.type != VAL_ARRAY) {
        fprintf(stderr, "Runtime Error: Object is not subscriptable\\n");
        exit(1);
    }
    if (idx < 0 || idx >= arr.as.array->size) {
        fprintf(stderr, "Runtime Error: Array index out of bounds: %d\\n", idx);
        exit(1);
    }
    return arr.as.array->data[idx];
}

Value val_array_set(Value arr, Value index, Value val) {
    if (arr.type != VAL_ARRAY) {
        fprintf(stderr, "Runtime Error: Object is not subscriptable\\n");
        exit(1);
    }
    int idx = (int)index.as.number;
    if (idx < 0 || idx >= arr.as.array->size) {
        fprintf(stderr, "Runtime Error: Array index out of bounds: %d\\n", idx);
        exit(1);
    }
    arr.as.array->data[idx] = val;
    return val;
}

Value val_floordiv(Value a, Value b) {
    if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
        if (b.as.number == 0.0) {
            fprintf(stderr, "Runtime Error: Division by zero\\n");
            exit(1);
        }
        return val_number(floor(a.as.number / b.as.number));
    }
    return val_number(0.0);
}

Value val_array_append(Value arr, Value item) {
    if (arr.type != VAL_ARRAY) {
        fprintf(stderr, "Runtime Error: append() first argument must be an array\\n");
        exit(1);
    }
    if (arr.as.array->size >= arr.as.array->capacity) {
        int new_cap = arr.as.array->capacity == 0 ? 8 : arr.as.array->capacity * 2;
        arr.as.array->data = realloc(arr.as.array->data, new_cap * sizeof(Value));
        arr.as.array->capacity = new_cap;
    }
    arr.as.array->data[arr.as.array->size++] = item;
    return arr;
}

Value val_array_pop(Value arr) {
    if (arr.type != VAL_ARRAY) {
        fprintf(stderr, "Runtime Error: pop() argument must be an array\\n");
        exit(1);
    }
    if (arr.as.array->size == 0) {
        fprintf(stderr, "Runtime Error: pop() on empty array\\n");
        exit(1);
    }
    return arr.as.array->data[--arr.as.array->size];
}
"""

class Transpiler:
    def __init__(self):
        self.header_lines = []
        self.function_lines = []
        self.top_level_lines = []
        
        self.declared_globals = set()
        self.declared_locals = set()
        self.in_function = False
        self.constants = set()
        self.has_main_def = False

    def emit(self, stmt: str):
        if self.in_function:
            self.function_lines.append("    " + stmt)
        else:
            self.top_level_lines.append("    " + stmt)

    def visit_statement(self, stmt):
        res = self.visit(stmt)
        if res is not None and isinstance(stmt, FunctionCallNode):
            self.emit(res + ";")

    def transpile(self, ast: list) -> str:
        # Prepend headers and runtime
        self.header_lines.append(C_RUNTIME)
        
        # Check if main function is defined in Wyrm
        for stmt in ast:
            if isinstance(stmt, FunctionDefNode) and stmt.name.name == "main":
                self.has_main_def = True

        # Forward declarations of user functions
        self.header_lines.append("\n// Forward declarations of user functions")
        for stmt in ast:
            if isinstance(stmt, FunctionDefNode):
                # Don't redeclare main as wyrm_fn_main since it's the entry point wrapper
                if stmt.name.name == "main":
                    continue
                params_str = ", ".join("Value " + p.name for p in stmt.params)
                self.header_lines.append(f"Value wyrm_fn_{stmt.name.name}({params_str});")

        self.header_lines.append("\n// Global variable declarations")

        # Translate AST
        for stmt in ast:
            self.visit_statement(stmt)

        # Assemble final program
        final_code = []
        final_code.extend(self.header_lines)
        final_code.append("\n// Function implementations")
        final_code.extend(self.function_lines)
        
        # Generate the main entry wrapper
        if self.has_main_def:
            # We already defined fn main() in Wyrm, which generated wyrm_fn_main()
            final_code.append("""
int main() {
    // Run any global top-level statements first
""")
            # Put global scope statements
            final_code.extend(self.top_level_lines)
            final_code.append("""    wyrm_fn_main();
    return 0;
}
""")
        else:
            # No fn main() in Wyrm, wrap top-level statements inside C main
            final_code.append("""
int main() {
""")
            final_code.extend(self.top_level_lines)
            final_code.append("""    return 0;
}
""")

        return "\n".join(final_code)

    def visit(self, node):
        method_name = f"visit_{type(node).__name__}"
        visitor = getattr(self, method_name, self.generic_visit)
        return visitor(node)

    def generic_visit(self, node):
        raise Exception(f"No visit_{type(node).__name__} method defined in Transpiler")

    def visit_NumberNode(self, node: NumberNode):
        return f"val_number({node.value})"

    def visit_StringNode(self, node: StringNode):
        escaped = node.value.replace('"', '\\"')
        return f'val_string("{escaped}")'

    def visit_BooleanNode(self, node: BooleanNode):
        return f"val_bool({str(node.value).lower()})"

    def visit_NoneNode(self, node: NoneNode):
        return "val_null()"

    def visit_IdentifierNode(self, node: IdentifierNode):
        return f"wyrm_var_{node.name}"

    def visit_BinaryOpNode(self, node: BinaryOpNode):
        left = self.visit(node.left)
        right = self.visit(node.right)
        op = node.op.value

        op_map = {
            '+': 'val_add',
            '-': 'val_sub',
            '*': 'val_mul',
            '/': 'val_div',
            '//': 'val_floordiv',
            '%': 'val_mod',
            '==': 'val_eq',
            '!=': 'val_ne',
            '<': 'val_lt',
            '>': 'val_gt',
            '<=': 'val_le',
            '>=': 'val_ge',
            'and': 'val_and',
            '&&': 'val_and',
            'or': 'val_or',
            '||': 'val_or'
        }
        if op in op_map:
            return f"{op_map[op]}({left}, {right})"
        raise Exception(f"Unknown binary operator: {op}")

    def visit_UnaryOpNode(self, node: UnaryOpNode):
        operand = self.visit(node.expr)
        op = node.op.value
        if op == '-':
            return f"val_sub(val_number(0.0), {operand})"
        elif op in ('!', 'not'):
            return f"val_not({operand})"
        raise Exception(f"Unknown unary operator: {op}")

    def visit_AssignmentNode(self, node: AssignmentNode):
        var_name = node.var_name.name
        value_expr = self.visit(node.value)
        c_var = f"wyrm_var_{var_name}"

        if self.in_function:
            # Local scope
            if var_name in self.constants:
                raise Exception(f"Compilation Error: Cannot assign to constant '{var_name}'")
                
            if node.is_declaration:
                if node.declaration_type == 'dec':
                    self.constants.add(var_name)
                self.declared_locals.add(var_name)
                stmt = f"Value {c_var} = {value_expr};"
            else:
                if var_name not in self.declared_locals:
                    self.declared_locals.add(var_name)
                    stmt = f"Value {c_var} = {value_expr};"
                else:
                    stmt = f"{c_var} = {value_expr};"
            self.emit(stmt)
        else:
            # Global scope
            if var_name in self.constants:
                raise Exception(f"Compilation Error: Cannot assign to constant '{var_name}'")
                
            if var_name not in self.declared_globals:
                self.declared_globals.add(var_name)
                self.header_lines.append(f"Value {c_var};")
                
            if node.is_declaration and node.declaration_type == 'dec':
                self.constants.add(var_name)
                
            stmt = f"{c_var} = {value_expr};"
            self.emit(stmt)

    def visit_IndexAssignNode(self, node: IndexAssignNode):
        obj = self.visit(node.obj)
        index = self.visit(node.index)
        value = self.visit(node.value)
        self.emit(f"val_array_set({obj}, {index}, {value});")

    def visit_PrintNode(self, node: PrintNode):
        args = [self.visit(expr) for expr in node.expressions]
        args_str = ", ".join(args)
        self.emit(f"val_print({len(args)}, {args_str});")

    def visit_IfNode(self, node: IfNode):
        cond = self.visit(node.condition)
        self.emit(f"if (val_to_bool({cond})) {{")
        for stmt in node.then_body:
            self.visit_statement(stmt)
        self.emit("}")
        
        for elif_cond, elif_body in node.elif_clauses:
            elif_cond_str = self.visit(elif_cond)
            self.emit(f"else if (val_to_bool({elif_cond_str})) {{")
            for stmt in elif_body:
                self.visit_statement(stmt)
            self.emit("}")
            
        if node.else_body:
            self.emit("else {")
            for stmt in node.else_body:
                self.visit_statement(stmt)
            self.emit("}")

    def visit_RepeatNode(self, node: RepeatNode):
        self.emit("do {")
        for stmt in node.body:
            self.visit_statement(stmt)
        cond = self.visit(node.condition)
        self.emit(f"}} while (!val_to_bool({cond}));")

    def visit_FunctionDefNode(self, node: FunctionDefNode):
        self.in_function = True
        func_name = node.name.name
        
        # main function gets mapped to wyrm_fn_main to avoid collision with C main
        c_func_name = "wyrm_fn_main" if func_name == "main" else f"wyrm_fn_{func_name}"
        params_str = ", ".join(f"Value wyrm_var_{p.name}" for p in node.params)
        
        # Track local variables
        old_locals = self.declared_locals.copy()
        self.declared_locals.clear()
        for p in node.params:
            self.declared_locals.add(p.name)

        self.function_lines.append(f"\nValue {c_func_name}({params_str}) {{")
        for stmt in node.body:
            self.visit_statement(stmt)
            
        self.function_lines.append("    return val_null();")
        self.function_lines.append("}")
        
        self.declared_locals = old_locals
        self.in_function = False

    def visit_FunctionCallNode(self, node: FunctionCallNode):
        func_name = node.name.name
        args = [self.visit(arg) for arg in node.args]
        args_str = ", ".join(args)

        builtins_map = {
            'input': 'val_input',
            'len': 'val_len',
            'type': 'val_type',
            'int': 'val_int',
            'float': 'val_float',
            'str': 'val_str',
            'abs': 'val_abs',
            'round': 'val_round',
            'pow': 'val_pow',
            'append': 'val_array_append',
            'pop': 'val_array_pop',
        }
        
        if func_name in builtins_map:
            if func_name == 'pow':
                return f"{builtins_map[func_name]}({args[0]}, {args[1]})"
            elif func_name == 'input':
                prompt_arg = args[0] if args else "val_string(\"\")"
                return f"val_input({prompt_arg})"
            elif func_name in ('append', 'pop'):
                # append(list, val) -> val_array_append(list, val)
                # pop(list)         -> val_array_pop(list)
                c_fn = builtins_map[func_name]
                args_str = ', '.join(args)
                return f"{c_fn}({args_str})"
            return f"{builtins_map[func_name]}({args[0]})"
        else:
            c_fn = "wyrm_fn_main" if func_name == "main" else f"wyrm_fn_{func_name}"
            return f"{c_fn}({args_str})"

    def visit_ListNode(self, node: ListNode):
        args = [self.visit(el) for el in node.elements]
        args_str = ", ".join(args)
        return f"val_array_init({len(args)}{', ' + args_str if args_str else ''})"

    def visit_IndexNode(self, node: IndexNode):
        obj = self.visit(node.obj)
        index = self.visit(node.index)
        return f"val_array_get({obj}, {index})"

    def visit_ReturnNode(self, node: ReturnNode):
        expr = self.visit(node.expr)
        self.emit(f"return {expr};")

    def visit_BreakNode(self, node):
        self.emit("break;")

    def visit_ContinueNode(self, node):
        # In do-while (repeat/til), continue goes to the condition check - correct semantics
        self.emit("continue;")

    def visit_UseNode(self, node: UseNode):
        raw_path = node.module_path
        global_pkg_dir = os.path.join(os.path.expanduser("~"), ".wyrm", "packages")
        possible_paths = [
            raw_path,
            raw_path + ".wyr",
            os.path.join("packages", raw_path),
            os.path.join("packages", raw_path + ".wyr"),
            os.path.join("packages", raw_path, "mod.wyr"),
            os.path.join("packages", raw_path, "main.wyr"),
            os.path.join(global_pkg_dir, raw_path),
            os.path.join(global_pkg_dir, raw_path + ".wyr"),
            os.path.join(global_pkg_dir, raw_path, "mod.wyr"),
            os.path.join(global_pkg_dir, raw_path, "main.wyr"),
        ]
        target = None
        for p in possible_paths:
            if os.path.exists(p) and os.path.isfile(p):
                target = p
                break
        if not target:
            raise Exception(f"Compilation Error: Cannot find module '{raw_path}'")

        with open(target, 'r', encoding='utf-8') as f:
            source = f.read()

        lexer = Lexer(source)
        tokens = lexer.tokenize()
        parser = Parser(tokens)
        ast = parser.parse()

        self.header_lines.append(f"\n// --- Module: {raw_path} ---")
        for stmt in ast:
            if isinstance(stmt, FunctionDefNode):
                if stmt.name.name == "main":
                    continue
                params_str = ", ".join("Value " + p.name for p in stmt.params)
                self.header_lines.append(f"Value wyrm_fn_{stmt.name.name}({params_str});")
            self.visit_statement(stmt)
        self.header_lines.append(f"// --- End of Module: {raw_path} ---\n")


def transpile_to_c(source: str) -> str:
    lexer = Lexer(source)
    tokens = lexer.tokenize()
    parser = Parser(tokens)
    ast = parser.parse()

    transpiler = Transpiler()
    return transpiler.transpile(ast)
