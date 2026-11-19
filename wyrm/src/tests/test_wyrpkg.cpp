#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#ifndef _WIN32
#include <unistd.h>
#endif

#define main wyrpkg_main
#include "../wyrpkg.cpp"
#undef main

namespace fs = std::filesystem;
namespace {

int g_checks = 0;
int g_failures = 0;

#define CHECK(condition)                                                            \
    do {                                                                            \
        ++g_checks;                                                                 \
        if (!(condition)) {                                                         \
            ++g_failures;                                                           \
            std::cerr << "FAIL " << __FILE__ << ":" << __LINE__                     \
                      << ": " << #condition << std::endl;                           \
        }                                                                           \
    } while (false)

std::string read_file(const fs::path& path) {
    std::ifstream file(path, std::ios::binary);
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void write_file(const fs::path& path, const std::string& content) {
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    file << content;
}

size_t count_occurrences(const std::string& haystack, const std::string& needle) {
    size_t count = 0;
    size_t position = 0;
    while ((position = haystack.find(needle, position)) != std::string::npos) {
        ++count;
        position += needle.size();
    }
    return count;
}

class ScratchDir {
public:
    explicit ScratchDir(const std::string& label) {
        static unsigned long counter = 0;
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        path_ = fs::temp_directory_path() /
                ("wyrm-wyrpkg-" + label + "-" + std::to_string(stamp) + "-" + std::to_string(++counter));
        fs::create_directories(path_);
    }
    ~ScratchDir() {
        std::error_code ignored;
        fs::remove_all(path_, ignored);
    }
    ScratchDir(const ScratchDir&) = delete;
    ScratchDir& operator=(const ScratchDir&) = delete;
    const fs::path& path() const { return path_; }

private:
    fs::path path_;
};

class CwdGuard {
public:
    CwdGuard() : original_(fs::current_path()) {}
    ~CwdGuard() { fs::current_path(original_); }
    CwdGuard(const CwdGuard&) = delete;
    CwdGuard& operator=(const CwdGuard&) = delete;

private:
    fs::path original_;
};

class EnvGuard {
public:
    explicit EnvGuard(const char* name) : name_(name) {
        const char* value = std::getenv(name_.c_str());
        if (value != nullptr) {
            had_value_ = true;
            value_ = value;
        }
    }
    ~EnvGuard() { assign(had_value_, value_); }
    EnvGuard(const EnvGuard&) = delete;
    EnvGuard& operator=(const EnvGuard&) = delete;

    void set_value(const fs::path& value) { assign(true, value.string()); }

private:
    void assign(bool present, const std::string& value) {
#ifdef _WIN32
        _putenv_s(name_.c_str(), present ? value.c_str() : "");
#else
        if (present) {
            ::setenv(name_.c_str(), value.c_str(), 1);
        } else {
            ::unsetenv(name_.c_str());
        }
#endif
    }

    std::string name_;
    std::string value_;
    bool had_value_ = false;
};

int run_cli(std::vector<std::string> args) {
    std::vector<char*> raw_args;
    raw_args.reserve(args.size() + 1);
    for (std::string& arg : args) raw_args.push_back(arg.data());
    raw_args.push_back(nullptr);
    return wyrpkg_main(static_cast<int>(args.size()), raw_args.data());
}

bool run_shell(const std::string& command) {
    return std::system(command.c_str()) == 0;
}

std::string shell_capture(const std::string& command) {
    std::string output;
#ifdef _WIN32
    FILE* pipe = _popen(command.c_str(), "r");
#else
    FILE* pipe = popen(command.c_str(), "r");
#endif
    if (pipe == nullptr) return output;
    char buffer[256];
    while (std::fgets(buffer, sizeof(buffer), pipe) != nullptr) output += buffer;
#ifdef _WIN32
    _pclose(pipe);
#else
    pclose(pipe);
#endif
    return output;
}

struct GitFixture {
    GitFixture(const fs::path& root, const std::string& name)
        : bare(root / (name + ".git")), work(root / name) {
        fs::create_directories(work);
        CHECK(run_shell("git init --bare -q \"" + bare.string() + "\" > /dev/null 2>&1"));
        CHECK(run_shell("git -C \"" + work.string() + "\" init -q"));
        CHECK(run_shell("git -C \"" + work.string() + "\" config user.email test@example.invalid"));
        CHECK(run_shell("git -C \"" + work.string() + "\" config user.name WyrmTest"));
        write_file(work / "README.md", "first\n");
        CHECK(run_shell("git -C \"" + work.string() + "\" add README.md"));
        CHECK(run_shell("git -C \"" + work.string() + "\" commit -q -m first"));
        CHECK(run_shell("git -C \"" + work.string() + "\" remote add origin \"" + bare.string() + "\""));
        CHECK(run_shell("git -C \"" + work.string() + "\" push -q -u origin HEAD"));
        first_revision = trim_str(shell_capture("git -C \"" + work.string() + "\" rev-parse HEAD"));
        CHECK(is_hex_revision(first_revision));
    }

    void advance() {
        write_file(work / "README.md", "second\n");
        CHECK(run_shell("git -C \"" + work.string() + "\" commit -q -am second"));
        CHECK(run_shell("git -C \"" + work.string() + "\" push -q origin HEAD"));
        second_revision = trim_str(shell_capture("git -C \"" + work.string() + "\" rev-parse HEAD"));
        CHECK(is_hex_revision(second_revision));
        CHECK(second_revision != first_revision);
    }

    fs::path bare;
    fs::path work;
    std::string first_revision;
    std::string second_revision;
};

void test_add_dispatch_and_manifest() {
    ScratchDir project("add");
    CwdGuard cwd_guard;
    fs::current_path(project.path());

    CHECK(run_cli({"wyrpkg", "add", "alpha", "1.2.3"}) == 0);
    std::string toml = read_file("wyrpkg.toml");
    CHECK(toml.find("[dependencies]") != std::string::npos);
    CHECK(toml.find("alpha = \"1.2.3\"") != std::string::npos);
    CHECK(toml.find("name = \"" + project.path().filename().string() + "\"") != std::string::npos);

    CHECK(run_cli({"wyrpkg", "add", "beta"}) == 0);
    toml = read_file("wyrpkg.toml");
    CHECK(toml.find("beta = \"latest\"") != std::string::npos);

    CHECK(run_cli({"wyrpkg", "add", "alpha", "9.9.9"}) == 0);
    toml = read_file("wyrpkg.toml");
    CHECK(count_occurrences(toml, "alpha =") == 1);
    CHECK(toml.find("alpha = \"9.9.9\"") == std::string::npos);
}

void test_argument_validation_and_unknown_command() {
    ScratchDir project("validate");
    CwdGuard cwd_guard;
    fs::current_path(project.path());

    CHECK(run_cli({"wyrpkg", "add"}) == 1);
    CHECK(!fs::exists("wyrpkg.toml"));
    CHECK(run_cli({"wyrpkg", "lock"}) == 1);
    CHECK(!fs::exists("wyrpkg.lock"));
    CHECK(run_cli({"wyrpkg", "unknown-command"}) == 1);
}

void test_lock_requires_installed_dependencies() {
    ScratchDir project("unpinned");
    CwdGuard cwd_guard;
    fs::current_path(project.path());
    write_toml_manifest("wyrpkg.toml", "demo", "0.1.0", "main.wyr");
    CHECK(run_cli({"wyrpkg", "add", "ghost", "1.0.0"}) == 0);
    CHECK(run_cli({"wyrpkg", "lock"}) == 1);
    CHECK(!fs::exists("wyrpkg.lock"));

    ScratchDir empty_project("emptylock");
    fs::current_path(empty_project.path());
    write_toml_manifest("wyrpkg.toml", "empty", "0.1.0", "main.wyr");
    CHECK(run_cli({"wyrpkg", "lock"}) == 0);
    auto parsed = read_project_lockfile("wyrpkg.lock");
    CHECK(parsed.ok);
    CHECK(parsed.lockfile.version == kLockfileVersion);
    CHECK(parsed.lockfile.packages.empty());
}

void test_help_lists_new_commands() {
    std::ostringstream captured;
    std::streambuf* previous = std::cout.rdbuf(captured.rdbuf());
    show_help();
    std::cout.rdbuf(previous);
    CHECK(captured.str().find("add <package> [version]") != std::string::npos);
    CHECK(captured.str().find("lock ") != std::string::npos);
}

void test_lockfile_roundtrip_and_stable_serialization() {
    WyrmLockfile lockfile;
    lockfile.packages["zeta"] = LockedPackage{"https://example.invalid/zeta.git", std::string(40, 'a'), "1.0.0"};
    lockfile.packages["alpha"] = LockedPackage{"https://example.invalid/alpha.git", std::string(40, 'b'), "2.0.0"};

    const std::string serialized = serialize_lockfile(lockfile);
    CHECK(serialized.find("\"version\": 1") != std::string::npos);
    CHECK(serialized.find("\"alpha\"") < serialized.find("\"zeta\""));

    auto parsed = LockfileParser(serialized).parse();
    CHECK(parsed.ok);
    CHECK(parsed.error.empty());
    CHECK(parsed.lockfile.version == kLockfileVersion);
    CHECK(parsed.lockfile.packages.size() == 2);
    CHECK(parsed.lockfile.packages.at("alpha").revision == std::string(40, 'b'));
    CHECK(serialize_lockfile(parsed.lockfile) == serialized);
}

void test_lockfile_rejects_malformed_input() {
    const std::string revision(40, 'a');
    const std::string valid_local =
        "{\"version\": 1, \"packages\": {\"local\": {\"source\": \"/opt/pkg\", "
        "\"revision\": \"\", \"version\": \"1.0.0\"}}}";
    CHECK(LockfileParser(valid_local).parse().ok);

    const std::vector<std::string> malformed = {
        "",
        "{",
        "{}",
        "{\"version\": 2, \"packages\": {}}",
        "{\"version\": 1}",
        "{\"version\": 1, \"packages\": {}, \"extra\": 3}",
        "{\"version\": -1, \"packages\": {}}",
        "{\"version\": 1, \"version\": 1, \"packages\": {}}",
        "{\"version\": 1, \"packages\": {\"a\": {\"source\": \"https://example.invalid/a.git\", "
        "\"revision\": \"\", \"version\": \"1.0.0\"}}}",
        "{\"version\": 1, \"packages\": {\"a\": {\"source\": \"https://example.invalid/a.git\", "
        "\"revision\": \"zzzz\", \"version\": \"1.0.0\"}}}",
        "{\"version\": 1, \"packages\": {\"a\": {\"source\": \"" + revision + "\", "
        "\"revision\": \"\", \"version\": \"1.0.0\", \"bogus\": \"x\"}}}",
        "{\"version\": 1, \"packages\": {}} trailing",
    };
    for (const std::string& content : malformed) {
        auto parsed = LockfileParser(content).parse();
        CHECK(!parsed.ok);
        CHECK(!parsed.error.empty());
    }
}

void test_git_head_resolution() {
    ScratchDir root("githead");
    GitFixture fixture(root.path(), "dep");
    const fs::path clone = root.path() / "clone";
    CHECK(run_shell("git clone -q \"" + fixture.bare.string() + "\" \"" + clone.string() + "\""));

    auto head = read_git_head(clone);
    CHECK(head.ok);
    CHECK(head.revision == fixture.first_revision);

    CHECK(run_shell("git -C \"" + clone.string() + "\" pack-refs --all"));
    head = read_git_head(clone);
    CHECK(head.ok);
    CHECK(head.revision == fixture.first_revision);

    CHECK(run_shell("git -C \"" + clone.string() + "\" checkout -q --detach HEAD"));
    head = read_git_head(clone);
    CHECK(head.ok);
    CHECK(head.revision == fixture.first_revision);

    auto missing = read_git_head(root.path());
    CHECK(!missing.ok);
    CHECK(!missing.error.empty());
}

void test_revision_pin_survives_remote_advance() {
    ScratchDir root("pin");
    GitFixture fixture(root.path(), "remote_pkg");
    const fs::path first_clone = root.path() / "first";
    const fs::path second_clone = root.path() / "second";
    CHECK(run_shell("git clone -q \"" + fixture.bare.string() + "\" \"" + first_clone.string() + "\""));

    ScratchDir home("pinhome");
    EnvGuard user_profile("USERPROFILE");
    user_profile.set_value(home.path());
    CwdGuard cwd_guard;
    fs::current_path(root.path());
    write_toml_manifest("wyrpkg.toml", "demo", "0.1.0", "main.wyr");

    const std::string source = "https://example.invalid/remote_pkg.git";
    auto first = prepare_install_lock("remote_pkg", source, first_clone, "1.0.0");
    CHECK(first.ok);
    CHECK(first.write_lock);
    CHECK(first.lockfile.packages.at("remote_pkg").revision == fixture.first_revision);
    std::string write_error;
    CHECK(write_project_lockfile("wyrpkg.lock", first.lockfile, write_error));
    CHECK(write_error.empty());

    fixture.advance();
    CHECK(run_shell("git clone -q \"" + fixture.bare.string() + "\" \"" + second_clone.string() + "\""));
    auto drifted = prepare_install_lock("remote_pkg", source, second_clone, "1.0.0");
    CHECK(!drifted.ok);
    CHECK(drifted.error.find("drifting dependency") != std::string::npos);

    auto replay = prepare_install_lock("remote_pkg", source, first_clone, "1.0.0");
    CHECK(replay.ok);
    CHECK(replay.lockfile.packages.at("remote_pkg").revision == fixture.first_revision);
}

void test_install_and_lock_cli_integration() {
    ScratchDir root("install");
    GitFixture fixture(root.path(), "remote_pkg");
    write_file(fixture.work / "wyrpkg.toml",
               "[package]\nname = \"remote_pkg\"\nversion = \"3.4.5\"\nentry = \"main.wyr\"\n");

    ScratchDir home("installhome");
    EnvGuard user_profile("USERPROFILE");
    user_profile.set_value(home.path());

    const fs::path project = root.path() / "project";
    fs::create_directories(project);
    CwdGuard cwd_guard;
    fs::current_path(project);
    CHECK(run_cli({"wyrpkg", "add", "remote_pkg", "1.0.0"}) == 0);

    CHECK(run_cli({"wyrpkg", "install", fixture.work.string()}) == 0);
    const fs::path installed = home.path() / ".wyrm" / "packages" / "remote_pkg";
    CHECK(fs::exists(installed));

    auto recorded = read_project_lockfile("wyrpkg.lock");
    CHECK(recorded.ok);
    CHECK(recorded.lockfile.version == kLockfileVersion);
    CHECK(recorded.lockfile.packages.count("remote_pkg") == 1);
    const LockedPackage& installed_entry = recorded.lockfile.packages.at("remote_pkg");
    CHECK(installed_entry.revision == fixture.first_revision);
    CHECK(installed_entry.version == "3.4.5");
    CHECK(installed_entry.source == fs::absolute(fixture.work).string());

    CHECK(run_cli({"wyrpkg", "lock"}) == 0);
    const std::string first_lock = read_file("wyrpkg.lock");
    auto locked = read_project_lockfile("wyrpkg.lock");
    CHECK(locked.ok);
    CHECK(locked.lockfile.packages.at("remote_pkg").revision == fixture.first_revision);
    CHECK(locked.lockfile.packages.at("remote_pkg").version == "3.4.5");
    CHECK(run_cli({"wyrpkg", "lock"}) == 0);
    CHECK(read_file("wyrpkg.lock") == first_lock);

    // A corrupted project lockfile must block both lock and install.
    write_file("wyrpkg.lock", "not-json");
    CHECK(run_cli({"wyrpkg", "lock"}) == 1);
    CHECK(read_file("wyrpkg.lock") == "not-json");

    WyrmLockfile drifted;
    drifted.packages["remote_pkg"] = LockedPackage{
        installed_entry.source, std::string(40, 'f'), installed_entry.version};
    std::string ignored_error;
    CHECK(write_project_lockfile("wyrpkg.lock", drifted, ignored_error));
    CHECK(run_cli({"wyrpkg", "lock"}) == 1);

    // A remote branch that advances must be rejected against the pinned revision.
    CHECK(write_project_lockfile("wyrpkg.lock", recorded.lockfile, ignored_error));
    fixture.advance();
    fs::remove_all(installed);
    CHECK(run_cli({"wyrpkg", "install", fixture.work.string()}) == 1);
    CHECK(!fs::exists(installed));
}

void test_install_without_project_context() {
    ScratchDir root("noproject");
    GitFixture fixture(root.path(), "dep");
    ScratchDir home("plainhome");
    EnvGuard user_profile("USERPROFILE");
    user_profile.set_value(home.path());
    CwdGuard cwd_guard;
    fs::create_directories(root.path() / "plain");
    fs::current_path(root.path() / "plain");

    CHECK(run_cli({"wyrpkg", "install", fixture.work.string()}) == 0);
    CHECK(!fs::exists("wyrpkg.lock"));
}

}  // namespace

int main() {
    test_add_dispatch_and_manifest();
    test_argument_validation_and_unknown_command();
    test_lock_requires_installed_dependencies();
    test_help_lists_new_commands();
    test_lockfile_roundtrip_and_stable_serialization();
    test_lockfile_rejects_malformed_input();
    test_git_head_resolution();
    test_revision_pin_survives_remote_advance();
    test_install_and_lock_cli_integration();
    test_install_without_project_context();

    std::cout << "wyrpkg tests: " << (g_checks - g_failures) << "/" << g_checks
              << " checks passed" << std::endl;
    return g_failures == 0 ? 0 : 1;
}
