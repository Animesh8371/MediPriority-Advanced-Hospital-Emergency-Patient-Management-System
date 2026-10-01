# MediPriority — Advanced Hospital Emergency & Patient Management System

Academic PBL Phase 2 prototype (Team DSCPP-III-T371). Uses **simulated hospital
data** — this is not a real clinical decision-making system.

## What's actually in here

- **C** (`backend/c/`): min-heap (triage priority), FIFO queue (appointments),
  hash table (patient lookup cache), singly linked list (medical history),
  graph + Dijkstra (simulated hospital transfer routing). Compiled as a
  static library `libmedialgo.a`.
- **C++** (`backend/cpp/`): OOP models, services (call into the C library),
  MySQL repositories (Connector/C++, JDBC-style API, prepared statements,
  transactions), and a Crow-based HTTP API.
- **MySQL** (`database/`): `schema.sql` (11 tables, FKs, constraints,
  indexes) + `seed.sql` (sample data).
- **Frontend** (`frontend/`): plain HTML/CSS/JS, 12 pages, talks to the API.
  Includes an Analytics page (trends, doctor workload, wait times, bed
  occupancy -- all rendered as plain CSS bars, no external chart library),
  a live notification bell (long waits / near-full ICU / no free ambulances
  / low doctor availability, computed fresh from MySQL on each poll, never
  stored), and CSV export (a full report from the backend, or just-searched
  patients client-side).

This was built and verified end-to-end during development: the C module's
5 test suites pass, the full stack was compiled with CMake, and every major
flow (login, patient registration/search, triage admit/peek/extract,
Dijkstra routing including the unreachable-hospital case, appointment
booking/FIFO call-next, medical history, bed allocation with transactional
double-booking prevention) was exercised against a real running MySQL
instance and a real running server via curl.

## 0. Opening this in VS Code

The project ships with a `.vscode/` folder (`c_cpp_properties.json`,
`tasks.json`, `launch.json`, `settings.json`, `extensions.json`) and a
`medi-priority.code-workspace` file, so IntelliSense, build tasks and
debugging are pre-wired -- you don't have to configure anything by hand.

1. Open the folder in VS Code, or (recommended, for a clearer C vs C++ view
   in the Explorer sidebar) `File > Open Workspace from File...` and pick
   `medi-priority.code-workspace`. That splits the tree into separate
   top-level entries for the C algorithms library, the C++ backend, the
   database SQL, and the frontend.
2. Install the recommended extensions when VS Code prompts you (or open the
   Extensions panel and run "Show Recommended Extensions"): **C/C++** and
   **CMake Tools**, both from Microsoft.
3. Install the actual toolchain and libraries first (step 1 below) -- the
   `.vscode` config does not install MSYS2/MySQL/OpenSSL for you, it just
   points VS Code at them once they exist. `cmake.configureOnOpen` is
   deliberately set to `false` so you don't get a red error banner before
   you've installed dependencies.
4. Once dependencies are installed and `.env`/environment variables are set
   (steps 2-3 below), use **Terminal > Run Task** (or `Ctrl+Shift+B`) and
   pick `CMake: Build server`, or press `F5` to build-and-debug directly.
   `CMake: Build C module + tests` and `CTest: Run C module tests` are also
   available as tasks if you just want to check the C data-structure module
   on its own.

## 1. Install dependencies (Windows 10/11 + VS Code)

1. **MSYS2** (provides GCC/G++ and CMake in one place): install from
   https://www.msys2.org, then in the MSYS2 UCRT64 terminal:
   ```
   pacman -Syu
   pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-make
   ```
   Add `C:\msys64\ucrt64\bin` to your Windows PATH so `gcc`, `g++`, `cmake`
   work from VS Code's terminal. Verify: `gcc --version`, `cmake --version`.

