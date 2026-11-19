# Contributing to Wyrm

Thank you for your interest in Wyrm. Wyrm is a native systems programming language with a self-hosted compiler, a C runtime, a package manager, and a growing standard library.

This guide explains how to report problems, propose changes, build the toolchain, run tests, and prepare a pull request.

## Before you start

Please read these files first:

- [README.md](README.md) for the project overview and commands.
- [docs/Docs.md](docs/Docs.md) for the language specification.
- [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md) for community expectations.
- [.agents/skills/the-7-coding-laws/SKILL.md](.agents/skills/the-7-coding-laws/SKILL.md) for maintainability guidance used by this repository.

Wyrm is still evolving. Language syntax, runtime behavior, compiler internals, and package metadata may change between releases. When reporting a problem, always include the Wyrm version and the commit or release that you tested.

## Repository layout

| Path | Purpose |
| --- | --- |
| `compiler/lexer/` | Native C++ lexer and lexer tests |
| `compiler/parser/` | Native C++ parser and parser tests |
| `compiler/interpreter/` | Interpreter, builtins, runtime setup, and tests |
| `compiler/transpiler/` | Native C or LLVM-oriented code generation |
| `compiler/wyrmc.wyr` | Self-hosted Wyrm compiler source |
| `wyrm/src/` | Bootstrap compiler and package manager entry points |
| `wyrm/lib/` | C runtime and native standard library backends |
| `library/std/` | Pure Wyrm standard packages |
| `examples/` | Runnable language examples and regression programs |
| `extension/` | Wyrm syntax highlighting extension |
| `docs/` | Language and project documentation |
| `dist/` | Local release packages and binaries, not source code |

## Reporting bugs

Before opening an issue:

1. Search existing issues and recent commits.
2. Reproduce the problem with the latest `main` branch or latest release.
3. Reduce the source to the smallest `.wyr` program that still fails.
4. Check whether the failure occurs in the interpreter, native build, self-hosted compiler, or package manager.

A useful bug report includes:

- Wyrm version, operating system, architecture, and compiler toolchain.
- The exact command that was run.
- A minimal `.wyr` reproduction or a small repository link.
- Expected behavior and actual behavior.
- Complete error output, including the first diagnostic.
- Whether the issue is a regression and which version last worked.

Do not include passwords, access tokens, private source code, or personal data in an issue. For a suspected security vulnerability, do not publish an exploit in a public issue. Contact the repository maintainer privately through GitHub first.

## Proposing features

Feature proposals are welcome when they are connected to a real use case. Before implementing a large language or runtime change, open an issue describing:

- The problem and the users affected.
- A short example of the proposed Wyrm syntax or API.
- Compatibility and migration concerns.
- Interpreter and native compiler behavior.
- Runtime, package, documentation, and test impact.

Small bug fixes and documentation corrections can usually be submitted directly as a pull request. Larger syntax, ownership, ABI, or standard library changes should be discussed before implementation.

## Development requirements

The native toolchain requires:

- Git.
- GCC or another compatible C compiler.
- G++ with C++20 support.
- PowerShell on Windows.
- Bash on Linux or macOS.

Optional dependencies may be needed for specific features. Native HTTP support requires libcurl. SDL examples require the relevant SDL development libraries.

## Building Wyrm

On Windows, run:

```powershell
powershell -ExecutionPolicy Bypass -File .\install.ps1
```

On Linux or macOS, run:

```bash
chmod +x install.sh
./install.sh
```

The installer builds `wyrmc`, `wyrpkg`, the runtime objects, and the self-hosted compiler stages. It also installs the runtime package and configures the user `PATH`.

For a local compiler build without installation, compile the native sources as described by the scripts in `compiler/*/tests/` and use `-DWYRM_NO_CURL` when libcurl is not configured.

## Running tests

Run the lexer tests:

```powershell
cd compiler\lexer\tests
cmd /c compile_tests.bat
```

