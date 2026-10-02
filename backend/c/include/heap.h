#ifndef MEDIPRIORITY_HEAP_H
#define MEDIPRIORITY_HEAP_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * EmergencyCase
 * -------------
 * severity: 1 = Critical, 2 = High, 3 = Moderate, 4 = Low
 *   (lower number = more urgent). This is a SIMULATED academic
 *   severity score, not a real clinical triage value.
 * arrival_time: monotonically increasing counter (or unix timestamp).
 *   Used ONLY to break ties within the same severity band, so that
 *   "first come, first served" holds among equally-severe cases.
 * priority_key: computed by heap_insert()/heap_update_priority() as
 *   severity * 1,000,000,000L + arrival_time
 *   A min-heap on this key naturally pops the most severe case first,
 *   and among equal severities, the one that arrived earliest.
 */
typedef struct {
    int  case_id;
    int  patient_id;
    int  severity;
    long arrival_time;
    long priority_key;
} EmergencyCase;

typedef struct {
    EmergencyCase *data;
    int size;
    int capacity;
} MinHeap;

/* Create a heap with the given starting capacity (grows automatically). */
MinHeap *heap_create(int initial_capacity);

/* Free all memory owned by the heap. Safe to call with NULL. */
void heap_destroy(MinHeap *h);

/* Insert a case. priority_key is computed internally from severity/arrival_time.
 * Returns 1 on success, 0 on failure (e.g. allocation failure). */
int heap_insert(MinHeap *h, EmergencyCase ec);

/* Remove and return the most urgent case. Returns 1 on success, 0 if empty. */
int heap_extract_min(MinHeap *h, EmergencyCase *out);

/* Look at the most urgent case without removing it. Returns 1 on success, 0 if empty. */
int heap_peek(MinHeap *h, EmergencyCase *out);

int heap_is_empty(MinHeap *h);
int heap_size(MinHeap *h);

/* Change the severity of an already-queued case (e.g. condition worsens) and
 * re-heapify. Returns 1 if found and updated, 0 if case_id not present. */
int heap_update_priority(MinHeap *h, int case_id, int new_severity);

/* Remove a specific case (e.g. patient was already treated elsewhere).
 * Returns 1 if found and removed, 0 if case_id not present. */
int heap_remove_case(MinHeap *h, int case_id);

#ifdef __cplusplus
}
#endif

#endif /* MEDIPRIORITY_HEAP_H */
