@echo off
setlocal

REM ============================================================================
REM MediPriority - run-frontend.bat (Command Prompt)
REM Serves the frontend as static files on http://localhost:5500, so the
REM browser doesn't hit file:// cross-origin restrictions when calling the API.
REM Requires Node.js (for npx) to be installed.
REM ============================================================================

set "FRONTEND_DIR=%~dp0frontend"

if not exist "%FRONTEND_DIR%" (
    echo [run-frontend.bat] Could not find the frontend folder at:
    echo   %FRONTEND_DIR%
    pause
    exit /b 1
)

cd /d "%FRONTEND_DIR%"
echo [run-frontend.bat] Serving frontend at http://localhost:5500/login.html
npx serve . -l 5500

pause
