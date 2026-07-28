# Wyrm Language Global Installer for Windows
# Installs native C11 Wyrm tools. Python is not required for wyrm/wyrmc/wyrpkg.

$ErrorActionPreference = "Stop"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$WyrmVersion = (Get-Content (Join-Path $ScriptDir "VERSION") -ErrorAction SilentlyContinue).Trim()
if (-not $WyrmVersion) { $WyrmVersion = "unknown" }

$InstallDir = Join-Path $HOME ".wyrm"
$BinDir = Join-Path $InstallDir "bin"

Write-Host "Initializing Wyrm native C installation..." -ForegroundColor Green
Push-Location $ScriptDir
try {
    gcc scr\wyrmc.c -o wyrm.exe -std=c11 -O2 -lm
    gcc scr\wyrmc.c -o wyrmc.exe -std=c11 -O2 -lm
    gcc scr\wyrpkg.c -o wyrpkg.exe -std=c11 -O2

    if (!(Test-Path "wyrm.exe") -or !(Test-Path "wyrmc.exe") -or !(Test-Path "wyrpkg.exe")) {
        throw "Failed to compile native Wyrm tools. Please ensure gcc is installed and available in PATH."
    }

    if (!(Test-Path $BinDir)) { New-Item -ItemType Directory -Force -Path $BinDir | Out-Null }
    Copy-Item "wyrm.exe" -Destination (Join-Path $BinDir "wyrm.exe") -Force
    Copy-Item "wyrmc.exe" -Destination (Join-Path $BinDir "wyrmc.exe") -Force
    Copy-Item "wyrpkg.exe" -Destination (Join-Path $BinDir "wyrpkg.exe") -Force
} finally {
    Pop-Location
}

$UserPath = [Environment]::GetEnvironmentVariable("PATH", "User")
if ($UserPath -notlike "*$BinDir*") {
    [Environment]::SetEnvironmentVariable("PATH", "$UserPath;$BinDir", "User")
    Write-Host "Added $BinDir to User PATH." -ForegroundColor Green
} else {
    Write-Host "$BinDir is already in User PATH." -ForegroundColor Yellow
}
$env:PATH += ";$BinDir"

Write-Host "Wyrm v$WyrmVersion native C tools installed globally." -ForegroundColor Green
Write-Host "Open a NEW terminal and run 'wyrm --version', 'wyrm build main.wyr', or 'wyrpkg list'." -ForegroundColor Cyan
