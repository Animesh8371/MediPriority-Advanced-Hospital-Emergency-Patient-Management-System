# ============================================================================
# MediPriority - run-backend.ps1 (PowerShell)
# Starts the compiled server with the MySQL Connector/C++ DLL on PATH.
# ============================================================================

$ProjectRoot = $PSScriptRoot

$candidates = @(
    "backend\build-msvc\Release",
    "backend\build-msvc\Debug",
    "backend\build\Release",
    "backend\build\Debug",
    "backend\build"
)
$backendExeDir = $null
foreach ($c in $candidates) {
    $full = Join-Path $ProjectRoot $c
    if (Test-Path (Join-Path $full "medipriority_server.exe")) { $backendExeDir = $full; break }
}

if (-not $backendExeDir) {
    Write-Host "[run-backend.ps1] Could not find medipriority_server.exe under backend\build*." -ForegroundColor Yellow
    Write-Host "Build it first (see README.md section 5), or edit this script to point at your build output."
    exit 1
}

# Auto-detect the MySQL Connector/C++ lib64 folder so its DLL is found at runtime.
$connectorLib = Get-ChildItem "C:\Program Files\MySQL" -Directory -Filter "MySQL Connector C++*" -ErrorAction SilentlyContinue |
    ForEach-Object { Join-Path $_.FullName "lib64" } |
    Where-Object { Test-Path $_ } |
    Select-Object -First 1

if ($connectorLib) {
    Write-Host "[run-backend.ps1] Using MySQL Connector/C++ from: $connectorLib" -ForegroundColor Green
    $env:PATH = "$connectorLib;$env:PATH"
} else {
    Write-Host "[run-backend.ps1] Warning: could not auto-detect the MySQL Connector/C++ lib64 folder." -ForegroundColor Yellow
    Write-Host "If the server fails with a missing-DLL error, set it manually, e.g.:"
    Write-Host '  $env:PATH = "C:\Program Files\MySQL\MySQL Connector C++ 26.7\lib64;$env:PATH"'
}

if (-not (Test-Path (Join-Path $ProjectRoot ".env"))) {
    Write-Host "[run-backend.ps1] No .env file found at the project root." -ForegroundColor Yellow
    Write-Host "Copy .env.example to .env and fill in your real MySQL details."
}

Write-Host "[run-backend.ps1] Starting MediPriority backend from $backendExeDir ..." -ForegroundColor Green
Set-Location $backendExeDir
& ".\medipriority_server.exe"
