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

if ((Test-Path "C:\Program Files\LLVM\bin") -and ($env:PATH -notlike "*LLVM\bin*")) {
    $env:PATH = "C:\Program Files\LLVM\bin;$env:PATH"
}

Write-Host "Initializing Wyrm global installation (v$WyrmVersion)..." -ForegroundColor Green
Push-Location $ScriptDir
try {
    # Create destination directories first
    # This is required so packages are in place before self-hosted compiler runs
    foreach ($dir in @($InstallRoot, $WyrmcDir, $WyrpkgDir, $PackagesDir)) {
        if (!(Test-Path $dir)) { New-Item -ItemType Directory -Force -Path $dir | Out-Null }
    }

    # Copy Wyrm package runtime files (standard library) to PackagesDir first
    # because the self-hosted compiler relies on these C files at ~/.wyrm/packages/wyrmlang/lib/ during AOT build
    Copy-Item -Path "wyrm\*" -Destination $PackagesDir -Recurse -Force

    # Compile runtime objects
    gcc -std=c11 -O2 -c wyrm\lib\wyrm_core.c  -o wyrm_core.o  -Iwyrm\lib
    gcc -std=c11 -O2 -c wyrm\lib\wyrm_arena.c -o wyrm_arena.o -Iwyrm\lib
    gcc -std=c11 -O2 -c wyrm\lib\wyrm_str.c   -o wyrm_str.o   -Iwyrm\lib
    gcc -std=c11 -O2 -c wyrm\lib\wyrm_ffi.c   -o wyrm_ffi.o   -Iwyrm\lib
    gcc -std=c11 -O2 -c wyrm\lib\stdlib\wyrm_std_json.c -o wyrm_std_json.o -Iwyrm\lib
    gcc -std=c11 -O2 -c wyrm\lib\stdlib\wyrm_std_yaml.c -o wyrm_std_yaml.o -Iwyrm\lib
    gcc -std=c11 -O2 -c wyrm\lib\stdlib\wyrm_std_sdl.c  -o wyrm_std_sdl.o  -Iwyrm\lib
    gcc -std=c11 -O2 -c wyrm\lib\stdlib\wyrm_std_collections.c -o wyrm_std_collections.o -Iwyrm\lib

    # Compile temporary C++ bootstrap compiler
    g++ wyrm\scr\wyrmc.cpp `
        compiler\lexer\lexer.cpp `
        compiler\parser\parser.cpp `
        compiler\interpreter\interpreter.cpp `
        compiler\interpreter\builtins.cpp `
        compiler\interpreter\stdlib_setup.cpp `
        compiler\transpiler\transpiler.cpp `
        wyrm_core.o wyrm_arena.o wyrm_str.o `
        wyrm_ffi.o wyrm_std_json.o wyrm_std_yaml.o `
        wyrm_std_sdl.o wyrm_std_collections.o `
        -o wyrmc_bootstrap.exe -std=c++20 -O2

    # Ensure no lingering compiler instances lock the binary
    Get-Process wyrmc, wyrmc_bootstrap, wyrmc_stage1, wyrpkg -ErrorAction SilentlyContinue | Stop-Process -Force -ErrorAction SilentlyContinue
    Start-Sleep -Milliseconds 200

    # Clean up any leftover temporary artifacts before Stage 1
    Remove-Item "compiler\wyrmc_temp.ll", "wyrmc_temp.ll" -ErrorAction SilentlyContinue
    Remove-Item "compiler\wyrmc.exe", "wyrmc_stage1.exe", "wyrmc.exe" -ErrorAction SilentlyContinue

    # Stage 1: Build the self-hosted compiler source using the temporary bootstrap compiler
    Write-Host "Self-hosting Stage 1: Building self-hosted compiler using bootstrap compiler..." -ForegroundColor Cyan
    $p1 = Start-Process -FilePath "cmd.exe" -ArgumentList '/c', '.\wyrmc_bootstrap.exe build compiler\wyrmc.wyr' -NoNewWindow -Wait -PassThru
    if ($p1.ExitCode -ne 0 -or !(Test-Path "compiler\wyrmc.exe")) {
        throw "Failed to compile Stage 1 self-hosted compiler (ExitCode: $($p1.ExitCode))."
    }
    Copy-Item "compiler\wyrmc.exe" -Destination "wyrmc_stage1.exe" -Force
    Remove-Item "compiler\wyrmc.exe" -ErrorAction SilentlyContinue
    Remove-Item "compiler\wyrmc_temp.ll" -ErrorAction SilentlyContinue

    # Stage 2: Rebuild the self-hosted compiler using the Stage 1 compiler to achieve true self-hosting
    Write-Host "Self-hosting Stage 2: Rebuilding self-hosted compiler using Stage 1 compiler..." -ForegroundColor Cyan
    $p2 = Start-Process -FilePath "cmd.exe" -ArgumentList '/c', '.\wyrmc_stage1.exe build compiler\wyrmc.wyr -o wyrmc.exe' -NoNewWindow -Wait -PassThru
    if ($p2.ExitCode -ne 0 -or !(Test-Path "wyrmc.exe")) {
        throw "Failed to compile Stage 2 self-hosted compiler (ExitCode: $($p2.ExitCode))."
    }
    Remove-Item "compiler\wyrmc_temp.ll" -ErrorAction SilentlyContinue

    # Compile wyrpkg -> .wyrm\wyrpkg\wyrpkg.exe
    g++ wyrm\scr\wyrpkg.cpp -o wyrpkg.exe -std=c++20 -O2

    # Compile bootstrap helper
    gcc wyrm\scr\bootstrap.c -o bootstrap.exe -std=c11 -O2

    # Clean up temporary object files and bootstrap executables
    Remove-Item wyrm_core.o, wyrm_arena.o, wyrm_str.o, wyrm_ffi.o, wyrm_std_json.o, wyrm_std_yaml.o, wyrm_std_sdl.o, wyrm_std_collections.o -ErrorAction SilentlyContinue
    Remove-Item wyrmc_bootstrap.exe, wyrmc_stage1.exe, compiler\wyrmc.exe -ErrorAction SilentlyContinue

    if (!(Test-Path "wyrmc.exe") -or !(Test-Path "wyrpkg.exe") -or !(Test-Path "bootstrap.exe")) {
        throw "Failed to compile native Wyrm tools. Please ensure gcc/g++ are in PATH."
    }

    # Install binaries into their subdirectories
    Copy-Item "wyrmc.exe"    -Destination (Join-Path $WyrmcDir  "wyrmc.exe")  -Force
    Copy-Item "wyrpkg.exe"   -Destination (Join-Path $WyrpkgDir "wyrpkg.exe") -Force
    Copy-Item "bootstrap.exe" -Destination (Join-Path $InstallRoot "bootstrap.exe") -Force

    Write-Host "  wyrmc  -> $WyrmcDir\wyrmc.exe"  -ForegroundColor Cyan
    Write-Host "  wyrpkg -> $WyrpkgDir\wyrpkg.exe" -ForegroundColor Cyan
} finally {
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