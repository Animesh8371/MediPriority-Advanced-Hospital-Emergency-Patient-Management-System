# ============================================================================
# MediPriority - run-frontend.ps1 (PowerShell)
# Serves the frontend as static files on http://localhost:5500.
# Requires Node.js (for npx) to be installed.
# ============================================================================

$FrontendDir = Join-Path $PSScriptRoot "frontend"

if (-not (Test-Path $FrontendDir)) {
    Write-Host "[run-frontend.ps1] Could not find the frontend folder at: $FrontendDir" -ForegroundColor Yellow
    exit 1
}

Set-Location $FrontendDir
Write-Host "[run-frontend.ps1] Serving frontend at http://localhost:5500/login.html" -ForegroundColor Green
npx serve . -l 5500
