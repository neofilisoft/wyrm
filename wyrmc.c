#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <windows.h>

#define WYRMC_VERSION "1.2"

bool file_exists(const char *path) {
    DWORD dwAttrib = GetFileAttributesA(path);
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

void get_python_command(char *out_cmd, size_t max_size, const char *module_args, const char *filename) {
    // 1. Check local workspace venv (Scripts = Windows layout, bin = Unix layout)
    if (file_exists(".\\venv\\Scripts\\python.exe")) {
        snprintf(out_cmd, max_size, ".\\venv\\Scripts\\python.exe %s \"%s\"", module_args, filename);
        return;
    }
    if (file_exists(".\\venv\\bin\\python.exe")) {
        snprintf(out_cmd, max_size, ".\\venv\\bin\\python.exe %s \"%s\"", module_args, filename);
        return;
    }
    
    // 2. Check global install venv relative to executable
    //    exe is at ~/.wyrm/bin/wyrmc.exe, venv is at ~/.wyrm/venv/
    char exe_path[MAX_PATH];
    GetModuleFileNameA(NULL, exe_path, MAX_PATH);
    char *last_slash = strrchr(exe_path, '\\');
    if (last_slash) {
        *last_slash = '\0'; // exe_path is now the bin dir
    }
    char *parent_slash = strrchr(exe_path, '\\');
    if (parent_slash) {
        *parent_slash = '\0'; // exe_path is now the install root (~/.wyrm)
    }
    
    char global_venv_win[MAX_PATH];
    snprintf(global_venv_win, sizeof(global_venv_win), "%s\\venv\\Scripts\\python.exe", exe_path);
    if (file_exists(global_venv_win)) {
        snprintf(out_cmd, max_size, "\"%s\" %s \"%s\"", global_venv_win, module_args, filename);
        return;
    }
    
    char global_venv_unix[MAX_PATH];
    snprintf(global_venv_unix, sizeof(global_venv_unix), "%s\\venv\\bin\\python.exe", exe_path);
    if (file_exists(global_venv_unix)) {
        snprintf(out_cmd, max_size, "\"%s\" %s \"%s\"", global_venv_unix, module_args, filename);
        return;
    }
    
    // 3. Fallback to system python
    snprintf(out_cmd, max_size, "python %s \"%s\"", module_args, filename);
}

int run_file(const char *command_type, const char *filename) {
    if (strcmp(command_type, "build") == 0) {
        printf("[wyrmc Compiler v%s] Compiling %s to native binary...\n", WYRMC_VERSION, filename);
    } else {
        printf("[wyrmc VM v%s] Executing %s via Wyrm Virtual Machine...\n", WYRMC_VERSION, filename);
    }

    char command[4096];
    char module_args[128];
    snprintf(module_args, sizeof(module_args), "-m wyrm %s", command_type);
    
    get_python_command(command, sizeof(command), module_args, filename);
    return system(command);
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "version") == 0) {
            printf("wyrmc version %s (Wyrm Compiler & LLVM)\n", WYRMC_VERSION);
            return 0;
        }
        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "help") == 0) {
            printf("Wyrm Compiler & VM (wyrmc) v%s\n", WYRMC_VERSION);
            printf("Usage:\n");
            printf("  wyrmc [command] <file.wyr>\n\n");
            printf("Commands:\n");
            printf("  build <file.wyr>  Compile source file to native binary\n");
            printf("  run <file.wyr>    Run source file or bytecode file\n");
            printf("  <file.wyr>        Run source file directly (default)\n\n");
            printf("Options:\n");
            printf("  --version, -v     Display compiler/VM version\n");
            printf("  --help, -h        Display help information\n");
            return 0;
        }

        if (strcmp(argv[1], "build") == 0) {
            if (argc < 3) {
                fprintf(stderr, "Error: Please specify the source file to build.\n");
                return 1;
            }
            return run_file("build", argv[2]);
        } else if (strcmp(argv[1], "run") == 0) {
            if (argc < 3) {
                fprintf(stderr, "Error: Please specify the file to run.\n");
                return 1;
            }
            return run_file("run", argv[2]);
        } else {
            // Default is run command
            return run_file("run", argv[1]);
        }
    }

    printf("wyrmc version %s (Wyrm Compiler & LLVM)\n", WYRMC_VERSION);
    printf("Use 'wyrmc --help' for usage.\n");
    return 0;
}
