#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>
#include <cstdlib>
#include <map>
#include <cctype>
#include <utility>

namespace fs = std::filesystem;

#ifndef WYRPKG_VERSION
#define WYRPKG_VERSION "1.2.1"
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
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    size_t i = 0;
    while (i < content.size()) {
        size_t k_start = content.find('"', i);
        if (k_start == std::string::npos) break;
        size_t k_end = content.find('"', k_start + 1);
        if (k_end == std::string::npos) break;
        std::string key = content.substr(k_start + 1, k_end - k_start - 1);

        size_t colon = content.find(':', k_end + 1);
        if (colon == std::string::npos) break;

        size_t v_start = content.find('"', colon + 1);
        if (v_start == std::string::npos) break;
        size_t v_end = content.find('"', v_start + 1);
        if (v_end == std::string::npos) break;
        std::string val = content.substr(v_start + 1, v_end - v_start - 1);

        packages[key] = val;
        i = v_end + 1;
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

struct ProjectManifest {
    std::string name;
    std::string version;
    std::string entry = "main.wyr";
    std::map<std::string, std::string> dependencies;
    bool found = false;
    std::string file_path;
};

static inline std::string trim_str(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

static inline std::string strip_quotes(const std::string& str) {
    std::string s = trim_str(str);
    if (s.size() >= 2 && ((s.front() == '"' && s.back() == '"') || (s.front() == '\'' && s.back() == '\''))) {
        return s.substr(1, s.size() - 2);
    }
    return s;
}

// --- Versioned project lockfile ------------------------------------------------
// The global ~/.wyrm/wyrpkg.lock stays a flat name -> source map for the legacy
// commands. Project reproducibility uses the typed, versioned format below: a
// remote identity plus the immutable git revision that was actually resolved.

inline constexpr int kLockfileVersion = 1;

struct LockedPackage {
    std::string source;
    std::string revision;
    std::string version;
};

struct WyrmLockfile {
    int version = kLockfileVersion;
    std::map<std::string, LockedPackage> packages;
};

struct LockfileReadResult {
    bool ok = false;
    std::string error;
    WyrmLockfile lockfile;
};

static std::string lowercase(const std::string& value) {
    std::string result(value);
    for (char& ch : result) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return result;
}

static bool is_hex_revision(const std::string& value) {
    if (value.size() != 40 && value.size() != 64) return false;
    for (unsigned char ch : value) {
        if (!std::isxdigit(ch)) return false;
    }
    return true;
}

static bool is_remote_source(const std::string& source) {
    return source.rfind("http://", 0) == 0 || source.rfind("https://", 0) == 0 ||
           source.rfind("git@", 0) == 0 || source.rfind("ssh://", 0) == 0 ||
           source.rfind("git://", 0) == 0;
}

static std::string json_escape(const std::string& value) {
    static const char hex_digits[] = "0123456789abcdef";
    std::string result;
    result.reserve(value.size() + 2);
    for (unsigned char ch : value) {
        switch (ch) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\b': result += "\\b"; break;
            case '\f': result += "\\f"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default:
                if (ch < 0x20) {
                    result += "\\u00";
                    result += hex_digits[(ch >> 4) & 0xF];
                    result += hex_digits[ch & 0xF];
                } else {
                    result += static_cast<char>(ch);
                }
        }
    }
    return result;
}

static void append_utf8(unsigned int code_point, std::string& out) {
    if (code_point <= 0x7F) {
        out += static_cast<char>(code_point);
    } else if (code_point <= 0x7FF) {
        out += static_cast<char>(0xC0 | (code_point >> 6));
        out += static_cast<char>(0x80 | (code_point & 0x3F));
    } else if (code_point <= 0xFFFF) {
        out += static_cast<char>(0xE0 | (code_point >> 12));
        out += static_cast<char>(0x80 | ((code_point >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (code_point & 0x3F));
    } else {
        out += static_cast<char>(0xF0 | (code_point >> 18));
        out += static_cast<char>(0x80 | ((code_point >> 12) & 0x3F));
        out += static_cast<char>(0x80 | ((code_point >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (code_point & 0x3F));
    }
}

class LockfileParser {
public:
    explicit LockfileParser(const std::string& text) : text_(text) {}

    LockfileReadResult parse() {
        LockfileReadResult result;
        skip_whitespace();
        if (!expect('{', result.error)) return result;
        bool saw_version = false;
        bool saw_packages = false;
        skip_whitespace();
        if (!consume('}')) {
            while (true) {
                std::string key;
                if (!parse_string(key, result.error)) return result;
                skip_whitespace();
                if (!expect(':', result.error)) return result;
                skip_whitespace();
                if (key == "version") {
                    if (saw_version) {
                        result.error = "duplicate 'version' key";
                        return result;
                    }
                    long long number = 0;
                    if (!parse_integer(number, result.error)) return result;
                    if (number != kLockfileVersion) {
                        result.error = "unsupported lockfile version " + std::to_string(number);
                        return result;
                    }
                    result.lockfile.version = static_cast<int>(number);
                    saw_version = true;
                } else if (key == "packages") {
                    if (saw_packages) {
                        result.error = "duplicate 'packages' key";
                        return result;
                    }
                    if (!parse_packages(result.lockfile.packages, result.error)) return result;
                    saw_packages = true;
                } else {
                    result.error = "unknown lockfile key '" + key + "'";
                    return result;
                }
                skip_whitespace();
                if (consume(',')) {
                    skip_whitespace();
                    continue;
                }
                if (!expect('}', result.error)) return result;
                break;
            }
        }
        skip_whitespace();
        if (position_ != text_.size()) {
            result.error = "trailing characters after lockfile";
            return result;
        }
        if (!saw_version || !saw_packages) {
            result.error = "lockfile must contain both 'version' and 'packages'";
            return result;
        }
        result.ok = true;
        return result;
    }

private:
    const std::string& text_;
    size_t position_ = 0;

    void skip_whitespace() {
        while (position_ < text_.size()) {
            const char ch = text_[position_];
            if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') {
                ++position_;
            } else {
                break;
            }
        }
    }

    bool consume(char expected) {
        if (position_ < text_.size() && text_[position_] == expected) {
            ++position_;
            return true;
        }
        return false;
    }

    bool expect(char expected, std::string& error) {
        if (consume(expected)) return true;
        error = std::string("expected '") + expected + "' at offset " + std::to_string(position_);
        return false;
    }

    bool parse_hex4(unsigned int& value, std::string& error) {
        if (position_ + 4 > text_.size()) {
            error = "truncated unicode escape";
            return false;
        }
        value = 0;
        for (int i = 0; i < 4; ++i) {
            const unsigned char ch = static_cast<unsigned char>(text_[position_++]);
            unsigned int digit = 0;
            if (ch >= '0' && ch <= '9') digit = ch - '0';
            else if (ch >= 'a' && ch <= 'f') digit = ch - 'a' + 10;
            else if (ch >= 'A' && ch <= 'F') digit = ch - 'A' + 10;
            else {
                error = "invalid unicode escape";
                return false;
            }
            value = (value << 4) | digit;
        }
        return true;
    }

    bool parse_string(std::string& out, std::string& error) {
        out.clear();
        if (!consume('"')) {
            error = "expected string at offset " + std::to_string(position_);
            return false;
        }
        while (position_ < text_.size()) {
            const unsigned char ch = static_cast<unsigned char>(text_[position_++]);
            if (ch == '"') return true;
            if (ch < 0x20) {
                error = "unescaped control character in string";
                return false;
            }
            if (ch != '\\') {
                out += static_cast<char>(ch);
                continue;
            }
            if (position_ >= text_.size()) {
                error = "unterminated escape sequence";
                return false;
            }
            const char escape_char = text_[position_++];
            switch (escape_char) {
                case '"': out += '"'; break;
                case '\\': out += '\\'; break;
                case '/': out += '/'; break;
                case 'b': out += '\b'; break;
                case 'f': out += '\f'; break;
                case 'n': out += '\n'; break;
                case 'r': out += '\r'; break;
                case 't': out += '\t'; break;
                case 'u': {
                    unsigned int code_point = 0;
                    if (!parse_hex4(code_point, error)) return false;
                    if (code_point >= 0xD800 && code_point <= 0xDBFF) {
                        if (position_ + 1 >= text_.size() ||
                            text_[position_] != '\\' || text_[position_ + 1] != 'u') {
                            error = "invalid surrogate pair";
                            return false;
                        }
                        position_ += 2;
                        unsigned int low_surrogate = 0;
                        if (!parse_hex4(low_surrogate, error)) return false;
                        if (low_surrogate < 0xDC00 || low_surrogate > 0xDFFF) {
                            error = "invalid low surrogate";
                            return false;
                        }
                        code_point = 0x10000 + ((code_point - 0xD800) << 10) + (low_surrogate - 0xDC00);
                    } else if (code_point >= 0xDC00 && code_point <= 0xDFFF) {
                        error = "unexpected low surrogate";
                        return false;
                    }
                    append_utf8(code_point, out);
                    break;
                }
                default:
                    error = "unknown escape sequence";
                    return false;
            }
        }
        error = "unterminated string";
        return false;
    }

    bool parse_integer(long long& out, std::string& error) {
        const size_t start = position_;
        while (position_ < text_.size() &&
               std::isdigit(static_cast<unsigned char>(text_[position_]))) {
            ++position_;
        }
        if (position_ == start) {
            error = "expected integer at offset " + std::to_string(position_);
            return false;
        }
        try {
            out = std::stoll(text_.substr(start, position_ - start));
        } catch (const std::exception&) {
            error = "lockfile version is out of range";
            return false;
        }
        return true;
    }

    bool parse_package(LockedPackage& out, std::string& error) {
        if (!expect('{', error)) return false;
        bool saw_source = false;
        bool saw_revision = false;
        bool saw_version = false;
        skip_whitespace();
        if (!consume('}')) {
            while (true) {
                std::string key;
                if (!parse_string(key, error)) return false;
                skip_whitespace();
                if (!expect(':', error)) return false;
                skip_whitespace();
                std::string value;
                if (!parse_string(value, error)) return false;
                if (key == "source") {
                    if (saw_source) { error = "duplicate 'source' field"; return false; }
                    out.source = value;
                    saw_source = true;
                } else if (key == "revision") {
                    if (saw_revision) { error = "duplicate 'revision' field"; return false; }
                    out.revision = lowercase(value);
                    saw_revision = true;
                } else if (key == "version") {
                    if (saw_version) { error = "duplicate 'version' field"; return false; }
                    out.version = value;
                    saw_version = true;
                } else {
                    error = "unknown package field '" + key + "'";
                    return false;
                }
                skip_whitespace();
                if (consume(',')) {
                    skip_whitespace();
                    continue;
                }
                if (!expect('}', error)) return false;
                break;
            }
        }
        if (!saw_source || !saw_revision || !saw_version) {
            error = "package entry must contain 'source', 'revision' and 'version'";
            return false;
        }
        return true;
    }

    bool parse_packages(std::map<std::string, LockedPackage>& packages, std::string& error) {
        if (!expect('{', error)) return false;
        skip_whitespace();
        if (consume('}')) return true;
        while (true) {
            std::string name;
            if (!parse_string(name, error)) return false;
            if (name.empty()) {
                error = "package name must not be empty";
                return false;
            }
            if (packages.find(name) != packages.end()) {
                error = "duplicate package '" + name + "'";
                return false;
            }
            skip_whitespace();
            if (!expect(':', error)) return false;
            skip_whitespace();
            LockedPackage package;
            if (!parse_package(package, error)) return false;
            if (package.source.empty()) {
                error = "package '" + name + "' has no source";
                return false;
            }
            if (!package.revision.empty() && !is_hex_revision(package.revision)) {
                error = "package '" + name + "' has a malformed revision";
                return false;
            }
            if (is_remote_source(package.source) && package.revision.empty()) {
                error = "package '" + name + "' has no immutable git revision";
                return false;
            }
            packages.emplace(name, std::move(package));
            skip_whitespace();
            if (consume(',')) {
                skip_whitespace();
                continue;
            }
            if (!expect('}', error)) return false;
            return true;
        }
    }
};

static std::string serialize_lockfile(const WyrmLockfile& lockfile) {
    std::ostringstream out;
    out << "{\n  \"version\": " << lockfile.version << ",\n";
    if (lockfile.packages.empty()) {
        out << "  \"packages\": {}\n";
    } else {
        out << "  \"packages\": {\n";
        bool first = true;
        for (const auto& entry : lockfile.packages) {
            if (!first) out << ",\n";
            first = false;
            out << "    \"" << json_escape(entry.first) << "\": {\n"
                << "      \"source\": \"" << json_escape(entry.second.source) << "\",\n"
                << "      \"revision\": \"" << json_escape(entry.second.revision) << "\",\n"
                << "      \"version\": \"" << json_escape(entry.second.version) << "\"\n"
                << "    }";
        }
        out << "\n  }\n";
    }
    out << "}\n";
    return out.str();
}

static LockfileReadResult read_project_lockfile(const fs::path& path) {
    LockfileReadResult result;
    if (!fs::exists(path)) {
        result.ok = true;
        return result;
    }
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        result.error = "cannot open '" + path.string() + "'";
        return result;
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return LockfileParser(buffer.str()).parse();
}

static bool write_project_lockfile(const fs::path& path, const WyrmLockfile& lockfile, std::string& error) {
    const fs::path staging_path = path.string() + ".tmp";
    {
        std::ofstream file(staging_path, std::ios::binary | std::ios::trunc);
        if (!file) {
            error = "cannot write '" + staging_path.string() + "'";
            return false;
        }
        file << serialize_lockfile(lockfile);
        if (!file) {
            error = "failed while writing '" + staging_path.string() + "'";
            return false;
        }
    }
    std::error_code rename_error;
    fs::rename(staging_path, path, rename_error);
    if (rename_error) {
        std::error_code cleanup_error;
        fs::remove(staging_path, cleanup_error);
        error = "cannot replace '" + path.string() + "': " + rename_error.message();
        return false;
    }
    return true;
}

static std::string find_packed_ref(const fs::path& git_dir, const std::string& ref) {
    std::ifstream file(git_dir / "packed-refs");
    if (!file) return std::string();
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#' || line[0] == '^') continue;
        const size_t space = line.find(' ');
        if (space == std::string::npos) continue;
        if (trim_str(line.substr(space + 1)) == ref) {
            return lowercase(trim_str(line.substr(0, space)));
        }
    }
    return std::string();
}

struct GitHeadResult {
    bool ok = false;
    std::string revision;
    std::string error;
};

static fs::path git_metadata_dir(const fs::path& checkout, std::string& error) {
    const fs::path dot_git = checkout / ".git";
    if (fs::is_directory(dot_git)) return dot_git;
    if (fs::is_regular_file(dot_git)) {
        std::ifstream file(dot_git);
        std::string line;
        if (std::getline(file, line)) {
            const std::string prefix = "gitdir:";
            const std::string contents = trim_str(line);
            if (contents.compare(0, prefix.size(), prefix) == 0) {
                fs::path target(trim_str(contents.substr(prefix.size())));
                if (!target.empty()) {
                    if (target.is_absolute()) return target;
                    return (checkout / target).lexically_normal();
                }
            }
        }
    }
    error = "no git metadata in '" + checkout.string() + "'";
    return fs::path();
}

// Filesystem-only revision lookup: no process is spawned and no shell is used.
static GitHeadResult read_git_head(const fs::path& checkout) {
    GitHeadResult result;
    std::string metadata_error;
    const fs::path git_dir = git_metadata_dir(checkout, metadata_error);
    if (git_dir.empty()) {
        result.error = metadata_error;
        return result;
    }
    std::ifstream head_file(git_dir / "HEAD");
    if (!head_file) {
        result.error = "cannot read git HEAD in '" + checkout.string() + "'";
        return result;
    }
    std::string head;
    std::getline(head_file, head);
    head = trim_str(head);

    const std::string ref_prefix = "ref: ";
    if (head.compare(0, ref_prefix.size(), ref_prefix) == 0) {
        const std::string ref = trim_str(head.substr(ref_prefix.size()));
        if (ref.rfind("refs/", 0) != 0 || ref.find("..") != std::string::npos) {
            result.error = "invalid git ref '" + ref + "'";
            return result;
        }
        std::ifstream ref_file(git_dir / ref);
        std::string revision;
        if (ref_file && std::getline(ref_file, revision)) {
            result.revision = lowercase(trim_str(revision));
        } else {
            result.revision = find_packed_ref(git_dir, ref);
        }
        const std::string heads_prefix = "refs/heads/";
        if (result.revision.empty() && ref.rfind(heads_prefix, 0) == 0) {
            const std::string remote_ref = "refs/remotes/origin/" + ref.substr(heads_prefix.size());
            std::ifstream remote_file(git_dir / remote_ref);
            std::string remote_revision;
            if (remote_file && std::getline(remote_file, remote_revision)) {
                result.revision = lowercase(trim_str(remote_revision));
            }
            if (result.revision.empty()) result.revision = find_packed_ref(git_dir, remote_ref);
        }
        if (!is_hex_revision(result.revision)) {
            result.error = "cannot resolve git ref '" + ref + "' to a commit in '" + checkout.string() + "'";
            result.revision.clear();
            return result;
        }
    } else {
        result.revision = lowercase(head);
        if (!is_hex_revision(result.revision)) {
            result.error = "invalid git HEAD in '" + checkout.string() + "'";
            result.revision.clear();
            return result;
        }
    }
    result.ok = true;
    return result;
}

ProjectManifest read_toml_manifest(const fs::path& path) {
    ProjectManifest manifest;
    if (!fs::exists(path)) return manifest;
    std::ifstream file(path);
    std::string line;
    std::string current_section = "";
    manifest.found = true;
    manifest.file_path = path.string();

    while (std::getline(file, line)) {
        size_t comment_pos = line.find('#');
        if (comment_pos != std::string::npos) {
            line = line.substr(0, comment_pos);
        }
        line = trim_str(line);
        if (line.empty()) continue;

        if (line.front() == '[' && line.back() == ']') {
            current_section = trim_str(line.substr(1, line.size() - 2));
            continue;
        }

        size_t eq_pos = line.find('=');
        if (eq_pos == std::string::npos) continue;

        std::string key = trim_str(line.substr(0, eq_pos));
        std::string val = strip_quotes(line.substr(eq_pos + 1));

        if (current_section == "package" || current_section.empty()) {
            if (key == "name") manifest.name = val;
            else if (key == "version") manifest.version = val;
            else if (key == "entry") manifest.entry = val;
        } else if (current_section == "dependencies") {
            if (!key.empty()) {
                manifest.dependencies[key] = val;
            }
        }
    }
    return manifest;
}

ProjectManifest read_project_manifest() {
    if (fs::exists("wyrpkg.toml")) {
        return read_toml_manifest("wyrpkg.toml");
    }
    if (fs::exists("wyrproj.json")) {
        ProjectManifest manifest;
        auto config = read_lock_file("wyrproj.json");
        manifest.found = true;
        manifest.file_path = "wyrproj.json";
        if (config.count("name")) manifest.name = config["name"];
        if (config.count("version")) manifest.version = config["version"];
        if (config.count("entry")) manifest.entry = config["entry"];
        return manifest;
    }
    return ProjectManifest{};
}

void write_toml_manifest(const fs::path& path, const std::string& name, const std::string& version, const std::string& entry) {
    std::ofstream file(path);
    file << "[package]\n";
    file << "name = \"" << name << "\"\n";
    file << "version = \"" << version << "\"\n";
    file << "entry = \"" << entry << "\"\n\n";
    file << "[dependencies]\n";
}

static std::string derive_dependency_source(const std::string& dependency) {
    if (is_remote_source(dependency)) return dependency;
    if (dependency.find('/') != std::string::npos) {
        std::string url = "https://github.com/" + dependency;
        if (url.size() < 4 || url.substr(url.size() - 4) != ".git") url += ".git";
        return url;
    }
    return "https://github.com/wyrm-lang/" + dependency + ".git";
}

struct InstallLockResult {
    bool ok = false;
    std::string error;
    bool write_lock = false;
    WyrmLockfile lockfile;
};

// Runs after a package has been placed on disk. It never builds a command
// string: it only reads git metadata from the filesystem and validates the
// pinned revision against what was actually resolved.
static InstallLockResult prepare_install_lock(const std::string& package_name,
                                              const std::string& source,
                                              const fs::path& target_dir,
                                              const std::string& fallback_version) {
    InstallLockResult result;
    const fs::path lock_path = "wyrpkg.lock";
    const bool project_present = fs::exists("wyrpkg.toml") || fs::exists("wyrproj.json");
    const bool lock_present = fs::exists(lock_path);
    if (!project_present && !lock_present) {
        result.ok = true;
        return result;
    }

    auto existing = read_project_lockfile(lock_path);
    if (!existing.ok) {
        result.error = "Malformed project lockfile: " + existing.error;
        return result;
    }
    result.lockfile = existing.lockfile;

    LockedPackage entry;
    entry.source = source;
    if (fs::exists(target_dir / ".git")) {
        auto head = read_git_head(target_dir);
        if (!head.ok) {
            result.error = "Cannot resolve installed revision: " + head.error;
            return result;
        }
        entry.revision = head.revision;
    }
    auto installed_manifest = read_toml_manifest(target_dir / "wyrpkg.toml");
    entry.version = installed_manifest.version.empty() ? fallback_version : installed_manifest.version;
    if (entry.version.empty()) entry.version = "latest";

    if (is_remote_source(entry.source) && entry.revision.empty()) {
        result.error = "Cannot pin remote package '" + package_name + "' without a git revision";
        return result;
    }
    auto pinned = result.lockfile.packages.find(package_name);
    if (pinned != result.lockfile.packages.end() &&
        !pinned->second.revision.empty() && pinned->second.revision != entry.revision) {
        result.error = "Locked revision " + pinned->second.revision + " for '" + package_name +
                       "' does not match installed revision " + entry.revision +
                       "; refusing to install a drifting dependency";
        return result;
    }
    result.lockfile.packages[package_name] = entry;
    result.write_lock = true;
    result.ok = true;
    return result;
}

void show_help() {
    std::cout << "wyrpkg " << WYRPKG_VERSION << " - Wyrm Package Manager\n\n"
              << "Usage:\n"
              << "  wyrpkg <command> [options]\n\n"
              << "Commands:\n"
              << "  new <name>                    Create a new Wyrm project with wyrpkg.toml\n"
              << "  init                          Initialize a Wyrm project (wyrpkg.toml) in current directory\n"
              << "  build                         Build the current project using wyrmc\n"
              << "  run                           Build and run the current project\n"
              << "  install <pkg|url|owner/repo>  Install a package from Git, GitHub, or local directory\n"
              << "  update <package>              Update an installed Git package to latest version\n"
              << "  add <package> [version]       Add a dependency to wyrpkg.toml\n"
              << "  lock                          Resolve dependencies into a versioned wyrpkg.lock\n"
              << "  publish                       Validate and prepare package for public distribution\n"
              << "  remove, uninstall <package>   Remove or uninstall an installed package\n"
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
        std::cout << "wyrpkg " << WYRPKG_VERSION << std::endl;
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

        write_toml_manifest(proj_path / "wyrpkg.toml", proj_name, "0.1.0", "main.wyr");

        std::cout << "Created project '" << proj_name << "' with wyrpkg.toml successfully." << std::endl;
        return 0;
    }

    if (cmd == "init") {
        std::string proj_name = fs::current_path().filename().string();
        if (!fs::exists("main.wyr") && !fs::exists("mod.wyr")) {
            std::ofstream main_file("main.wyr");
            main_file << "fn main() {\n    print(\"Hello from " << proj_name << "!\")\n}\n";
            main_file.close();
        }
        if (!fs::exists("wyrpkg.toml") && !fs::exists("wyrproj.json")) {
            write_toml_manifest("wyrpkg.toml", proj_name, "0.1.0", "main.wyr");
        }
        std::cout << "Initialized Wyrm project (wyrpkg.toml) in current directory." << std::endl;
        return 0;
    }

    if (cmd == "build") {
        std::string entry = "main.wyr";
        auto manifest = read_project_manifest();
        if (manifest.found && !manifest.entry.empty()) {
            entry = manifest.entry;
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
        auto manifest = read_project_manifest();
        if (manifest.found && !manifest.entry.empty()) {
            entry = manifest.entry;
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
                const std::string source_path = fs::absolute(src_path).string();
                auto pin = prepare_install_lock(pkg_real_name, source_path, target_dir, "");
                if (!pin.ok) {
                    std::cerr << "Error: " << pin.error << std::endl;
                    std::error_code cleanup_error;
                    fs::remove_all(target_dir, cleanup_error);
                    return 1;
                }
                if (pin.write_lock) {
                    std::string write_error;
                    if (!write_project_lockfile("wyrpkg.lock", pin.lockfile, write_error)) {
                        std::cerr << "Error: " << write_error << std::endl;
                        std::error_code cleanup_error;
                        fs::remove_all(target_dir, cleanup_error);
                        return 1;
                    }
                }
                lock[pkg_real_name] = source_path;
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

        auto pin = prepare_install_lock(pkg_real_name, git_url, target_dir, "");
        if (!pin.ok) {
            std::cerr << "Error: " << pin.error << std::endl;
            std::error_code cleanup_error;
            fs::remove_all(target_dir, cleanup_error);
            return 1;
        }
        if (pin.write_lock) {
            std::string write_error;
            if (!write_project_lockfile("wyrpkg.lock", pin.lockfile, write_error)) {
                std::cerr << "Error: " << write_error << std::endl;
                std::error_code cleanup_error;
                fs::remove_all(target_dir, cleanup_error);
                return 1;
            }
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
        auto manifest = read_project_manifest();
        if (!manifest.found) {
            std::cerr << "Error: No 'wyrpkg.toml' found in the current directory. Run 'wyrpkg init' first." << std::endl;
            return 1;
        }

        std::string name = manifest.name;
        std::string ver = manifest.version;
        std::string entry = manifest.entry.empty() ? "main.wyr" : manifest.entry;

        if (name.empty() || ver.empty()) {
            std::cerr << "Error: '" << manifest.file_path << "' must specify both 'name' and 'version'." << std::endl;
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
    } else if (cmd == "add") {
        if (argc < 3) {
            std::cerr << "Error: Usage: wyrpkg add <package> [version]" << std::endl;
            return 1;
        }
        std::string pkg_name = argv[2];
        std::string pkg_version = (argc >= 4) ? argv[3] : "latest";

        // Find or create wyrpkg.toml
        fs::path toml_path = "wyrpkg.toml";
        if (!fs::exists(toml_path)) {
            std::string proj = fs::current_path().filename().string();
            write_toml_manifest(toml_path, proj, "0.1.0", "main.wyr");
        }

        // Read existing content and append dependency
        std::ifstream rf(toml_path);
        std::string content((std::istreambuf_iterator<char>(rf)), std::istreambuf_iterator<char>());
        rf.close();

        // Check if [dependencies] section exists
        bool has_deps = content.find("[dependencies]") != std::string::npos;
        std::string dep_line = pkg_name + " = \"" + pkg_version + "\"";

        if (!has_deps) {
            content += "\n[dependencies]\n";
        }
        // Check if already listed
        if (content.find(pkg_name + " =") != std::string::npos) {
            std::cout << "Package '" << pkg_name << "' already in wyrpkg.toml. Update the version manually if needed." << std::endl;
            return 0;
        }
        content += dep_line + "\n";

        std::ofstream wf(toml_path);
        wf << content;
        std::cout << "Added '" << pkg_name << " = \"" << pkg_version << "\"' to wyrpkg.toml" << std::endl;
        return 0;
    } else if (cmd == "lock") {
        auto manifest = read_project_manifest();
        if (!manifest.found) {
            std::cerr << "Error: No wyrpkg.toml found. Run 'wyrpkg init' first." << std::endl;
            return 1;
        }

        const fs::path project_lock_path = "wyrpkg.lock";
        auto existing_lock = read_project_lockfile(project_lock_path);
        if (!existing_lock.ok) {
            std::cerr << "Error: Malformed project lockfile '" << project_lock_path << "': "
                      << existing_lock.error << std::endl;
            return 1;
        }

        auto global_lock = read_lock_file(lock_file);
        WyrmLockfile resolved_lock;

        for (const auto& dependency : manifest.dependencies) {
            const std::string& dep_name = dependency.first;
            const fs::path dep_path = packages_dir / dep_name;
            LockedPackage entry;
            entry.version = dependency.second;

            if (fs::exists(dep_path)) {
                auto installed_manifest = read_toml_manifest(dep_path / "wyrpkg.toml");
                if (!installed_manifest.version.empty()) entry.version = installed_manifest.version;

                if (fs::exists(dep_path / ".git")) {
                    auto head = read_git_head(dep_path);
                    if (!head.ok) {
                        std::cerr << "Error: Cannot resolve pinned revision for '" << dep_name << "': "
                                  << head.error << std::endl;
                        return 1;
                    }
                    entry.revision = head.revision;

                    auto pinned = existing_lock.lockfile.packages.find(dep_name);
                    if (pinned != existing_lock.lockfile.packages.end() &&
                        !pinned->second.revision.empty() && pinned->second.revision != entry.revision) {
                        std::cerr << "Error: Locked revision " << pinned->second.revision
                                  << " for '" << dep_name << "' does not match installed revision "
                                  << entry.revision << "; review the lockfile before updating." << std::endl;
                        return 1;
                    }
                    if (pinned != existing_lock.lockfile.packages.end() && !pinned->second.source.empty()) {
                        entry.source = pinned->second.source;
                    } else if (global_lock.count(dep_name)) {
                        entry.source = global_lock[dep_name];
                    } else {
                        entry.source = derive_dependency_source(dep_name);
                    }
                } else {
                    entry.source = global_lock.count(dep_name) ? global_lock[dep_name]
                                                              : fs::absolute(dep_path).string();
                }
            } else {
                std::cerr << "Error: Cannot pin dependency '" << dep_name
                          << "': package is not installed. Run 'wyrpkg install " << dep_name
                          << "' first." << std::endl;
                return 1;
            }

            if (entry.version.empty()) entry.version = "latest";
            resolved_lock.packages[dep_name] = entry;
        }

        std::string write_error;
        if (!write_project_lockfile(project_lock_path, resolved_lock, write_error)) {
            std::cerr << "Error: " << write_error << std::endl;
            return 1;
        }
        std::cout << "Lockfile written to " << project_lock_path << " (version " << kLockfileVersion
                  << ", " << resolved_lock.packages.size() << " package(s))" << std::endl;
        return 0;
    } else {
        std::cout << "Unknown command '" << cmd << "'. Run 'wyrpkg --help' for usage." << std::endl;
        return 1;
    }

    return 0;
}
