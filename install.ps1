# Wyrm Language Global Installer for Windows
# Installs native C11 Wyrm tools.

$ErrorActionPreference = "Stop"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$WyrmVersion = (Get-Content (Join-Path $ScriptDir "VERSION") -ErrorAction SilentlyContinue).Trim()
if (-not $WyrmVersion) { $WyrmVersion = "unknown" }

$InstallDir = Join-Path $HOME ".wyrm"
$PackagesDir = Join-Path $InstallDir "packages\wyrmlang"

Write-Host "Initializing Wyrm global installation..." -ForegroundColor Green
Push-Location $ScriptDir
try {
    # Compile bootstrap, wyrmc, and wyrpkg from new locations
    gcc -std=c11 -O2 -c wyrm\lib\wyrm_core.c -o wyrm_core.o -Iwyrm\lib
    gcc -std=c11 -O2 -c wyrm\lib\wyrm_arena.c -o wyrm_arena.o -Iwyrm\lib
    gcc -std=c11 -O2 -c wyrm\lib\wyrm_str.c -o wyrm_str.o -Iwyrm\lib
    g++ wyrm\scr\wyrmc.cpp compiler\lexer\lexer.cpp compiler\parser\parser.cpp compiler\interpreter\interpreter.cpp compiler\interpreter\builtins.cpp compiler\transpiler\transpiler.cpp wyrm_core.o wyrm_arena.o wyrm_str.o -o wyrmc.exe -std=c++20 -O2 "-DWYRMC_VERSION=`"$WyrmVersion`""
    Remove-Item wyrm_core.o, wyrm_arena.o, wyrm_str.o -ErrorAction SilentlyContinue
    g++ wyrm\scr\wyrpkg.cpp -o wyrpkg.exe -std=c++20 -O2 "-DWYRPKG_VERSION=`"$WyrmVersion`""
    gcc wyrm\scr\bootstrap.c -o bootstrap.exe -std=c11 -O2

    if (!(Test-Path "wyrmc.exe") -or !(Test-Path "wyrpkg.exe") -or !(Test-Path "bootstrap.exe")) {
        throw "Failed to compile native Wyrm tools. Please ensure gcc is installed and available in PATH."
    }

    if (!(Test-Path $InstallDir)) { New-Item -ItemType Directory -Force -Path $InstallDir | Out-Null }
    if (!(Test-Path $PackagesDir)) { New-Item -ItemType Directory -Force -Path $PackagesDir | Out-Null }

    Copy-Item "wyrmc.exe" -Destination (Join-Path $InstallDir "wyrmc.exe") -Force
    Copy-Item "wyrpkg.exe" -Destination (Join-Path $InstallDir "wyrpkg.exe") -Force
    Copy-Item "bootstrap.exe" -Destination (Join-Path $InstallDir "bootstrap.exe") -Force

    # Copy wyrm package runtime files (excluding executables/subdirectories if not needed, but recurse is fine)
    Copy-Item -Path "wyrm\*" -Destination $PackagesDir -Recurse -Force
} finally {
    Pop-Location
}

$UserPath = [Environment]::GetEnvironmentVariable("PATH", "User")
if ($UserPath -notlike "*$InstallDir*") {
    [Environment]::SetEnvironmentVariable("PATH", "$UserPath;$InstallDir", "User")
    Write-Host "Added $InstallDir to User PATH." -ForegroundColor Green
} else {
    Write-Host "$InstallDir is already in User PATH." -ForegroundColor Yellow
}
$env:PATH += ";$InstallDir"

Write-Host "Wyrm v$WyrmVersion tools and package runtime installed globally." -ForegroundColor Green
Write-Host "Open a NEW terminal and run 'wyrmc --version' or 'wyrpkg --version'." -ForegroundColor Cyan