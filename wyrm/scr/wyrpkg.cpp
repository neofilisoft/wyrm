#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <cstdlib>
#include <map>

namespace fs = std::filesystem;

#ifndef WYRPKG_VERSION
#define WYRPKG_VERSION "2.4.1"
#endif

fs::path get_install_dir() {
    std::string home_dir;
    if (const char* h = std::getenv("USERPROFILE")) {
        home_dir = h;
    } else if (const char* h = std::getenv("HOME")) {
        home_dir = h;
    }
    if (home_dir.empty()) {
        return fs::current_path();
    }
    return fs::path(home_dir) / ".wyrm";
}

std::map<std::string, std::string> read_lock_file(const fs::path& path) {
    std::map<std::string, std::string> packages;
    if (!fs::exists(path)) return packages;
    std::ifstream file(path);
    std::string line;
    while (std::getline(file, line)) {
        size_t colon = line.find(':');
        if (colon == std::string::npos) continue;
        
        std::string key = line.substr(0, colon);
        std::string value = line.substr(colon + 1);
        
        auto clean = [](std::string& s) {
            std::string res;
            for (char c : s) {
                if (c != '"' && c != ',' && c != ' ' && c != '\t' && c != '{' && c != '}') {
                    res += c;
                }
            }
            s = res;
        };
        clean(key);
        clean(value);
        if (!key.empty() && !value.empty()) {
            packages[key] = value;
        }
    }
    return packages;
}

void write_lock_file(const fs::path& path, const std::map<std::string, std::string>& packages) {
    std::ofstream file(path);
    file << "{\n";
    auto it = packages.begin();
    while (it != packages.end()) {
        file << "  \"" << it->first << "\": \"" << it->second << "\"";
        if (++it != packages.end()) {
            file << ",\n";
        } else {
            file << "\n";
        }
    }
    file << "}\n";
}

