@echo off
REM ============================================================================
REM MediPriority - start-all.bat (Command Prompt)
REM
REM medipriority_server.exe (and npx serve) each block their own terminal
REM forever, so they can't be chained one after another in a single window --
REM that's why commands typed after the server line never seemed to run.
REM This opens each one in its own window instead.
REM ============================================================================

echo Starting backend and frontend in separate windows...
start "MediPriority Backend"  cmd /k "%~dp0run-backend.bat"
start "MediPriority Frontend" cmd /k "%~dp0run-frontend.bat"

echo.
echo Backend health check:  http://127.0.0.1:8080/api/health
echo Frontend login page:   http://localhost:5500/login.html
