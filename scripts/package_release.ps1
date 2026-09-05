# Wyrm v3.2.0 Binary Release Packager for Windows x64
$ErrorActionPreference = "Stop"

$RootDir = (Get-Item $PSScriptRoot).Parent.FullName
$Version = (Get-Content (Join-Path $RootDir "VERSION")).Trim()
$DistDir = Join-Path $RootDir "dist"
$StageDir = Join-Path $DistDir "wyrm-v$Version-windows-x64"

Write-Host "Packaging Wyrm v$Version for Windows x64..." -ForegroundColor Green

# Clean and recreate staging directories
if (Test-Path $DistDir) { Remove-Item $DistDir -Recurse -Force }
New-Item -ItemType Directory -Force -Path $StageDir | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $StageDir "bin") | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $StageDir "packages\wyrmlang\lib") | Out-Null

# 1. Copy executables to bin/
$wyrmcSrc = Join-Path $env:USERPROFILE ".wyrm\wyrmc\wyrmc.exe"
if (!(Test-Path $wyrmcSrc)) { $wyrmcSrc = Join-Path $RootDir "wyrmc.exe" }

$wyrpkgSrc = Join-Path $env:USERPROFILE ".wyrm\wyrpkg\wyrpkg.exe"
if (!(Test-Path $wyrpkgSrc)) { $wyrpkgSrc = Join-Path $RootDir "wyrpkg.exe" }

Copy-Item $wyrmcSrc -Destination (Join-Path $StageDir "bin\wyrmc.exe") -Force
Copy-Item $wyrpkgSrc -Destination (Join-Path $StageDir "bin\wyrpkg.exe") -Force

# Copy SDL2.dll if present
$sdlSrc = Join-Path $RootDir "SDL2.dll"
if (Test-Path $sdlSrc) {
    Copy-Item $sdlSrc -Destination (Join-Path $StageDir "bin\SDL2.dll") -Force
}

# 2. Copy C runtime and standard library packages
Copy-Item (Join-Path $RootDir "wyrm\lib\*") -Destination (Join-Path $StageDir "packages\wyrmlang\lib") -Recurse -Force
Copy-Item (Join-Path $RootDir "VERSION") -Destination (Join-Path $StageDir "packages\wyrmlang\VERSION") -Force

# 3. Copy documentation and license
Copy-Item (Join-Path $RootDir "LICENSE") -Destination $StageDir -Force
Copy-Item (Join-Path $RootDir "README.md") -Destination $StageDir -Force
Copy-Item (Join-Path $RootDir "VERSION") -Destination $StageDir -Force

# 4. Create install.bat for manual unzip users
$batContent = @"
@echo off
setlocal
echo Installing Wyrm v$Version...

set "TARGET=%USERPROFILE%\.wyrm"
if not exist "%TARGET%\wyrmc" mkdir "%TARGET%\wyrmc"
if not exist "%TARGET%\wyrpkg" mkdir "%TARGET%\wyrpkg"
if not exist "%TARGET%\packages\wyrmlang" mkdir "%TARGET%\packages\wyrmlang"

copy /y "%~dp0bin\wyrmc.exe" "%TARGET%\wyrmc\" >nul
copy /y "%~dp0bin\wyrpkg.exe" "%TARGET%\wyrpkg\" >nul
if exist "%~dp0bin\SDL2.dll" copy /y "%~dp0bin\SDL2.dll" "%TARGET%\wyrmc\" >nul
xcopy /s /e /y /q "%~dp0packages\wyrmlang\*" "%TARGET%\packages\wyrmlang\" >nul

powershell -NoProfile -Command "[Environment]::SetEnvironmentVariable('PATH', [Environment]::GetEnvironmentVariable('PATH', 'User') + ';%TARGET%\wyrmc;%TARGET%\wyrpkg', 'User')" >nul 2>&1

echo Wyrm v$Version installed successfully to %TARGET%!
echo Please restart your terminal or type: wyrmc --version
pause
"@
Set-Content -Path (Join-Path $StageDir "install.bat") -Value $batContent -Encoding ASCII

# 5. Create ZIP archives
$zipPathV = Join-Path $DistDir "wyrm-v$Version-windows-x64.zip"
$zipPathPlain = Join-Path $DistDir "wyrm-$Version-windows-x64.zip"

Write-Host "Compressing $zipPathV..." -ForegroundColor Cyan
Compress-Archive -Path "$StageDir\*" -DestinationPath $zipPathV -Force
Copy-Item $zipPathV -Destination $zipPathPlain -Force

# 6. Calculate SHA256 checksum
$hash = (Get-FileHash -Path $zipPathV -Algorithm SHA256).Hash
Set-Content -Path (Join-Path $DistDir "wyrm-v$Version-windows-x64.zip.sha256") -Value "$hash  wyrm-v$Version-windows-x64.zip" -Encoding ASCII

Write-Host "Archive SHA256: $hash" -ForegroundColor Yellow

# 7. Generate Scoop Manifest
$scoopDir = Join-Path $RootDir "scoop"
if (!(Test-Path $scoopDir)) { New-Item -ItemType Directory -Force -Path $scoopDir | Out-Null }

$scoopManifest = @"
{
    "version": "$Version",
    "description": "A static systems programming language with a self-hosted compiler and gradual static typing",
    "homepage": "https://github.com/neofilisoft/wyrm",
    "license": "MIT",
    "architecture": {
        "64bit": {
            "url": "https://github.com/neofilisoft/wyrm/releases/download/v$Version/wyrm-v$Version-windows-x64.zip",
            "hash": "$hash"
        }
    },
    "bin": [
        "bin\\wyrmc.exe",
        "bin\\wyrpkg.exe"
    ],
    "post_install": [
        "`$targetDir = Join-Path `$env:USERPROFILE '.wyrm\\packages\\wyrmlang'",
        "if (!(Test-Path `$targetDir)) { New-Item -ItemType Directory -Force -Path `$targetDir | Out-Null }",
        "Copy-Item -Path (Join-Path `$dir 'packages\\wyrmlang\\*') -Destination `$targetDir -Recurse -Force"
    ],
    "checkver": "github",
    "autoupdate": {
        "architecture": {
            "64bit": {
                "url": "https://github.com/neofilisoft/wyrm/releases/download/v`$version/wyrm-v`$version-windows-x64.zip"
            }
        }
    }
}
"@

Set-Content -Path (Join-Path $scoopDir "wyrm.json") -Value $scoopManifest -Encoding UTF8
Set-Content -Path (Join-Path $RootDir "wyrm.json") -Value $scoopManifest -Encoding UTF8

Write-Host "Generated Scoop manifest at:" -ForegroundColor Green
Write-Host "  $scoopDir\wyrm.json"
Write-Host "  $RootDir\wyrm.json"
Write-Host "Package creation completed successfully!" -ForegroundColor Green
