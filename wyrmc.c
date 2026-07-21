#include <stdio.h>
#include <string.h>

#define WYRMC_VERSION "1.0.0"

int main(int argc, char *argv[]) {
    if (argc > 1) {
        if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0) {
            printf("wyrmc version %s\n", WYRMC_VERSION);
            return 0;
        }
        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            printf("Wyrm Compiler (wyrmc) v%s\n", WYRMC_VERSION);
            printf("Usage: wyrmc [options] [file.wyr]\n");
            printf("Options:\n");
            printf("  --version, -v    Display compiler version\n");
            printf("  --help, -h       Display help information\n");
            return 0;
        }
    }

    printf("wyrmc version %s\n", WYRMC_VERSION);
    printf("Use 'wyrmc --help' for usage.\n");
    return 0;
}
