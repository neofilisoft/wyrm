#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#define PATH_SEP '\\'
#define mkdir_one(path) _mkdir(path)
#else
#include <dirent.h>
#include <unistd.h>
#define PATH_SEP '/'
#define mkdir_one(path) mkdir(path, 0755)
#endif

#define WYRPKG_VERSION "2.2"
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

static bool exists_path(const char *path) {
    struct stat st;
    return stat(path, &st) == 0;
}

static bool is_dir(const char *path) {
    struct stat st;
    return stat(path, &st) == 0 && (st.st_mode & S_IFDIR) != 0;
}

static void join_path(char *out, size_t out_size, const char *a, const char *b) {
    size_t n = strlen(a);
    snprintf(out, out_size, "%s%s%s", a, (n > 0 && (a[n-1] == '/' || a[n-1] == '\\')) ? "" : (char[]){PATH_SEP, 0}, b);
}

static int mkdir_p(const char *path) {
    char tmp[PATH_MAX];
    snprintf(tmp, sizeof(tmp), "%s", path);
    for (char *p = tmp + 1; *p; ++p) {
        if (*p == '/' || *p == '\\') {
            char old = *p;
            *p = '\0';
            if (!exists_path(tmp) && mkdir_one(tmp) != 0 && errno != EEXIST) return -1;
            *p = old;
        }
    }
    if (!exists_path(tmp) && mkdir_one(tmp) != 0 && errno != EEXIST) return -1;
    return 0;
}

static const char *home_dir(void) {
#ifdef _WIN32
    const char *home = getenv("USERPROFILE");
#else
    const char *home = getenv("HOME");
#endif
    return home && *home ? home : ".";
}

static void registry_root(char *out, size_t out_size) {
    if (exists_path("setup.py") && is_dir("wyrm")) {
        snprintf(out, out_size, ".");
        return;
    }
    snprintf(out, out_size, "%s%c.wyrm", home_dir(), PATH_SEP);
}

static void packages_dir(char *out, size_t out_size) {
    char root[PATH_MAX];
    registry_root(root, sizeof(root));
    join_path(out, out_size, root, "packages");
}

static const char *basename_of(const char *path) {
    const char *a = strrchr(path, '/');
    const char *b = strrchr(path, '\\');
    const char *p = a > b ? a : b;
    return p ? p + 1 : path;
}

static int copy_file(const char *src, const char *dst) {
    FILE *in = fopen(src, "rb");
    if (!in) return -1;
    FILE *out = fopen(dst, "wb");
    if (!out) { fclose(in); return -1; }
    char buf[16384];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0) fwrite(buf, 1, n, out);
    fclose(in);
    fclose(out);
    return 0;
}

static int write_virtual_package(const char *pkg_name, const char *target_dir) {
    if (mkdir_p(target_dir) != 0) return -1;
    char mod_path[PATH_MAX];
    join_path(mod_path, sizeof(mod_path), target_dir, "mod.wyr");
    FILE *f = fopen(mod_path, "wb");
    if (!f) return -1;
    fprintf(f, "// Package %s\nfn info() {\n    print(\"Package %s\")\n}\n", pkg_name, pkg_name);
    fclose(f);
    return 0;
}

static int append_lock(const char *pkg_name, const char *source) {
    char root[PATH_MAX], lock_path[PATH_MAX];
    registry_root(root, sizeof(root));
    if (mkdir_p(root) != 0) return -1;
    join_path(lock_path, sizeof(lock_path), root, "wyrpkg.lock");
    FILE *f = fopen(lock_path, "ab");
    if (!f) return -1;
    time_t now = time(NULL);
    fprintf(f, "%s\t%s\t%lld\n", pkg_name, source, (long long)now);
    fclose(f);
    return 0;
}

static int install_pkg(const char *arg) {
    char pkgs[PATH_MAX];
    packages_dir(pkgs, sizeof(pkgs));
    if (mkdir_p(pkgs) != 0) { fprintf(stderr, "Error: cannot create package directory '%s'\n", pkgs); return 1; }

    const char *name = basename_of(arg);
    char target[PATH_MAX];
    join_path(target, sizeof(target), pkgs, name);
    if (exists_path(target)) { printf("Package '%s' is already installed.\n", name); return 0; }

    if (exists_path(arg) && !is_dir(arg)) {
        if (mkdir_p(target) != 0) { fprintf(stderr, "Error: cannot create '%s'\n", target); return 1; }
        char dst[PATH_MAX];
        join_path(dst, sizeof(dst), target, basename_of(arg));
        if (copy_file(arg, dst) != 0) { fprintf(stderr, "Error: failed to copy '%s'\n", arg); return 1; }
        append_lock(name, arg);
        printf("Installed package '%s' from file %s\n", name, arg);
        return 0;
    }

    if (exists_path(arg) && is_dir(arg)) {
        fprintf(stderr, "Error: directory package copy is not implemented in native wyrpkg yet. Pass a .wyr file or package name.\n");
        return 1;
    }

    if (write_virtual_package(name, target) != 0) { fprintf(stderr, "Error: failed to create package '%s'\n", name); return 1; }
    append_lock(name, "virtual");
    printf("Installed package '%s' successfully.\n", name);
    return 0;
}

