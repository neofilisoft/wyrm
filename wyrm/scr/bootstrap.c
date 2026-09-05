#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <windows.h>

bool find_workspace(const char *exe_path, char *out_workspace) {
    char current[MAX_PATH];
    strncpy(current, exe_path, sizeof(current) - 1);
    current[sizeof(current) - 1] = '\0';
    
    char *last_slash = strrchr(current, '\\');
    if (last_slash) {
        *last_slash = '\0';
    } else {
        strcpy(current, ".");
    }
    
    for (int i = 0; i < 4; i++) {
        char version_path[MAX_PATH];
        snprintf(version_path, sizeof(version_path), "%s\\VERSION", current);
        if (GetFileAttributesA(version_path) != INVALID_FILE_ATTRIBUTES) {
            GetFullPathNameA(current, MAX_PATH, out_workspace, NULL);
            return true;
        }
        char *parent = strrchr(current, '\\');
        if (parent) {
            *parent = '\0';
        } else {
            break;
        }
    }
    
    if (GetFileAttributesA(".\\VERSION") != INVALID_FILE_ATTRIBUTES) {
        GetFullPathNameA(".", MAX_PATH, out_workspace, NULL);
        return true;
    }
    return false;
}

bool create_dir_recursive(const char *path) {
    char temp[MAX_PATH];
    strncpy(temp, path, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';
    
    char *p = temp;
    if (temp[1] == ':') {
        p += 3;
    }
    
    while (*p) {
        if (*p == '\\' || *p == '/') {
            char old = *p;
            *p = '\0';
            CreateDirectoryA(temp, NULL);
            *p = old;
        }
        p++;
    }
    return CreateDirectoryA(temp, NULL) || GetLastError() == ERROR_ALREADY_EXISTS;
}

void add_to_path(const char *install_dir) {
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Environment", 0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS) {
        char old_path[8192] = {0};
        DWORD old_path_len = sizeof(old_path);
        DWORD type = REG_SZ;
        
        if (RegQueryValueExA(hKey, "Path", NULL, &type, (LPBYTE)old_path, &old_path_len) != ERROR_SUCCESS) {
            old_path[0] = '\0';
        }
        
        if (strstr(old_path, install_dir) == NULL) {
            char new_path[16384];
            if (strlen(old_path) > 0) {
                if (old_path[strlen(old_path) - 1] == ';') {
                    snprintf(new_path, sizeof(new_path), "%s%s", old_path, install_dir);
                } else {
                    snprintf(new_path, sizeof(new_path), "%s;%s", old_path, install_dir);
                }
            } else {
                snprintf(new_path, sizeof(new_path), "%s", install_dir);
            }
            
            if (RegSetValueExA(hKey, "Path", 0, REG_EXPAND_SZ, (const BYTE*)new_path, (DWORD)(strlen(new_path) + 1)) == ERROR_SUCCESS) {
                printf("Successfully added %s to User PATH.\n", install_dir);
                SendMessageTimeoutA(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)"Environment", SMTO_ABORTIFHUNG, 1000, NULL);
            } else {
                fprintf(stderr, "Warning: Failed to set new PATH in registry.\n");
            }
        } else {
            printf("%s is already in User PATH.\n", install_dir);
        }
        RegCloseKey(hKey);
    } else {
        fprintf(stderr, "Warning: Failed to open HKCU\\Environment key to modify PATH.\n");
    }
}

