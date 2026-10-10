@echo off
setlocal enabledelayedexpansion

REM ============================================================================
REM MediPriority - run-backend.bat (Command Prompt)
REM Starts the compiled server with the MySQL Connector/C++ DLL on PATH.
REM
REM This looks for the .exe in the common build output locations. If yours
REM is somewhere else, edit BACKEND_EXE_DIR below to point at it directly.
REM ============================================================================

set "PROJECT_ROOT=%~dp0"

REM --- Find the built server executable ---
set "BACKEND_EXE_DIR="
for %%D in (
    "backend\build-msvc\Release"
    "backend\build-msvc\Debug"
    "backend\build\Release"
    "backend\build\Debug"
    "backend\build"
) do (
    if exist "%PROJECT_ROOT%%%~D\medipriority_server.exe" if not defined BACKEND_EXE_DIR (
        set "BACKEND_EXE_DIR=%PROJECT_ROOT%%%~D"
    )
)

if not defined BACKEND_EXE_DIR (
    echo [run-backend.bat] Could not find medipriority_server.exe under backend\build* .
    echo Build it first ^(see README.md section 5^), or edit BACKEND_EXE_DIR in this
    echo script to point directly at the folder containing the .exe.
    pause
    exit /b 1
)

REM --- Find the MySQL Connector/C++ lib64 folder, so its DLL is on PATH ---
REM (This is very likely why the server "wasn't running properly": the
REM  connector's DLL has to be found at runtime, not just at link time.)
set "CONNECTOR_LIB="
for /d %%D in ("C:\Program Files\MySQL\MySQL Connector C++*") do (
    if exist "%%D\lib64" if not defined CONNECTOR_LIB set "CONNECTOR_LIB=%%D\lib64"
)

if not defined CONNECTOR_LIB (
    echo [run-backend.bat] Warning: could not auto-detect the MySQL Connector/C++
    echo lib64 folder under "C:\Program Files\MySQL\". If the server fails to
    echo start with a missing-DLL error, set CONNECTOR_LIB manually below this line.
    REM set "CONNECTOR_LIB=C:\Program Files\MySQL\MySQL Connector C++ 26.7\lib64"
) else (
    echo [run-backend.bat] Using MySQL Connector/C++ from: %CONNECTOR_LIB%
    set "PATH=%CONNECTOR_LIB%;%PATH%"
)

if not exist "%PROJECT_ROOT%.env" (
    echo [run-backend.bat] No .env file found at the project root.
    echo Copy .env.example to .env and fill in your real MySQL details --
    echo the server will otherwise fall back to root / no password.
)

echo [run-backend.bat] Starting MediPriority backend from %BACKEND_EXE_DIR% ...
cd /d "%BACKEND_EXE_DIR%"
medipriority_server.exe

pause
