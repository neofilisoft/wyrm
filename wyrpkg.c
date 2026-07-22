#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <windows.h>

#define WYRPKG_VERSION "1.2"

bool file_exists(const char *path) {
    DWORD dwAttrib = GetFileAttributesA(path);
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

void get_python_command(char *out_cmd, size_t max_size, const char *module_args) {
    // 1. Check local workspace venv (Scripts = Windows layout, bin = Unix layout)
    if (file_exists(".\\venv\\Scripts\\python.exe")) {
        snprintf(out_cmd, max_size, ".\\venv\\Scripts\\python.exe %s", module_args);
        return;
    }
    if (file_exists(".\\venv\\bin\\python.exe")) {
        snprintf(out_cmd, max_size, ".\\venv\\bin\\python.exe %s", module_args);
        return;
    }
    
    // 2. Check global install venv relative to executable
    //    exe is at ~/.wyrm/bin/wyrpkg.exe, venv is at ~/.wyrm/venv/
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
        snprintf(out_cmd, max_size, "\"%s\" %s", global_venv_win, module_args);
        return;
    }
    
    char global_venv_unix[MAX_PATH];
    snprintf(global_venv_unix, sizeof(global_venv_unix), "%s\\venv\\bin\\python.exe", exe_path);
    if (file_exists(global_venv_unix)) {
        snprintf(out_cmd, max_size, "\"%s\" %s", global_venv_unix, module_args);
        return;
    }
    
    // 3. Fallback to system python
    snprintf(out_cmd, max_size, "python %s", module_args);
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "version") == 0) {
            printf("wyrpkg version %s\n", WYRPKG_VERSION);
            return 0;
        }
        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "help") == 0) {
            printf("wyrpkg version %s - Wyrm Package Manager\n", WYRPKG_VERSION);
            printf("Usage: wyrpkg <command> [options]\n");
            printf("Commands:\n");
            printf("  install <package>   Install a package\n");
            printf("  remove <package>    Remove an installed package\n");
            printf("  list                List installed packages\n");
            printf("  --version, -v       Display version\n");
            return 0;
        }
    }

    // Pass execution to python -m wyrm.pkg
    char base_cmd[512];
    get_python_command(base_cmd, sizeof(base_cmd), "-m wyrm.pkg");
    
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "%s", base_cmd);
    
    for (int i = 1; i < argc; i++) {
        strcat(cmd, " ");
        strcat(cmd, argv[i]);
    }
    return system(cmd);
}
