#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <cstdlib>
#include <map>

namespace fs = std::filesystem;

#ifndef WYRPKG_VERSION
#define WYRPKG_VERSION "2.4.0"
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
              << "  install <package>   Install a package from local registry or directory\n"
              << "  remove <package>    Remove an installed package\n"
              << "  list                List installed packages\n"
              << "  version, --version  Show version\n"
              << "  help, --help        Show help message\n";
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
        std::cout << "wyrpkg version " << WYRPKG_VERSION << std::endl;
        return 0;
    }
    
    if (cmd == "--help" || cmd == "-h" || cmd == "help") {
        show_help();
        return 0;
    }

    if (cmd == "install") {
        if (argc < 3) {
            std::cerr << "Error: Please specify package name or path to install." << std::endl;
            return 1;
        }
        std::string pkg_name = argv[2];
        fs::create_directories(packages_dir);

        fs::path src_path(pkg_name);
        std::map<std::string, std::string> lock = read_lock_file(lock_file);

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
                std::cout << "Installed package '" << pkg_real_name << "' from " << src_path << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Error: Failed to install package: " << e.what() << std::endl;
                return 1;
            }
        } else {
            fs::path target_dir = packages_dir / pkg_name;
            if (fs::exists(target_dir)) {
                std::cout << "Package '" << pkg_name << "' is already installed." << std::endl;
                return 0;
            }
            try {
                fs::create_directories(target_dir);
                std::ofstream init_wyr(target_dir / "mod.wyr");
                init_wyr << "// Package " << pkg_name << "\nfn info() {\n    print(\"Package " << pkg_name << "\")\n}\n";
                init_wyr.close();

                lock[pkg_name] = "virtual";
                write_lock_file(lock_file, lock);
                std::cout << "Installed package '" << pkg_name << "' successfully." << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Error: Failed to install package: " << e.what() << std::endl;
                return 1;
            }
        }
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
