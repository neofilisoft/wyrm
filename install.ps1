# Wyrm Language Global Installer for Windows
# Installs native C11 Wyrm tools into the new subdirectory layout:
#   $HOME\.wyrm\wyrmc\wyrmc.exe
#   $HOME\.wyrm\wyrpkg\wyrpkg.exe

$ErrorActionPreference = "Stop"
$ScriptDir = if ($PSScriptRoot) { $PSScriptRoot } else { (Get-Location).Path }
$WyrmVersion = (Get-Content (Join-Path $ScriptDir "VERSION") -ErrorAction SilentlyContinue).Trim()
if (-not $WyrmVersion) { $WyrmVersion = "unknown" }

$InstallRoot  = Join-Path $HOME ".wyrm"
$WyrmcDir     = Join-Path $InstallRoot "wyrmc"
$WyrpkgDir    = Join-Path $InstallRoot "wyrpkg"
$PackagesDir  = Join-Path $InstallRoot "packages\wyrmlang"
$LibraryDir   = Join-Path $InstallRoot "library"

# Only processes launched by THIS installer may ever be stopped. Name-based
# process killing is never used because it would kill unrelated user compilers.
$InstallerOwnedProcesses = [System.Collections.Generic.List[System.Diagnostics.Process]]::new()

function Stop-InstallerOwnedProcesses {
    foreach ($proc in $InstallerOwnedProcesses) {
        try {
            if ($proc -and -not $proc.HasExited) {
                Stop-Process -Id $proc.Id -Force -ErrorAction Stop
            }
        } catch {
            Write-Host "Warning: could not stop installer-launched process: $_" -ForegroundColor Yellow
        }
    }
}

function Remove-InstallerArtifact {
    param([string[]]$Artifacts)
    foreach ($artifact in $Artifacts) {
        if (Test-Path $artifact) {
            try {
                Remove-Item $artifact -Force -ErrorAction Stop
            } catch {
                Write-Host "Error: cannot replace '$artifact' because it is currently in use." -ForegroundColor Red
                Write-Host "Close any running Wyrm compiler or package-manager process that holds this file, then re-run the installer." -ForegroundColor Red
                throw
            }
        }
    }
}

function Copy-InstallerBinary {
    param([string]$Source, [string]$Destination)
    try {
        Copy-Item $Source -Destination $Destination -Force -ErrorAction Stop
    } catch {
        Write-Host "Error: cannot install '$Destination' because it is currently in use." -ForegroundColor Red
        Write-Host "Close any running Wyrm compiler or package-manager process, then re-run the installer." -ForegroundColor Red
        throw
    }
}

if ((Test-Path "C:\Program Files\LLVM\bin") -and ($env:PATH -notlike "*LLVM\bin*")) {
    $env:PATH = "C:\Program Files\LLVM\bin;$env:PATH"
}

# The LLVM self-hosting stage compiles generated IR with clang. gcc/g++ can build
# the bootstrap compiler but cannot replace clang for that stage.
if (-not (Get-Command clang -ErrorAction SilentlyContinue)) {
    Write-Host "Error: clang was not found in PATH." -ForegroundColor Red
    Write-Host "The LLVM self-hosting stage requires clang to compile generated LLVM IR."
    Write-Host "gcc/g++ alone are not sufficient. Install LLVM (https://llvm.org) and ensure"
    Write-Host "clang.exe is on PATH, then re-run this installer."
    exit 1
}
Write-Host "LLVM stage compiler: clang" -ForegroundColor Cyan