void show_help() {
    std::cout << "wyrpkg v" << WYRPKG_VERSION << " - Wyrm Package Manager\n\n"
              << "Usage:\n"
              << "  wyrpkg <command> [options]\n\n"
              << "Commands:\n"
              << "  new <name>                    Create a new Wyrm project in a new directory\n"
              << "  init                          Initialize a Wyrm project in the current directory\n"
              << "  build                         Build the current project using wyrmc\n"
              << "  run                           Build and run the current project\n"
              << "  install <pkg|url|owner/repo>  Install a package from Git, GitHub, or local directory\n"
              << "  update <package>              Update an installed Git package to latest version\n"
              << "  publish                       Validate and prepare package for public distribution\n"
              << "  remove <package>              Remove an installed package\n"
              << "  list                          List installed packages\n"
              << "  version, --version            Show version\n"
              << "  help, --help                  Show help message\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        show_help();
        return 0;
    }

    std::string cmd = argv[1];
    fs::path install_dir = get_install_dir();
    fs::path packages_dir = install_dir / "packages";
    fs::path lock_file = install_dir / "wyrpkg.lock";

    if (cmd == "--version" || cmd == "-v" || cmd == "version") {
        std::cout << "wyrpkg v" << WYRPKG_VERSION << std::endl;
        return 0;
    }

    if (cmd == "--help" || cmd == "-h" || cmd == "help") {
        show_help();
        return 0;
    }

    if (cmd == "new") {
        if (argc < 3) {
            std::cerr << "Error: Please specify project name." << std::endl;
            return 1;
        }
        std::string proj_name = argv[2];
        fs::path proj_path = fs::current_path() / proj_name;
        if (fs::exists(proj_path)) {
            std::cerr << "Error: Directory '" << proj_name << "' already exists." << std::endl;
            return 1;
        }
        fs::create_directories(proj_path);
        
        std::ofstream main_file(proj_path / "main.wyr");
        main_file << "fn main() {\n    print(\"Hello from " << proj_name << "!\")\n}\n";
        main_file.close();

        std::ofstream config(proj_path / "wyrproj.json");
        config << "{\n  \"name\": \"" << proj_name << "\",\n  \"version\": \"0.1.0\",\n  \"entry\": \"main.wyr\"\n}\n";
        config.close();

        std::cout << "Created project '" << proj_name << "' successfully." << std::endl;
        return 0;
    }

    if (cmd == "init") {
        std::string proj_name = fs::current_path().filename().string();
        if (!fs::exists("main.wyr") && !fs::exists("mod.wyr")) {
            std::ofstream main_file("main.wyr");
            main_file << "fn main() {\n    print(\"Hello from " << proj_name << "!\")\n}\n";
            main_file.close();
        }
        if (!fs::exists("wyrproj.json")) {
            std::ofstream config("wyrproj.json");
            config << "{\n  \"name\": \"" << proj_name << "\",\n  \"version\": \"0.1.0\",\n  \"entry\": \"main.wyr\"\n}\n";
            config.close();
        }
        std::cout << "Initialized Wyrm project in current directory." << std::endl;
        return 0;
    }

    if (cmd == "build") {
        std::string entry = "main.wyr";
        if (fs::exists("wyrproj.json")) {
            auto config = read_lock_file("wyrproj.json");
            if (config.count("entry")) {
                entry = config["entry"];
            }
        }
        if (!fs::exists(entry)) {
            std::cerr << "Error: Entry file '" << entry << "' not found." << std::endl;
            return 1;
        }
        std::string build_cmd = "wyrmc build " + entry;
        std::cout << "Building project..." << std::endl;
        int res = std::system(build_cmd.c_str());
        if (res != 0) {
            std::cerr << "Error: Build failed." << std::endl;
            return res;
        }
        return 0;
    }

    if (cmd == "run") {
        std::string entry = "main.wyr";
        std::string proj_name = "main";
        if (fs::exists("wyrproj.json")) {
            auto config = read_lock_file("wyrproj.json");
            if (config.count("entry")) {
                entry = config["entry"];
            }
            if (config.count("name")) {
                proj_name = config["name"];
            }
        }
        if (!fs::exists(entry)) {
            std::cerr << "Error: Entry file '" << entry << "' not found." << std::endl;
            return 1;
        }
        std::string build_cmd = "wyrmc build " + entry;
        std::cout << "Building project..." << std::endl;
        int res = std::system(build_cmd.c_str());
        if (res != 0) {
            std::cerr << "Error: Build failed." << std::endl;
            return res;
        }
        
        fs::path entry_path(entry);
        std::string exe_name = entry_path.stem().string();
#ifdef _WIN32
        exe_name += ".exe";
#endif
        
        if (!fs::exists(exe_name)) {
            std::cerr << "Error: Compiled binary '" << exe_name << "' not found." << std::endl;
            return 1;
        }
        
        std::cout << "Running " << exe_name << "..." << std::endl;
        std::string run_cmd = "";
#ifdef _WIN32
        run_cmd = ".\\" + exe_name;
#else
        run_cmd = "./" + exe_name;
#endif
        return std::system(run_cmd.c_str());
    }

    if (cmd == "install") {
        if (argc < 3) {
            std::cerr << "Error: Please specify package name, git URL, or owner/repo to install." << std::endl;
            return 1;
        }
        std::string pkg_arg = argv[2];
        fs::create_directories(packages_dir);

        fs::path src_path(pkg_arg);
        std::map<std::string, std::string> lock = read_lock_file(lock_file);

        // 1. Local filesystem path installation
        if (fs::exists(src_path)) {
            std::string pkg_real_name = src_path.filename().string();
            fs::path target_dir = packages_dir / pkg_real_name;
            if (fs::exists(target_dir)) {
                std::cout << "Package '" << pkg_real_name << "' is already installed." << std::endl;
                return 0;
            }

            try {
                if (fs::is_directory(src_path)) {
                    fs::copy(src_path, target_dir, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
                } else {
                    fs::create_directories(target_dir);
                    fs::copy(src_path, target_dir / src_path.filename(), fs::copy_options::overwrite_existing);
                }
                lock[pkg_real_name] = fs::absolute(src_path).string();
                write_lock_file(lock_file, lock);
                std::cout << "Installed package '" << pkg_real_name << "' from local path " << src_path << std::endl;
                return 0;
            } catch (const std::exception& e) {
                std::cerr << "Error: Failed to install local package: " << e.what() << std::endl;
                return 1;
            }
        }

        // 2. Remote Git / GitHub Package Registry resolution
        std::string git_url;
        std::string pkg_real_name;

        if (pkg_arg.find("http://") == 0 || pkg_arg.find("https://") == 0 || pkg_arg.find("git@") == 0) {
            git_url = pkg_arg;
            size_t last_slash = git_url.find_last_of("/\\");
            pkg_real_name = (last_slash != std::string::npos) ? git_url.substr(last_slash + 1) : pkg_arg;
            if (pkg_real_name.size() > 4 && pkg_real_name.substr(pkg_real_name.size() - 4) == ".git") {
                pkg_real_name = pkg_real_name.substr(0, pkg_real_name.size() - 4);
            }
        } else if (pkg_arg.find('/') != std::string::npos) {
            // Shorthand owner/repo -> https://github.com/owner/repo.git
            git_url = "https://github.com/" + pkg_arg;
            if (git_url.size() < 4 || git_url.substr(git_url.size() - 4) != ".git") {
                git_url += ".git";
            }
            size_t slash = pkg_arg.find_last_of('/');
            pkg_real_name = pkg_arg.substr(slash + 1);
        } else {
            // Official / Community registry resolution: wyrm-lang/<pkg_name>
            git_url = "https://github.com/wyrm-lang/" + pkg_arg + ".git";
            pkg_real_name = pkg_arg;
        }

        fs::path target_dir = packages_dir / pkg_real_name;
        if (fs::exists(target_dir)) {
            std::cout << "Package '" << pkg_real_name << "' is already installed. Use 'wyrpkg update " 
                      << pkg_real_name << "' to update." << std::endl;
            return 0;
        }

        std::cout << "Fetching package '" << pkg_real_name << "' from " << git_url << "..." << std::endl;
        std::string clone_cmd = "git clone --depth 1 \"" + git_url + "\" \"" + target_dir.string() + "\"";
        int clone_res = std::system(clone_cmd.c_str());
        if (clone_res != 0) {
            std::cerr << "Error: Failed to clone package from '" << git_url << "'.\n"
                      << "  Please verify that git is installed and the repository URL exists." << std::endl;
            return 1;
        }

        lock[pkg_real_name] = git_url;
        write_lock_file(lock_file, lock);
        std::cout << "Successfully installed package '" << pkg_real_name << "'!" << std::endl;
        return 0;
    } else if (cmd == "update") {
        if (argc < 3) {
            std::cerr << "Error: Please specify package name to update." << std::endl;
            return 1;
        }
        std::string pkg_name = argv[2];
        fs::path target_dir = packages_dir / pkg_name;
        if (!fs::exists(target_dir)) {
            std::cerr << "Error: Package '" << pkg_name << "' is not installed." << std::endl;
            return 1;
        }

        if (fs::exists(target_dir / ".git")) {
            std::cout << "Updating package '" << pkg_name << "' from remote..." << std::endl;
            std::string pull_cmd = "git -C \"" + target_dir.string() + "\" pull";
            int res = std::system(pull_cmd.c_str());
            if (res != 0) {
                std::cerr << "Error: Failed to pull updates for '" << pkg_name << "'." << std::endl;
                return 1;
            }
            std::cout << "Package '" << pkg_name << "' is now up to date." << std::endl;
            return 0;
        } else {
            std::cout << "Notice: Package '" << pkg_name << "' was installed from a local path and cannot be updated via git." << std::endl;
            return 0;
        }
    } else if (cmd == "publish") {
        if (!fs::exists("wyrproj.json")) {
            std::cerr << "Error: No 'wyrproj.json' found in the current directory. Run 'wyrpkg init' first." << std::endl;
            return 1;
        }

        auto config = read_lock_file("wyrproj.json");
        std::string name = config.count("name") ? config["name"] : "";
        std::string ver = config.count("version") ? config["version"] : "";
        std::string entry = config.count("entry") ? config["entry"] : "main.wyr";

        if (name.empty() || ver.empty()) {
            std::cerr << "Error: 'wyrproj.json' must specify both 'name' and 'version'." << std::endl;
            return 1;
        }

        if (!fs::exists(entry) && !fs::exists("mod.wyr") && !fs::exists("main.wyr")) {
            std::cerr << "Error: Package entry file not found. Ensure 'main.wyr' or 'mod.wyr' exists." << std::endl;
            return 1;
        }

        std::cout << "Validating package for release...\n"
                  << "  Package: " << name << "\n"
                  << "  Version: " << ver << "\n"
                  << "  Entry:   " << entry << "\n";

        std::cout << "\nPackage validation passed! To publish to the Public Registry:\n"
                  << "  1. Push this repository to GitHub (e.g. https://github.com/<your-name>/" << name << ")\n"
                  << "  2. Create and push a Git release tag:\n"
                  << "       git tag v" << ver << "\n"
                  << "       git push origin v" << ver << "\n"
                  << "  3. Anyone can now install your package directly with:\n"
                  << "       wyrpkg install <your-name>/" << name << "\n";
        return 0;
    } else if (cmd == "remove" || cmd == "uninstall") {
        if (argc < 3) {
            std::cerr << "Error: Please specify package name to remove." << std::endl;
            return 1;
        }
        std::string pkg_name = argv[2];
        fs::path target_dir = packages_dir / pkg_name;
        if (!fs::exists(target_dir)) {
            std::cout << "Package '" << pkg_name << "' is not installed." << std::endl;
            return 0;
        }

        try {
            fs::remove_all(target_dir);
            std::map<std::string, std::string> lock = read_lock_file(lock_file);
            lock.erase(pkg_name);
            write_lock_file(lock_file, lock);
            std::cout << "Removed package '" << pkg_name << "'." << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: Failed to remove package: " << e.what() << std::endl;
            return 1;
        }
    } else if (cmd == "list") {
        std::map<std::string, std::string> lock = read_lock_file(lock_file);
        if (lock.empty()) {
            std::cout << "No packages installed." << std::endl;
            return 0;
        }
        std::cout << "Installed packages:" << std::endl;
        for (const auto& pair : lock) {
            std::cout << " - " << pair.first << " (" << pair.second << ")" << std::endl;
        }
    } else {
        std::cout << "Unknown command '" << cmd << "'. Run 'wyrpkg --help' for usage." << std::endl;
        return 1;
    }

    return 0;
}