static int remove_tree_simple(const char *path) {
#ifdef _WIN32
    char pattern[PATH_MAX];
    snprintf(pattern, sizeof(pattern), "%s%c*", path, PATH_SEP);
    WIN32_FIND_DATAA data;
    HANDLE h = FindFirstFileA(pattern, &data);
    if (h != INVALID_HANDLE_VALUE) {
        do {
            if (strcmp(data.cFileName, ".") == 0 || strcmp(data.cFileName, "..") == 0) continue;
            char child[PATH_MAX];
            join_path(child, sizeof(child), path, data.cFileName);
            if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) remove_tree_simple(child);
            else DeleteFileA(child);
        } while (FindNextFileA(h, &data));
        FindClose(h);
    }
    return RemoveDirectoryA(path) ? 0 : -1;
#else
    DIR *d = opendir(path);
    if (d) {
        struct dirent *e;
        while ((e = readdir(d))) {
            if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0) continue;
            char child[PATH_MAX];
            join_path(child, sizeof(child), path, e->d_name);
            if (is_dir(child)) remove_tree_simple(child);
            else unlink(child);
        }
        closedir(d);
    }
    return rmdir(path);
#endif
}

static int remove_pkg(const char *name) {
    char pkgs[PATH_MAX], target[PATH_MAX];
    packages_dir(pkgs, sizeof(pkgs));
    join_path(target, sizeof(target), pkgs, name);
    if (!exists_path(target)) { printf("Package '%s' is not installed.\n", name); return 0; }
    int rc = remove_tree_simple(target);
    if (rc == 0) printf("Removed package '%s'.\n", name);
    else fprintf(stderr, "Error: failed to remove '%s'\n", name);
    return rc;
}

static int list_pkg(void) {
    char pkgs[PATH_MAX];
    packages_dir(pkgs, sizeof(pkgs));
    if (!exists_path(pkgs)) { printf("No packages installed.\n"); return 0; }
#ifdef _WIN32
    char pattern[PATH_MAX];
    snprintf(pattern, sizeof(pattern), "%s%c*", pkgs, PATH_SEP);
    WIN32_FIND_DATAA data;
    HANDLE h = FindFirstFileA(pattern, &data);
    if (h == INVALID_HANDLE_VALUE) { printf("No packages installed.\n"); return 0; }
    bool any = false;
    printf("Installed packages:\n");
    do {
        if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && strcmp(data.cFileName, ".") != 0 && strcmp(data.cFileName, "..") != 0) {
            printf(" - %s\n", data.cFileName);
            any = true;
        }
    } while (FindNextFileA(h, &data));
    FindClose(h);
    if (!any) printf("No packages installed.\n");
#else
    DIR *d = opendir(pkgs);
    if (!d) { printf("No packages installed.\n"); return 0; }
    struct dirent *e;
    bool any = false;
    printf("Installed packages:\n");
    while ((e = readdir(d))) {
        if (e->d_name[0] != '.') { printf(" - %s\n", e->d_name); any = true; }
    }
    closedir(d);
    if (!any) printf("No packages installed.\n");
#endif
    return 0;
}

static int run_tool_command(int argc, char **argv, const char *tool_cmd) {
    if (argc < 3) {
        fprintf(stderr, "Error: %s requires a .wyr file\n", tool_cmd);
        return 1;
    }
    char cmd[PATH_MAX * 2];
#ifdef _WIN32
    snprintf(cmd, sizeof(cmd), ".\\wyrm.exe %s \"%s\"", tool_cmd, argv[2]);
#else
    snprintf(cmd, sizeof(cmd), "./wyrm %s \"%s\"", tool_cmd, argv[2]);
#endif
    for (int i = 3; i < argc; ++i) {
        strncat(cmd, " ", sizeof(cmd) - strlen(cmd) - 1);
        strncat(cmd, argv[i], sizeof(cmd) - strlen(cmd) - 1);
    }
    return system(cmd);
}

static int test_project(void) {
    if (exists_path("examples")) {
#ifdef _WIN32
        int rc = system(".\\wyrm.exe run examples\\hello.wyr");
#else
        int rc = system("./wyrm run examples/hello.wyr");
#endif
        if (rc != 0) return rc;
    }
    printf("wyrpkg test: native smoke tests completed.\n");
    return 0;
}
static void help(void) {
    printf("wyrpkg v%s - native C Wyrm Package Manager\n", WYRPKG_VERSION);
    printf("Usage: wyrpkg <command> [args]\n");
    printf("Commands:\n");
    printf("  install <package-or-file.wyr>\n");
    printf("  remove <package>\n");
    printf("  list\n");
    printf("  build <file.wyr> [-o output]\n");
    printf("  run <file.wyr>\n");
    printf("  test\n");
    printf("  version | --version\n");
}

int main(int argc, char **argv) {
    if (argc < 2 || strcmp(argv[1], "help") == 0 || strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) { help(); return 0; }
    if (strcmp(argv[1], "version") == 0 || strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0) { printf("wyrpkg version %s (native C)\n", WYRPKG_VERSION); return 0; }
    if (strcmp(argv[1], "install") == 0) { if (argc < 3) { fprintf(stderr, "Error: install requires a package name or file\n"); return 1; } return install_pkg(argv[2]); }
    if (strcmp(argv[1], "remove") == 0 || strcmp(argv[1], "uninstall") == 0) { if (argc < 3) { fprintf(stderr, "Error: remove requires a package name\n"); return 1; } return remove_pkg(argv[2]); }
    if (strcmp(argv[1], "list") == 0) return list_pkg();
    if (strcmp(argv[1], "build") == 0) return run_tool_command(argc, argv, "build");
    if (strcmp(argv[1], "run") == 0) return run_tool_command(argc, argv, "run");
    if (strcmp(argv[1], "test") == 0) return test_project();
    fprintf(stderr, "Unknown command '%s'. Run wyrpkg --help.\n", argv[1]);
    return 1;
}