int main(int argc, char *argv[]) {
    char exe_path[MAX_PATH];
    char workspace[MAX_PATH];
    char version_path[MAX_PATH];
    char version[128];
    char install_root[MAX_PATH];
    char wyrmc_dir[MAX_PATH];
    char wyrpkg_dir[MAX_PATH];
    char packages_dest[MAX_PATH];
    char build_cmd[8192];
    char cmd1[4096];
    char cmd2[4096];
    char cmd3[4096];
    char copy_cmd[4096];
    char *end;
    FILE *vf;
    const char *user_profile;

    GetModuleFileNameA(NULL, exe_path, MAX_PATH);

    if (!find_workspace(exe_path, workspace)) {
        fprintf(stderr, "Error: Could not locate workspace root containing VERSION file.\n");
        return 1;
    }

    snprintf(version_path, sizeof(version_path), "%s\\VERSION", workspace);
    vf = fopen(version_path, "r");
    if (!vf) {
        fprintf(stderr, "Error: Failed to open VERSION file.\n");
        return 1;
    }
    if (!fgets(version, sizeof(version), vf)) {
        strcpy(version, "3.2.0");
    }
    fclose(vf);

    // Strip whitespace from version
    end = version + strlen(version) - 1;
    while (end >= version && (*end == '\r' || *end == '\n' || *end == ' ' || *end == '\t')) {
        *end = '\0';
        end--;
    }

    printf("Bootstrapping Wyrm v%s from workspace: %s\n", version, workspace);

    user_profile = getenv("USERPROFILE");
    if (!user_profile) {
        user_profile = getenv("HOME");
    }
    if (!user_profile) {
        fprintf(stderr, "Error: Could not determine user profile directory.\n");
        return 1;
    }

    snprintf(install_root,   sizeof(install_root),   "%s\\.wyrm",          user_profile);
    snprintf(wyrmc_dir,      sizeof(wyrmc_dir),       "%s\\wyrmc",          install_root);
    snprintf(wyrpkg_dir,     sizeof(wyrpkg_dir),      "%s\\wyrpkg",         install_root);
    snprintf(packages_dest,  sizeof(packages_dest),   "%s\\packages\\wyrmlang", install_root);

    printf("Creating installation directories...\n");
    if (!create_dir_recursive(packages_dest) ||
        !create_dir_recursive(wyrmc_dir) ||
        !create_dir_recursive(wyrpkg_dir)) {
        fprintf(stderr, "Error: Failed to create directories under %s.\n", install_root);
        return 1;
    }

    printf("Compiling compiler (wyrmc) and package manager (wyrpkg)...\n");

    snprintf(cmd1, sizeof(cmd1),
             "gcc -O2 -std=c11 -c \"%s\\wyrm\\lib\\wyrm_core.c\" -o \"%s\\wyrm_core.o\" -I\"%s\\wyrm\\lib\" && "
             "gcc -O2 -std=c11 -c \"%s\\wyrm\\lib\\wyrm_arena.c\" -o \"%s\\wyrm_arena.o\" -I\"%s\\wyrm\\lib\" && "
             "gcc -O2 -std=c11 -c \"%s\\wyrm\\lib\\wyrm_str.c\" -o \"%s\\wyrm_str.o\" -I\"%s\\wyrm\\lib\" && "
             "gcc -O2 -std=c11 -c \"%s\\wyrm\\lib\\wyrm_ffi.c\" -o \"%s\\wyrm_ffi.o\" -I\"%s\\wyrm\\lib\" && "
             "gcc -O2 -std=c11 -c \"%s\\wyrm\\lib\\stdlib\\wyrm_std_json.c\" -o \"%s\\wyrm_std_json.o\" -I\"%s\\wyrm\\lib\" && "
             "gcc -O2 -std=c11 -c \"%s\\wyrm\\lib\\stdlib\\wyrm_std_yaml.c\" -o \"%s\\wyrm_std_yaml.o\" -I\"%s\\wyrm\\lib\" && "
             "gcc -O2 -std=c11 -c \"%s\\wyrm\\lib\\stdlib\\wyrm_std_sdl.c\" -o \"%s\\wyrm_std_sdl.o\" -I\"%s\\wyrm\\lib\" && "
             "gcc -O2 -std=c11 -c \"%s\\wyrm\\lib\\stdlib\\wyrm_std_collections.c\" -o \"%s\\wyrm_std_collections.o\" -I\"%s\\wyrm\\lib\" && "
             "gcc -O2 -std=c11 -c \"%s\\wyrm\\lib\\stdlib\\wyrm_std_random.c\" -o \"%s\\wyrm_std_random.o\" -I\"%s\\wyrm\\lib\"",
             workspace, install_root, workspace,
             workspace, install_root, workspace,
             workspace, install_root, workspace,
             workspace, install_root, workspace,
             workspace, install_root, workspace,
             workspace, install_root, workspace,
             workspace, install_root, workspace,
             workspace, install_root, workspace,
             workspace, install_root, workspace);

    if (system(cmd1) != 0) {
        fprintf(stderr, "Error: Failed to compile C runtime objects.\n");
        return 1;
    }

    snprintf(build_cmd, sizeof(build_cmd),
             "g++ -O2 -std=c++20"
             " \"%s\\wyrm\\scr\\wyrmc.cpp\""
             " \"%s\\compiler\\lexer\\lexer.cpp\""
             " \"%s\\compiler\\parser\\parser.cpp\""
             " \"%s\\compiler\\interpreter\\interpreter.cpp\""
             " \"%s\\compiler\\interpreter\\builtins.cpp\""
             " \"%s\\compiler\\interpreter\\stdlib_setup.cpp\""
             " \"%s\\compiler\\transpiler\\transpiler.cpp\""
             " \"%s\\wyrm_core.o\" \"%s\\wyrm_arena.o\" \"%s\\wyrm_str.o\""
             " \"%s\\wyrm_ffi.o\" \"%s\\wyrm_std_json.o\" \"%s\\wyrm_std_yaml.o\""
             " \"%s\\wyrm_std_sdl.o\" \"%s\\wyrm_std_collections.o\" \"%s\\wyrm_std_random.o\""
             " -o \"%s\\wyrmc_stage0.exe\""
             " -DWYRM_VERSION=\\\"%s\\\" -DWYRMC_VERSION=\\\"%s\\\"",
             workspace, workspace, workspace, workspace, workspace, workspace, workspace,
             install_root, install_root, install_root,
             install_root, install_root, install_root,
             install_root, install_root, install_root,
             install_root, version, version);
    if (system(build_cmd) != 0) {
        fprintf(stderr, "Error: Failed to compile Stage 0 bootstrap compiler. Please ensure g++ is installed and available in PATH.\n");
        return 1;
    }

    printf("Compiling self-hosted compiler (compiler/wyrmc.wyr) -> wyrmc.exe...\n");
    snprintf(build_cmd, sizeof(build_cmd),
             "\"%s\\wyrmc_stage0.exe\" build \"%s\\compiler\\wyrmc.wyr\" -o \"%s\\wyrmc.exe\"",
             install_root, workspace, wyrmc_dir);
    if (system(build_cmd) != 0) {
        fprintf(stderr, "Error: Failed to compile self-hosted wyrmc from compiler/wyrmc.wyr.\n");
        return 1;
    }

    snprintf(build_cmd, sizeof(build_cmd),
             "g++ -O2 -std=c++20 \"%s\\wyrm\\scr\\wyrpkg.cpp\""
             " -o \"%s\\wyrpkg.exe\""
             " -DWYRM_VERSION=\\\"%s\\\" -DWYRPKG_VERSION=\\\"%s\\\"",
             workspace, wyrpkg_dir, version, version);
    if (system(build_cmd) != 0) {
        fprintf(stderr, "Error: Failed to compile wyrpkg.exe. Please ensure g++ is installed and available in PATH.\n");
        return 1;
    }

    printf("Copying Wyrm package runtime files...\n");
    snprintf(copy_cmd, sizeof(copy_cmd),
             "xcopy /E /I /Y \"%s\\wyrm\" \"%s\"", workspace, packages_dest);
    if (system(copy_cmd) != 0) {
        fprintf(stderr, "Warning: Package copy command returned non-zero status.\n");
    }

    printf("Configuring environment PATH...\n");
    add_to_path(wyrmc_dir);
    add_to_path(wyrpkg_dir);

    printf("\nWyrm Toolchain v%s successfully bootstrapped!\n", version);
    printf("  wyrmc  -> %s\\wyrmc.exe\n", wyrmc_dir);
    printf("  wyrpkg -> %s\\wyrpkg.exe\n", wyrpkg_dir);
    printf("Please restart your terminal to apply PATH changes.\n");

    return 0;
}
