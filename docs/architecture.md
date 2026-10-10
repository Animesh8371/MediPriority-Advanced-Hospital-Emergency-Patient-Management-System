# Architecture

## Layered flow

```
Frontend (HTML/CSS/JS)
    |  HTTP/JSON
Crow controllers (backend/cpp/controllers/)
    |
Services (backend/cpp/services/)  <-- business logic + C library calls
    |                    \
Repositories (MySQL)      C library (backend/c/) via extern "C"
    |
MySQL (database/schema.sql)
```

Every one of the 5 mandated C data structures is called from a specific
C++ service, not left unused:

| C structure | Service | Why |
|---|---|---|
| Min-heap (`heap.c`) | `TriageService` | Emergency case priority ordering |
| FIFO queue (`queue.c`) | `AppointmentService` | Strict booked-order appointment calling |
| Hash table (`hashtable.c`) | `PatientService` | Fast in-session patient lookup cache |
| Linked list (`linkedlist.c`) | `MedicalHistoryService` | Append-only chronological history |
| Graph + Dijkstra (`graph.c`, `dijkstra.c`) | `RoutingService` | Simulated hospital transfer routing |

## In-memory structure vs. MySQL

For every one of the above, **MySQL is the single persistent source of
truth**; the C structure is a live, in-memory index/cache rebuilt from
MySQL at server startup (`loadFromDatabase()` on each service) and kept in
sync on every write (write to MySQL first, then to the C structure, so a
crash never leaves an entry only in memory). On restart, the in-memory
structures are empty and get rebuilt — nothing is lost, because nothing
authoritative ever lived only in C memory.

## C/C++ integration

The C sources compile into a static library `libmedialgo.a`. Every C header
wraps its declarations in `extern "C" { ... }` (guarded by `#ifdef
__cplusplus`), so C++ can include them directly without name-mangling
issues. Data crosses the boundary as plain C structs (e.g. `EmergencyCase`,
`Appointment`) — never `std::string`/STL containers directly — with the
C++ layer converting to/from `std::string` at the boundary.

## Auth (added to MVP)

A `users` table stores `username`, a salted-and-hashed password, and a
`role` (`admin`/`staff`). `AuthService` issues an opaque 32-byte random
session token on login, kept in an in-memory `unordered_map` (token ->
user_id). Every protected route checks `Authorization: Bearer <token>` via
`AuthMiddleware::authenticate()`.

**Known limitation, stated plainly**: this is a simplified scheme for an
academic prototype. Two specific simplifications:
1. Sessions live only in server memory — restarting the server logs
   everyone out. A production system would use signed/expiring JWTs or a
   shared session store (Redis, DB-backed sessions).
2. Passwords are hashed with salted SHA-256 (10,000 rounds), not
   bcrypt/Argon2. Plain SHA-256, even salted and iterated, is much faster
   to brute-force than a dedicated password-hashing algorithm. This was a
   deliberate trade-off to avoid adding another external dependency beyond
   what the project already requires (OpenSSL, which Connector/C++ already
   depends on transitively), keeping the build simple for a student project.

## MySQL connection

`backend/cpp/utils/Database.h` wraps a single `sql::Connection` (MySQL
Connector/C++, classic JDBC-style API) opened once at startup and reused.
This is intentionally **not** a connection pool — correct for demonstrating
proper open/reuse/close connection management in an academic MVP, but a
known limitation for concurrent load at any real scale.

## Transactions

`BedRepository::allocate()` and `AmbulanceRepository::assign()` both run
inside an explicit transaction with `SELECT ... FOR UPDATE` to re-check
availability at the moment of allocation, preventing two near-simultaneous
requests from double-booking the same bed/ambulance. This was verified
directly: allocating an already-occupied bed returns HTTP 409, never a
silent double-allocation.

## Known limitations / future scope

- Dijkstra is O(V²) (simple array scan over up to `MAX_HOSPITALS = 32`
  nodes) rather than heap-based O((V+E) log V) — fine at this scale,
  documented as a natural optimization for a larger simulated network.
- No connection pooling (see above).
- No rate limiting / brute-force lockout on login.
- The routing module is a simulated, static graph — not connected to any
  real mapping service, and not real-time.