Write-Host "Initializing Wyrm global installation (v$WyrmVersion)..." -ForegroundColor Green
Push-Location $ScriptDir
try {
    # Create destination directories first
    # This is required so packages are in place before self-hosted compiler runs
    foreach ($dir in @($InstallRoot, $WyrmcDir, $WyrpkgDir, $PackagesDir, $LibraryDir)) {
        if (!(Test-Path $dir)) { New-Item -ItemType Directory -Force -Path $dir | Out-Null }
    }

    # Copy Wyrm package runtime files (standard library) to PackagesDir first
    # because the self-hosted compiler relies on these C files at ~/.wyrm/packages/wyrmlang/lib/ during AOT build
    Copy-Item -Path "wyrm\*" -Destination $PackagesDir -Recurse -Force

    # Copy third-party libraries (e.g. glsl) to LibraryDir if available
    if (Test-Path "library") {
        Copy-Item -Path "library\*" -Destination $LibraryDir -Recurse -Force
    }

    # Compile runtime objects
    gcc -std=c11 -O2 -c wyrm\lib\wyrm_core.c  -o wyrm_core.o  -Iwyrm\lib
    gcc -std=c11 -O2 -c wyrm\lib\wyrm_arena.c -o wyrm_arena.o -Iwyrm\lib
    gcc -std=c11 -O2 -c wyrm\lib\wyrm_str.c   -o wyrm_str.o   -Iwyrm\lib
    gcc -std=c11 -O2 -c wyrm\lib\wyrm_ffi.c   -o wyrm_ffi.o   -Iwyrm\lib
    gcc -std=c11 -O2 -c wyrm\lib\stdlib\wyrm_std_json.c -o wyrm_std_json.o -Iwyrm\lib
    gcc -std=c11 -O2 -c wyrm\lib\stdlib\wyrm_std_yaml.c -o wyrm_std_yaml.o -Iwyrm\lib
    gcc -std=c11 -O2 -c wyrm\lib\stdlib\wyrm_std_collections.c -o wyrm_std_collections.o -Iwyrm\lib
    gcc -std=c11 -O2 -c wyrm\lib\stdlib\wyrm_std_random.c -o wyrm_std_random.o -Iwyrm\lib
    gcc -std=c11 -O2 -c wyrm\lib\stdlib\wyrm_std_time.c   -o wyrm_std_time.o   -Iwyrm\lib
    gcc -std=c11 -O2 -c wyrm\lib\stdlib\wyrm_std_os.c     -o wyrm_std_os.o     -Iwyrm\lib
    gcc -std=c11 -O2 -c wyrm\lib\stdlib\wyrm_std_fs.c     -o wyrm_std_fs.o     -Iwyrm\lib
    gcc -std=c11 -O2 -c wyrm\lib\stdlib\wyrm_std_path.c   -o wyrm_std_path.o   -Iwyrm\lib
    gcc -std=c11 -O2 -c wyrm\lib\stdlib\wyrm_std_io.c     -o wyrm_std_io.o     -Iwyrm\lib
    # HTTP uses a graceful no-curl fallback unless libcurl is available; select it
    # to keep the installer dependency-free (matches the project's test build).
    gcc -std=c11 -O2 -DWYRM_NO_CURL -c wyrm\lib\stdlib\wyrm_std_http.c -o wyrm_std_http.o -Iwyrm\lib

    # Compile temporary C++ bootstrap compiler
    g++ wyrm\src\wyrmc.cpp `
        compiler\lexer\lexer.cpp `
        compiler\parser\parser.cpp `
        compiler\interpreter\interpreter.cpp `
        compiler\interpreter\builtins.cpp `
        compiler\interpreter\stdlib_setup.cpp `
        compiler\transpiler\transpiler.cpp `
        wyrm_core.o wyrm_arena.o wyrm_str.o `
        wyrm_ffi.o wyrm_std_json.o wyrm_std_yaml.o `
        wyrm_std_collections.o wyrm_std_random.o wyrm_std_time.o `
        wyrm_std_os.o wyrm_std_fs.o wyrm_std_path.o `
        wyrm_std_io.o wyrm_std_http.o `
        -o wyrmc_bootstrap.exe -std=c++20 -O2

    # Clean up any leftover temporary artifacts before Stage 1. If a file is
    # locked by another program, report it instead of killing unrelated processes.
    Remove-Item "compiler\wyrmc_temp.ll", "wyrmc_temp.ll" -ErrorAction SilentlyContinue
    Remove-InstallerArtifact @("compiler\wyrmc.exe", "wyrmc_stage1.exe", "wyrmc.exe")

    # Stage 1: Build the self-hosted compiler source using the temporary bootstrap compiler
    Write-Host "Self-hosting Stage 1: Building self-hosted compiler using bootstrap compiler..." -ForegroundColor Cyan
    $p1 = Start-Process -FilePath "cmd.exe" -ArgumentList '/c', '.\wyrmc_bootstrap.exe build compiler\wyrmc.wyr' -NoNewWindow -Wait -PassThru
    $InstallerOwnedProcesses.Add($p1)
    if ($p1.ExitCode -ne 0 -or !(Test-Path "compiler\wyrmc.exe")) {
        throw "Failed to compile Stage 1 self-hosted compiler (ExitCode: $($p1.ExitCode))."
    }
    Copy-Item "compiler\wyrmc.exe" -Destination "wyrmc_stage1.exe" -Force
    Remove-Item "compiler\wyrmc.exe" -ErrorAction SilentlyContinue
    Remove-Item "compiler\wyrmc_temp.ll" -ErrorAction SilentlyContinue

    # Stage 2: Rebuild the self-hosted compiler using the Stage 1 compiler to achieve true self-hosting
    Write-Host "Self-hosting Stage 2: Rebuilding self-hosted compiler using Stage 1 compiler..." -ForegroundColor Cyan
    $p2 = Start-Process -FilePath "cmd.exe" -ArgumentList '/c', '.\wyrmc_stage1.exe build compiler\wyrmc.wyr -o wyrmc.exe' -NoNewWindow -Wait -PassThru
    $InstallerOwnedProcesses.Add($p2)
    if ($p2.ExitCode -ne 0 -or !(Test-Path "wyrmc.exe")) {
        throw "Failed to compile Stage 2 self-hosted compiler (ExitCode: $($p2.ExitCode))."
    }
    Remove-Item "compiler\wyrmc_temp.ll" -ErrorAction SilentlyContinue

    # Compile wyrpkg -> .wyrm\wyrpkg\wyrpkg.exe
    g++ wyrm\src\wyrpkg.cpp -o wyrpkg.exe -std=c++20 -O2

    # Compile bootstrap helper
    gcc wyrm\src\bootstrap.c -o bootstrap.exe -std=c11 -O2

    # Clean up temporary object files and bootstrap executables
    Remove-Item wyrm_core.o, wyrm_arena.o, wyrm_str.o, wyrm_ffi.o, wyrm_std_json.o, wyrm_std_yaml.o, wyrm_std_collections.o, wyrm_std_random.o, wyrm_std_time.o, wyrm_std_os.o, wyrm_std_fs.o, wyrm_std_path.o, wyrm_std_io.o, wyrm_std_http.o -ErrorAction SilentlyContinue
    Remove-InstallerArtifact @("wyrmc_bootstrap.exe", "wyrmc_stage1.exe", "compiler\wyrmc.exe")

    if (!(Test-Path "wyrmc.exe") -or !(Test-Path "wyrpkg.exe") -or !(Test-Path "bootstrap.exe")) {
        throw "Failed to compile native Wyrm tools. Please ensure gcc/g++ are in PATH."
    }

    # Install binaries into their subdirectories
    Copy-InstallerBinary "wyrmc.exe"     (Join-Path $WyrmcDir    "wyrmc.exe")
    Copy-InstallerBinary "wyrpkg.exe"    (Join-Path $WyrpkgDir   "wyrpkg.exe")
    Copy-InstallerBinary "bootstrap.exe" (Join-Path $InstallRoot "bootstrap.exe")

    Write-Host "  wyrmc  -> $WyrmcDir\wyrmc.exe"  -ForegroundColor Cyan
    Write-Host "  wyrpkg -> $WyrpkgDir\wyrpkg.exe" -ForegroundColor Cyan

    # Install/update IDE syntax extensions (VS Code, Antigravity IDE, Cursor)
    $extDirs = @(
        (Join-Path $env:USERPROFILE ".vscode\extensions"),
        (Join-Path $env:USERPROFILE ".antigravity-ide\extensions"),
        (Join-Path $env:USERPROFILE ".cursor\extensions")
    )
    foreach ($ed in $extDirs) {
        if (Test-Path $ed) {
            Get-ChildItem $ed -Directory -Filter "neofilisoft.wyrm-syntax-*" | Remove-Item -Recurse -Force -ErrorAction SilentlyContinue
            $destExt = Join-Path $ed "neofilisoft.wyrm-syntax-$WyrmVersion"
            Copy-Item "extension" -Destination $destExt -Recurse -Force
            Write-Host "  syntax -> $destExt" -ForegroundColor Cyan
        }
    }

    $vsixPath = Join-Path "extension" "wyrm-syntax-$WyrmVersion.vsix"
    if (Test-Path $vsixPath) {
        if (Get-Command code -ErrorAction SilentlyContinue) {
            try {
                code --install-extension $vsixPath --force | Out-Null
                Write-Host "  Registered in VS Code via code CLI" -ForegroundColor Green
            } catch {}
        }
        if (Get-Command cursor -ErrorAction SilentlyContinue) {
            try {
                cursor --install-extension $vsixPath --force | Out-Null
                Write-Host "  Registered in Cursor via cursor CLI" -ForegroundColor Green
            } catch {}
        }
    }
} finally {
    # Stop only processes this installer launched; never touch unrelated user processes.
    Stop-InstallerOwnedProcesses
    Pop-Location
}

# Add both tool dirs to User PATH
$UserPath = [Environment]::GetEnvironmentVariable("PATH", "User")
foreach ($dir in @($WyrmcDir, $WyrpkgDir)) {
    if ($UserPath -notlike "*$dir*") {
        $UserPath = "$UserPath;$dir"
        Write-Host "Added $dir to User PATH." -ForegroundColor Green
    } else {
        Write-Host "$dir is already in User PATH." -ForegroundColor Yellow
    }
}
[Environment]::SetEnvironmentVariable("PATH", $UserPath, "User")
$env:PATH += ";$WyrmcDir;$WyrpkgDir"

Write-Host ""
Write-Host "Wyrm v$WyrmVersion installed successfully." -ForegroundColor Green
Write-Host "Open a NEW terminal and run 'wyrmc --version' or 'wyrpkg --version'." -ForegroundColor Cyan