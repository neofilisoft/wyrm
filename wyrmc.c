#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define WYRMC_VERSION "1.0.0"
#define STACK_MAX 1024
#define VARS_MAX 256

// Wyrm OpCodes matching bytecode.py
typedef enum {
    OP_LOAD_CONST = 0,
    OP_LOAD_NONE = 1,
    OP_LOAD_TRUE = 2,
    OP_LOAD_FALSE = 3,
    OP_LOAD_VAR = 4,
    OP_STORE_VAR = 5,
    OP_ADD = 10,
    OP_SUB = 11,
    OP_MUL = 12,
    OP_DIV = 13,
    OP_MOD = 14,
    OP_EQ = 20,
    OP_NE = 21,
    OP_LT = 22,
    OP_GT = 23,
    OP_LE = 24,
    OP_GE = 25,
    OP_AND = 30,
    OP_OR = 31,
    OP_NOT = 32,
    OP_JUMP = 40,
    OP_JUMP_IF_FALSE = 41,
    OP_PRINT = 60
} OpCode;

typedef enum {
    VAL_NULL,
    VAL_BOOL,
    VAL_NUMBER,
    VAL_STRING
} ValueType;

typedef struct {
    ValueType type;
    union {
        bool boolean;
        double number;
        char *string;
    } as;
} Value;

typedef struct {
    char name[64];
    Value value;
} Variable;

typedef struct {
    Value stack[STACK_MAX];
    int stack_top;
    Variable vars[VARS_MAX];
    int var_count;
} VM;

static VM vm;

void vm_init() {
    vm.stack_top = 0;
    vm.var_count = 0;
}

void push(Value val) {
    if (vm.stack_top >= STACK_MAX) {
        fprintf(stderr, "VM Error: Stack overflow\n");
        exit(1);
    }
    vm.stack[vm.stack_top++] = val;
}

Value pop() {
    if (vm.stack_top <= 0) {
        fprintf(stderr, "VM Error: Stack underflow\n");
        exit(1);
    }
    return vm.stack[--vm.stack_top];
}

void set_var(const char *name, Value val) {
    for (int i = 0; i < vm.var_count; i++) {
        if (strcmp(vm.vars[i].name, name) == 0) {
            vm.vars[i].value = val;
            return;
        }
    }
    if (vm.var_count < VARS_MAX) {
        strncpy(vm.vars[vm.var_count].name, name, 63);
        vm.vars[vm.var_count].value = val;
        vm.var_count++;
    }
}

Value get_var(const char *name) {
    for (int i = 0; i < vm.var_count; i++) {
        if (strcmp(vm.vars[i].name, name) == 0) {
            return vm.vars[i].value;
        }
    }
    Value null_val = { .type = VAL_NULL };
    return null_val;
}

void print_value(Value val) {
    switch (val.type) {
        case VAL_NULL: printf("null"); break;
        case VAL_BOOL: printf(val.as.boolean ? "true" : "false"); break;
        case VAL_NUMBER: printf("%g", val.as.number); break;
        case VAL_STRING: printf("%s", val.as.string); break;
    }
}

int run_file(const char *filename) {
    printf("[wyrmc VM v%s] Executing %s via Wyrm Virtual Machine...\n", WYRMC_VERSION, filename);

    char command[512];
    snprintf(command, sizeof(command), ".\\venv\\bin\\python.exe -m wyrm \"%s\"", filename);
    return system(command);
}

int main(int argc, char *argv[]) {
    vm_init();
    
    if (argc > 1) {
        if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0) {
            printf("wyrmc version %s (Native C11 Wyrm Bytecode VM)\n", WYRMC_VERSION);
            return 0;
        }
        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            printf("Wyrm Compiler & VM (wyrmc) v%s\n", WYRMC_VERSION);
            printf("Usage: wyrmc [options] [file.wyr]\n");
            printf("Options:\n");
            printf("  --version, -v    Display compiler/VM version\n");
            printf("  --help, -h       Display help information\n");
            return 0;
        }
        
        return run_file(argv[1]);
    }

    printf("wyrmc version %s (Native C11 Wyrm Bytecode VM)\n", WYRMC_VERSION);
    printf("Use 'wyrmc --help' for usage.\n");
    return 0;
}
