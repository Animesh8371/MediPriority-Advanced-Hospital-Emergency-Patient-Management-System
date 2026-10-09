# API Reference

Base URL: `http://localhost:8080` (configurable via `MEDIPRIORITY_API_PORT`).
All endpoints except `/api/auth/*` and `/api/health` require
`Authorization: Bearer <token>` (obtained from `/api/auth/login`).
All request/response bodies are JSON. Errors return `{"success": false,
"message": "..."}` with a non-2xx status code.

## Auth

| Method | Path | Purpose | Body | C module involved |
|---|---|---|---|---|
| POST | `/api/auth/register` | Create a user | `{username, password, role}` | none |
| POST | `/api/auth/login` | Log in, get a session token | `{username, password}` | none |
| POST | `/api/auth/logout` | Invalidate the current token | (uses header) | none |

## Patients

| Method | Path | Purpose | C module |
|---|---|---|---|
| POST | `/api/patients` | Register a patient. Validates name/age/gender/phone/blood group. Detects (but does not block) likely duplicates by name+phone. | hash table (cache warm) |
| GET | `/api/patients/{id}` | Fetch one patient | hash table (fast path, falls back to MySQL) |
| GET | `/api/patients/search?name=...` | Fuzzy name search | none (MySQL `LIKE`) |

## Emergency triage

| Method | Path | Purpose | C module |
|---|---|---|---|
| POST | `/api/emergency-cases` | Admit a case `{patient_id, severity(1-4), category}` | **min-heap**: inserted after MySQL row created |
| GET | `/api/emergency-cases/priority-queue` | Peek the most urgent waiting case + count | **min-heap**: `heap_peek` |
| POST | `/api/triage/prioritize` | Extract (remove) the most urgent case, mark "In Treatment" | **min-heap**: `heap_extract_min` |
| PUT | `/api/emergency-cases/{id}/severity` | Change severity (condition worsens/improves) `{severity}` | **min-heap**: `heap_update_priority` |

Severity: `1=Critical, 2=High, 3=Moderate, 4=Low`. This is a simulated
academic severity score, not a real clinical value.

## Doctors

| Method | Path | Purpose |
|---|---|---|
| GET | `/api/doctors` | List all doctors |
| POST | `/api/doctors` | Add a doctor `{name, age, gender, specialization}` |
| POST | `/api/doctors/{id}/assign` | Mark busy (assigned to a case) |
| POST | `/api/doctors/{id}/release` | Mark available again |

## Beds

| Method | Path | Purpose |
|---|---|---|
| GET | `/api/beds` | List all beds |
| POST | `/api/beds` | Add a bed `{bed_type: General\|Emergency\|ICU}` |
| POST | `/api/beds/allocate` | Allocate `{bed_id, patient_id, doctor_id?}` — transactional, rejects if already occupied (409) |
| POST | `/api/beds/release` | Release `{bed_id}` |

## Ambulances

| Method | Path | Purpose |
|---|---|---|
| GET | `/api/ambulances` | List all |
| POST | `/api/ambulances` | Register `{vehicle_number}` |
| POST | `/api/ambulances/assign` | Assign `{ambulance_id}` — rejects if not Available (409) |
| POST | `/api/ambulances/release` | Release `{ambulance_id}` |

## Appointments

| Method | Path | Purpose | C module |
|---|---|---|---|
| GET | `/api/appointments` | List upcoming (Scheduled) | none |
| POST | `/api/appointments` | Book `{patient_id, doctor_id, scheduled_time}` — rejects exact-time doctor conflicts (409) | **FIFO queue**: enqueued after MySQL insert |
| POST | `/api/appointments/call-next` | Dequeue the next appointment in booked order | **FIFO queue**: `queue_dequeue` |
| POST | `/api/appointments/{id}/cancel` | Cancel | none |

## Medical history

| Method | Path | Purpose | C module |
|---|---|---|---|
| POST | `/api/medical-history` | Add entry `{patient_id, diagnosis, notes}` | **linked list**: appended after MySQL insert |
| GET | `/api/medical-history/{patient_id}` | Chronological history (reads MySQL directly, complete across restarts) | none (by design — see architecture.md) |

## Hospital routing

| Method | Path | Purpose | C module |
|---|---|---|---|
| GET | `/api/hospitals` | List simulated hospitals | graph (loaded at startup) |
| POST | `/api/routing/shortest-path` | `{from_hospital_id, to_hospital_id}` — Dijkstra shortest path over the simulated network. Returns 404 with a message if unreachable. | **graph + Dijkstra** |

## Reports & Analytics

| Method | Path | Purpose |
|---|---|---|
| GET | `/api/reports/summary` | Live dashboard counts, pulled directly from MySQL (never fabricated) |
| GET | `/api/reports/analytics` | Live trends: 7-day emergency arrivals, busiest doctors (active-case count), average wait time by severity for currently-waiting cases, bed occupancy by type, appointment status breakdown |
| GET | `/api/reports/export.csv` | Downloads the summary + analytics above as a single CSV file |

## Notifications

| Method | Path | Purpose |
|---|---|---|
| GET | `/api/notifications` | Live operational alerts (`level`: `critical`\|`warning`\|`info`), recomputed from current MySQL state on every call. No alert is ever stored -- there's no "unread" flag to manage, and a restart doesn't lose or duplicate anything. Current rules: critical cases waiting > 15 min, ICU beds ≥ 90% occupied, zero ambulances available, < 20% of doctors available; if none of those fire but cases are waiting, one informational alert is returned instead. |

## Health

| Method | Path | Purpose |
|---|---|---|
| GET | `/api/health` | Liveness check, no auth required |

## Advanced admission (new)

### POST /api/admissions
Registers a patient, creates the emergency case (priority queue) and auto-allocates a bed and doctor.
Body: `name, age, gender, phone, blood_group, emergency_contact, date_of_birth, severity (1-4), category`.
Ward by severity: 1 -> ICU (fallback Emergency), 2 -> Emergency (General), 3 -> General (Emergency), 4 -> General.
Doctor: on duty, below `max_patients`, specialty matched to `category`, least loaded first.
Response (201): `patient_id`, `patient_code` (MP-000042), `case_id`, `bed` or null, `doctor` or null, `waitlisted`.

### POST /api/admissions/{caseId}/allocate
Retry bed + doctor allocation for a waitlisted case after a bed is released.

### GET /api/admissions/preview?severity=1&category=Cardiac
Read-only: the ward and doctor the allocator would pick right now.

### GET /api/doctors/{id}/patients
Patients a doctor currently handles. Doctor JSON now has `max_patients`, `current_patients`, `free_slots`, `accepting`.
`POST /api/doctors` accepts optional `max_patients` (1-50, default 5).
