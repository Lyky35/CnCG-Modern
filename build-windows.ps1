# CnC Generals Zero Hour - Windows x64 Build Script
# Requires: Visual Studio 2022, CMake 3.20+, Vulkan SDK

param(
    [string]$BuildType = "Release",
    [string]$Preset = "windows-x64-msvc"
)

$ErrorActionPreference = "Stop"

Write-Host "=== CnC Generals Zero Hour - Windows x64 Build ===" -ForegroundColor Cyan
Write-Host "Build type: $BuildType"
Write-Host "Preset: $Preset"
Write-Host ""

# Check for Visual Studio 2022
$vsPath = "${env:ProgramFiles}\Microsoft Visual Studio\2022"
if (-not (Test-Path $vsPath)) {
    Write-Error "Visual Studio 2022 not found. Please install VS 2022 with C++ workload."
    exit 1
}

# Check for Vulkan SDK
if (-not $env:VULKAN_SDK) {
    Write-Warning "VULKAN_SDK environment variable not set. Vulkan renderer may not work."
}

# Configure
Write-Host "Configuring..." -ForegroundColor Yellow
cmake --preset $Preset
if ($LASTEXITCODE -ne 0) {
    Write-Error "CMake configuration failed."
    exit 1
}

# Build
Write-Host "Building..." -ForegroundColor Yellow
cmake --build --preset $Preset
if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed."
    exit 1
}

# Verify output
$exePath = "build/$Preset/bin/RTS.exe"
if (Test-Path $exePath) {
    Write-Host ""
    Write-Host "=== Build Successful ===" -ForegroundColor Green
    Write-Host "Executable: $exePath"

    # Check for x64
    $dumpbin = "${env:ProgramFiles}\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\*\bin\Hostx64\x64\dumpbin.exe"
    $dumpbinPath = (Resolve-Path $dumpbin -ErrorAction SilentlyContinue | Select-Object -First 1).Path
    if ($dumpbinPath) {
        Write-Host ""
        Write-Host "Verifying x64 architecture..." -ForegroundColor Yellow
        & $dumpbinPath /headers $exePath | Select-String "machine"
    }
} else {
    Write-Error "RTS.exe not found at expected path: $exePath"
    exit 1
}
