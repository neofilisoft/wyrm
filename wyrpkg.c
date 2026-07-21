#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WYRPKG_VERSION "0.1.0"

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

    // Pass execution to python wyrpkg.py
    char cmd[1024] = "python wyrpkg.py";
    for (int i = 1; i < argc; i++) {
        strcat(cmd, " ");
        strcat(cmd, argv[i]);
    }
    return system(cmd);
}