Run the parser tests:

```powershell
cd compiler\parser\tests
cmd /c compile_tests.bat
```

Run the interpreter tests:

```powershell
cd compiler\interpreter\tests
cmd /c compile_tests.bat
```

Run example programs with the compiler:

```text
wyrmc run examples/hello.wyr
wyrmc build examples/hello.wyr -o hello
wyrmc check examples/test_readme_match.wyr
wyrmc test examples
```

When testing a compiler change, test both interpreter execution and native compilation when the feature is supported by both paths. For ownership, reference counting, weak references, arrays, or runtime changes, add a regression test that exercises the cleanup path as well as the successful result.

## Writing Wyrm tests

Place reusable language examples and regression programs under `examples/`. Use a descriptive `test_*.wyr` name when the file is intended to be discovered by `wyrmc test`.

Tests should:

- Be deterministic and non-interactive.
- Avoid network access unless the test explicitly verifies an optional HTTP feature.
- Avoid writing outside a temporary test location.
- Check observable output or return status.
- Reproduce the bug before the fix and fail for the old behavior.
- Cover both valid behavior and useful diagnostics when applicable.

For compiler unit tests, follow the existing C++ test style in `compiler/lexer/tests`, `compiler/parser/tests`, and `compiler/interpreter/tests`. Keep generated executables, object files, temporary C files, and LLVM IR out of commits.

## Code and documentation style

- Keep changes focused on one problem or feature.
- Prefer clear names and small functions.
- Keep decisions separate from filesystem, process, network, and compiler side effects where practical.
- Preserve useful error context and source locations.
- Do not silently change ownership or reference-counting behavior.
- When changing syntax, update the lexer, parser, interpreter, native transpiler, self-hosted compiler, examples, tests, and documentation as applicable.
- When changing the runtime ABI, update declarations, implementations, generated call paths, and every platform build script.
- Avoid unrelated formatting changes, vendored source edits, and generated output.
- Do not use em dashes or en dashes in user-facing project documentation. Use a comma, colon, parentheses, or a regular hyphen instead.

Documentation changes should include runnable examples when they describe syntax or commands. Keep version references consistent with `VERSION`, `wyrm/VERSION`, package metadata, compiler version constants, and release documentation.

## Git and pull requests

Create a topic branch from the current `main` branch:

```bash
git switch main
git pull --rebase origin main
git switch -c codex/short-description
```

Before opening a pull request:

1. Review the complete diff with `git diff`.
2. Run the relevant unit tests and example programs.
3. Run `git diff --check`.
4. Confirm that no generated binaries, object files, temporary C files, LLVM IR, credentials, or local paths are included.
5. Update documentation and release metadata when the public behavior changes.

Keep commits small and logically ordered. A commit should leave the repository buildable and testable. Use a concise imperative subject, preferably 72 characters or fewer, such as:

```text
Fix array ownership when storing borrowed values
Add parser coverage for named struct literals
Update native build instructions for Linux
```

Pull requests should explain:

- What changed.
- Why the change is needed.
- Which files or subsystems are affected.
- How it was tested.
- Any platform, ABI, performance, or compatibility concerns.

Pull requests that change language syntax or runtime behavior should include tests and documentation in the same change whenever possible.

## Generated files and vendored code

Do not commit local build output. The repository ignores common generated files such as `*.exe`, `*.o`, `*.ll`, `_wyrm_temp.c`, and build directories. If an old generated file is already tracked, remove it from Git rather than re-adding it after a local build.

Treat `library/glsl/` as a graphics package and vendored shader toolchain. Avoid editing upstream files unless the pull request is specifically about that integration. Prefer changes in Wyrm wrapper files, package metadata, or documented integration code.

## License

By contributing to Wyrm, you agree that your contribution is made available under the repository license. See [LICENSE](LICENSE) for the full MIT License text.

Thanks for helping make Wyrm more reliable, useful, and easier to learn.
