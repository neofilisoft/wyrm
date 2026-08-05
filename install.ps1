# Wyrm Language Global Installer for Windows
# Installs native C11 Wyrm tools into the new subdirectory layout:
#   $HOME\.wyrm\wyrmc\wyrmc.exe
#   $HOME\.wyrm\wyrpkg\wyrpkg.exe

$ErrorActionPreference = "Stop"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$WyrmVersion = (Get-Content (Join-Path $ScriptDir "VERSION") -ErrorAction SilentlyContinue).Trim()
if (-not $WyrmVersion) { $WyrmVersion = "unknown" }

$InstallRoot  = Join-Path $HOME ".wyrm"
$WyrmcDir     = Join-Path $InstallRoot "wyrmc"
$WyrpkgDir    = Join-Path $InstallRoot "wyrpkg"
$PackagesDir  = Join-Path $InstallRoot "packages\wyrmlang"

Write-Host "Initializing Wyrm global installation (v$WyrmVersion)..." -ForegroundColor Green
Push-Location $ScriptDir
try {
    # Compile runtime objects
    gcc -std=c11 -O2 -c wyrm\lib\wyrm_core.c  -o wyrm_core.o  -Iwyrm\lib
    gcc -std=c11 -O2 -c wyrm\lib\wyrm_arena.c -o wyrm_arena.o -Iwyrm\lib
    gcc -std=c11 -O2 -c wyrm\lib\wyrm_str.c   -o wyrm_str.o   -Iwyrm\lib

    # Compile wyrmc -> .wyrm\wyrmc\wyrmc.exe
    g++ wyrm\scr\wyrmc.cpp `
        compiler\lexer\lexer.cpp `
        compiler\parser\parser.cpp `
        compiler\interpreter\interpreter.cpp `
        compiler\interpreter\builtins.cpp `
        compiler\transpiler\transpiler.cpp `
        wyrm_core.o wyrm_arena.o wyrm_str.o `
        -o wyrmc.exe -std=c++20 -O2 `
        "-DWYRMC_VERSION=`"$WyrmVersion`""

    # Compile wyrpkg -> .wyrm\wyrpkg\wyrpkg.exe
    g++ wyrm\scr\wyrpkg.cpp -o wyrpkg.exe -std=c++20 -O2 `
        "-DWYRPKG_VERSION=`"$WyrmVersion`""

    # Compile bootstrap helper
    gcc wyrm\scr\bootstrap.c -o bootstrap.exe -std=c11 -O2

    Remove-Item wyrm_core.o, wyrm_arena.o, wyrm_str.o -ErrorAction SilentlyContinue

    if (!(Test-Path "wyrmc.exe") -or !(Test-Path "wyrpkg.exe") -or !(Test-Path "bootstrap.exe")) {
        throw "Failed to compile native Wyrm tools. Please ensure gcc/g++ are in PATH."
    }

    # Create destination directories
    foreach ($dir in @($InstallRoot, $WyrmcDir, $WyrpkgDir, $PackagesDir)) {
        if (!(Test-Path $dir)) { New-Item -ItemType Directory -Force -Path $dir | Out-Null }
    }

    # Install binaries into their subdirectories
    Copy-Item "wyrmc.exe"    -Destination (Join-Path $WyrmcDir  "wyrmc.exe")  -Force
    Copy-Item "wyrpkg.exe"   -Destination (Join-Path $WyrpkgDir "wyrpkg.exe") -Force
    Copy-Item "bootstrap.exe" -Destination (Join-Path $InstallRoot "bootstrap.exe") -Force

    # Copy Wyrm package runtime files
    Copy-Item -Path "wyrm\*" -Destination $PackagesDir -Recurse -Force

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
