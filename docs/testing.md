# Testing

This documents what was **actually run**, not a claimed test plan. Timestamps
and output are from the development session that produced this project.

## C module (Phase 2)

Built and run via CMake/CTest:
```
cd backend/build
cmake ..
cmake --build . --target test_heap test_queue test_hashtable test_linkedlist test_graph
ctest --output-on-failure
```
Result (actual, all 5 passed):
```
1/5 Test #1: test_heap ........................   Passed
2/5 Test #2: test_queue .......................   Passed
3/5 Test #3: test_hashtable ...................   Passed
4/5 Test #4: test_linkedlist ..................   Passed
5/5 Test #5: test_graph .......................   Passed
100% tests passed, 0 tests failed out of 5
```

Edge cases covered inside these tests:
- **Empty heap**: `heap_extract_min`/`heap_peek` on an empty heap return 0
  cleanly (no crash). Verified in `test_heap.c`.
- **Empty queue**: `queue_dequeue`/`queue_peek` on empty return 0 cleanly.
  Verified in `test_queue.c`.
- **Unreachable hospital**: Dijkstra correctly returns 0 (failure) for a
  destination with no path, rather than a bogus distance. Verified in
  `test_graph.c` with an intentionally isolated 5th hospital node.
- **Hash collisions**: `test_hashtable.c` uses a deliberately small
  bucket count to force collisions and confirms chaining still finds the
  right record.
- **Duplicate-key overwrite vs. new entry**: re-inserting an existing
  `patient_id` into the hash table overwrites in place (count unchanged),
  confirmed in `test_hashtable.c`.

Compiled with `-Wall -Wextra`, zero warnings.

## Full stack (Phases 3-5)

Built with:
```
cd backend/build
cmake ..
cmake --build . --target medipriority_server
```
Result: compiled and linked cleanly against MySQL Connector/C++
(`libmysqlcppconn`), OpenSSL, and pthreads.

### End-to-end run against a real MySQL instance

`schema.sql` and `seed.sql` were loaded into a real MySQL 8.0 server, the
compiled `medipriority_server` was started against it, and the following
was exercised with `curl` (not simulated):

1. **Login** as the seeded `admin` / `Admin@123` account — token issued.
2. **Unauthorized access**: a request to `/api/patients/1` with no token
   correctly returned HTTP 401.
3. **Patient registration**, retrieval by ID, and name search — all
   returned correct data reflecting what was actually stored in MySQL.
4. **Dashboard summary** (`/api/reports/summary`) reflected the real,
   current row counts in MySQL, including the just-registered patient.
5. **Triage heap**: three emergency cases were admitted out of severity
   order (Moderate, then Critical, then Low). Peeking the priority queue
   immediately after correctly returned the **Critical** case, not the
   first-admitted one. Extracting it and peeking again correctly surfaced
   the pre-existing seeded **High**-severity case next — proving both the
   severity ordering and the arrival-time tie-break logic work through the
   real HTTP -> service -> C heap path, not just in the isolated C test.
6. **Dijkstra routing**: requesting the shortest path from hospital 1 to
   hospital 4 correctly returned the 3-hop route (cost 10) rather than the
   shorter-looking but more expensive 2-hop direct route (cost 12).
   Requesting a path to the deliberately isolated hospital 5 correctly
   returned a 404 "unreachable" response.
7. **Appointments FIFO**: two appointments were booked, and "call next"
   correctly returned the first one booked, not the second.
8. **Medical history**: an entry was added and then retrieved for the same
   patient, in chronological order alongside the seeded entries.
9. **Bed allocation transaction**: allocating bed #1 succeeded; immediately
   allocating the same bed #1 to a different patient was correctly
   **rejected with HTTP 409** rather than silently double-booking it.
10. **Doctor creation** succeeded and appeared in subsequent listings.

### Known constraint on how this was tested

Background processes (MySQL, the server) do not persist between separate
tool invocations in the development sandbox used to build this project, so
each end-to-end run above was executed as one consolidated script (start
MySQL -> load schema/seed -> start server -> run curl assertions) rather
than as separately-timed steps. This has no bearing on the correctness of
the code itself, only on how the verification session was structured — it
is noted here for transparency rather than left unstated.

## What was *not* run

- No load/concurrency testing beyond the single-request transaction check
  above (bed double-allocation). A real concurrent-request race test would
  need multiple simultaneous clients, which is reasonable further work.
- No `valgrind` memory-leak run: valgrind's Debian package failed to fetch
  in the sandbox during development. The C module's malloc/free are
  paired 1:1 per `_create`/`_destroy` function by inspection, but this
  should be verified with `valgrind --leak-check=full ./build/test_heap`
  (etc.) on a machine where valgrind installs successfully.
- No automated frontend/browser test suite — the frontend was written
  against the documented API contract and each page's fetch calls were
  code-reviewed against the actual controller signatures, but not driven
  by a browser automation tool in this session.

## Recommended manual test checklist for your PBL demo

- [ ] Register a patient with invalid age (e.g. 0 or 200) → rejected
- [ ] Register the same name+phone twice → duplicate warning shown, not blocked
- [ ] Admit 3 emergency cases with different severities → priority queue
      always shows the most severe first
- [ ] Extract all cases until the queue is empty → next peek shows "no cases waiting"
- [ ] Try to allocate an already-occupied bed → rejected
- [ ] Try to release a bed that isn't allocated → handled without crashing
- [ ] Request a route to a hospital with no connecting edges → "unreachable"
- [ ] Book two appointments for the same doctor at the same time → conflict rejected
