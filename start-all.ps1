# ============================================================================
# MediPriority - start-all.ps1 (PowerShell)
# Opens the backend and frontend each in their own PowerShell window, since
# both processes block their terminal forever and can't be chained in one.
# ============================================================================

$root = $PSScriptRoot

Start-Process powershell -ArgumentList "-NoExit", "-File", (Join-Path $root "run-backend.ps1")
Start-Process powershell -ArgumentList "-NoExit", "-File", (Join-Path $root "run-frontend.ps1")

Write-Host "Backend health check:  http://127.0.0.1:8080/api/health"
Write-Host "Frontend login page:   http://localhost:5500/login.html"