2. **MySQL Community Server 8.x + MySQL Connector/C++ 8.x**: install both
   via the MySQL Installer for Windows (https://dev.mysql.com/downloads/installer/).
   Note the install path of the Connector/C++ `include` and `lib64` folders
   (typically `C:\Program Files\MySQL\MySQL Connector C++ 8.0\`) — the
   provided `CMakeLists.txt` already looks there by default.

3. **OpenSSL**: MSYS2's `mingw-w64-ucrt-x86_64-openssl` package, or the
   version bundled with your MySQL install (Connector/C++ depends on it
   anyway, so it's often already present).

4. **Crow** (single-header HTTP framework): already vendored at
   `backend/third_party/crow_all.h` — nothing to install. It needs a
   standalone Asio header; install via
   `pacman -S mingw-w64-ucrt-x86_64-asio` (or use the Boost::asio +
   `-DCROW_USE_BOOST` route if you already have Boost).

5. **VS Code extensions**: "C/C++" and "CMake Tools" (both from Microsoft).

## 2. Set up the database

1. Start MySQL (via MySQL Workbench, the Windows service, or `mysqld` directly).
2. Set/confirm your root password, then run:
   ```
   mysql -u root -p < database/schema.sql
   mysql -u root -p < database/seed.sql
   ```
   This creates the `medipriority` database, all 11 tables, and loads sample
   data — including a seeded `admin` / `Admin@123` login.

## 3. Configure environment variables

```
copy .env.example .env
```
Edit `.env` with your real MySQL host/user/password.

**The server loads `.env` automatically** (see `backend/cpp/utils/Config.h`)
— it checks the current folder and a couple of parent folders at startup,
so this works the same whether you launch it from `backend/build/`, from
`backend/`, or from the project root, and **regardless of which shell you
use** (Command Prompt, PowerShell, VS Code's integrated terminal, or a
double-click). You do not need to `set`/`$env:` anything by hand anymore.

> Previously this project only documented the PowerShell `$env:NAME="value"`
> syntax. If you were typing that into Command Prompt (`cmd.exe`), it fails
> silently — `cmd.exe` doesn't understand `$env:`, so none of the variables
> were actually set, and the server fell back to its built-in defaults
> (`root` user, no password), which is almost never your real setup. That's
> the most likely reason the server "wouldn't run properly" from `cmd`. The
> automatic `.env` loading above fixes this for good, but for reference,
> here's the equivalent `cmd.exe` syntax if you ever do want to set them by
> hand (e.g. to override `.env` for one run):
> ```
> set MEDIPRIORITY_DB_HOST=127.0.0.1
> set MEDIPRIORITY_DB_PORT=3306
> set MEDIPRIORITY_DB_USER=root
> set MEDIPRIORITY_DB_PASSWORD=your_password
> set MEDIPRIORITY_DB_NAME=medipriority
> set MEDIPRIORITY_API_PORT=8080
> ```
> and the PowerShell syntax:
> ```powershell
> $env:MEDIPRIORITY_DB_HOST="127.0.0.1"
> $env:MEDIPRIORITY_DB_PORT="3306"
> $env:MEDIPRIORITY_DB_USER="root"
> $env:MEDIPRIORITY_DB_PASSWORD="your_password"
> $env:MEDIPRIORITY_DB_NAME="medipriority"
> $env:MEDIPRIORITY_API_PORT="8080"
> ```
> A real environment variable, if set, always overrides the matching value
> in `.env`.

Two launcher scripts are included at the project root once you've built the
server (step 5): `run.bat` for Command Prompt and `run.ps1` for PowerShell.
Both just check that `.env` and the built `.exe` exist and then start the
server, so you don't have to remember the exact path every time.

## 4. Build the C module (optional standalone check)

```
cd backend
mkdir build && cd build
cmake ..
cmake --build . --target medialgo test_heap test_queue test_hashtable test_linkedlist test_graph
ctest --output-on-failure
```
All 5 tests should print `ALL TESTS PASSED`.

## 5. Build the full backend (C + C++ + MySQL + HTTP API)

Still inside `backend/build`:
```
cmake ..
cmake --build . --target medipriority_server
```
If CMake can't find MySQL Connector/C++, pass its paths explicitly:
```
cmake .. -DMYSQLCPPCONN_INCLUDE_DIR="C:/Program Files/MySQL/MySQL Connector C++ 8.0/include" -DMYSQLCPPCONN_LIB="C:/Program Files/MySQL/MySQL Connector C++ 8.0/lib64/mysqlcppconn.lib"
```

## 6. Start the server

Easiest: from the project root, run `start-all.bat` (Command Prompt) or
`.\start-all.ps1` (PowerShell). This opens the backend and the frontend
static server **each in their own window** and points PATH at your MySQL
Connector/C++ install automatically. That last part matters: the connector's
`.dll` has to be found by Windows at *runtime*, not just when linking, so if
you start `medipriority_server.exe` directly without it on PATH you'll get a
missing-DLL error that looks like the server "isn't running properly" even
though the build succeeded.

> **Why two windows, not one?** `medipriority_server.exe` runs forever and
> never hands control back to its terminal — so if you type `cd frontend`
> and `npx serve` right after starting it in the *same* window, those
> commands just sit there typed and never actually run. Two windows, two
> processes, is not optional here.

If you'd rather run it by hand, `run-backend.bat`/`run-backend.ps1` do just
the backend half (auto-detecting your build output under `backend/build*`
and your Connector/C++ install under `C:\Program Files\MySQL\`), for example:
```
cd backend\build-msvc\Release
set PATH=C:\Program Files\MySQL\MySQL Connector C++ 26.7\lib64;%PATH%
medipriority_server.exe
```
You should see it connect to MySQL and warm up each C data structure
(hash table, heap, queue, graph) from the database, then start listening
on port 8080. Check `http://localhost:8080/api/health`.

## 7. Run the frontend

`run-frontend.bat`/`run-frontend.ps1` do this for you (see step 6), or by hand:
```
cd frontend
npx serve . -l 5500
```
then visit `http://localhost:5500/login.html`. Log in with
`admin` / `Admin@123`. Opening `frontend/login.html` directly in a browser
also works in most setups, but a static server avoids any cross-origin
issues some browsers apply to `file://` pages.

## 8. Test the application

- C module: `ctest` in `backend/build` (see step 4).
- API: use curl or Postman against the endpoints in `docs/api.md`.
- Frontend: walk through each page manually; every button performs a real
  API call (no fake buttons).

See `docs/testing.md` for the specific edge cases (empty heap, no beds
available, unreachable hospital, duplicate patient, double-booked bed) and
exactly how each was exercised during development.

## Known limitations (stated, not hidden)

- Session tokens are kept in server memory only — a server restart logs
  everyone out. A production system would use signed/expiring tokens or a
  shared session store.
- Password hashing is salted SHA-256 with 10,000 rounds, not bcrypt/Argon2.
  Documented trade-off in `docs/architecture.md`.
- The `Database` wrapper holds a single MySQL connection, not a pool.
- Dijkstra runs at O(V²) (simple array scan), fine for the small simulated
  network here; a heap-based O((V+E) log V) version is natural future work.
- This is a simulated academic dataset. Nothing here is a real medical
  diagnosis, real clinical triage tool, or real-time hospital routing.
