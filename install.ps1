# Wyrm Language Global Installer for Windows

$ScriptDir   = Split-Path -Parent $MyInvocation.MyCommand.Definition
$WyrmVersion = (Get-Content (Join-Path $ScriptDir "VERSION") -ErrorAction SilentlyContinue).Trim()
if (-not $WyrmVersion) { $WyrmVersion = "unknown" }

$InstallDir = Join-Path $HOME ".wyrm"
$BinDir = Join-Path $InstallDir "bin"
$VenvDir = Join-Path $InstallDir "venv"

Write-Host "Initializing Wyrm global installation..." -ForegroundColor Green

# 1. Compile native binaries
Write-Host "Compiling wyrmc and wyrpkg..." -ForegroundColor Cyan
if (Test-Path "wyrmc.exe") { Remove-Item "wyrmc.exe" }
if (Test-Path "wyrpkg.exe") { Remove-Item "wyrpkg.exe" }

gcc wyrmc.c -o wyrmc.exe -std=c11 -O2
gcc wyrpkg.c -o wyrpkg.exe -std=c11 -O2

if (!(Test-Path "wyrmc.exe") -or !(Test-Path "wyrpkg.exe")) {
    Write-Error "Failed to compile wyrmc.exe or wyrpkg.exe. Please ensure gcc is installed and available in PATH."
    exit 1
}

# 2. Create install directories
Write-Host "Creating installation directories at $BinDir..." -ForegroundColor Cyan
if (!(Test-Path $BinDir)) {
    New-Item -ItemType Directory -Force -Path $BinDir | Out-Null
}

# 3. Create isolated virtual environment for Wyrm
Write-Host "Creating isolated Python environment at $VenvDir..." -ForegroundColor Cyan
if (!(Test-Path $VenvDir)) {
    python -m venv $VenvDir
}

# 4. Install Wyrm package inside the isolated environment
Write-Host "Installing Wyrm packages into the environment..." -ForegroundColor Cyan
$PipPath = Join-Path $VenvDir "Scripts\pip.exe"
if (!(Test-Path $PipPath)) {
    $PipPath = Join-Path $VenvDir "bin\pip.exe"
}
if (!(Test-Path $PipPath)) {
    $PipPath = Join-Path $VenvDir "bin\pip"
}
& $PipPath install (Join-Path $ScriptDir ".")

# 5. Copy binaries to install directory
Write-Host "Copying executables to installation path..." -ForegroundColor Cyan
Copy-Item "wyrmc.exe" -Destination (Join-Path $BinDir "wyrmc.exe") -Force
Copy-Item "wyrpkg.exe" -Destination (Join-Path $BinDir "wyrpkg.exe") -Force

# 6. Add to User PATH & current process PATH
Write-Host "Adding Wyrm binary directory to user PATH..." -ForegroundColor Cyan
$UserPath = [Environment]::GetEnvironmentVariable("PATH", "User")
if ($UserPath -notlike "*$BinDir*") {
    $NewUserPath = "$UserPath;$BinDir"
    [Environment]::SetEnvironmentVariable("PATH", $NewUserPath, "User")
    Write-Host "Added $BinDir to User PATH." -ForegroundColor Green
} else {
    Write-Host "$BinDir is already in User PATH." -ForegroundColor Yellow
}

# Apply to current running session immediately
$env:PATH += ";$BinDir"

# Broadcast environment change to Windows explorer (forces new shells to reload PATH instantly)
try {
    $Signature = @'
    [DllImport("user32.dll", SetLastError = true, CharSet = CharSet.Auto)]
    public static extern IntPtr SendMessageTimeout(
        IntPtr hWnd, uint Msg, IntPtr wParam, string lParam,
        uint fuFlags, uint uTimeout, out IntPtr lpdwResult);
'@
    $UpdateEnv = Add-Type -MemberDefinition $Signature -Name "Win32Env" -Namespace "Win32" -PassThru -ErrorAction SilentlyContinue
    if ($UpdateEnv) {
        $Result = 0
        [void]$UpdateEnv::SendMessageTimeout([IntPtr]0xffff, 0x001A, [IntPtr]0, "Environment", 2, 1000, [ref]$Result)
    }
} catch {
    # Ignore broadcast failures
}

Write-Host "Wyrm v$WyrmVersion has been successfully installed globally!" -ForegroundColor Green
Write-Host "You can now open a NEW terminal window and run 'wyrmc --version' or 'wyrpkg --version' from any folder!" -ForegroundColor Cyan

