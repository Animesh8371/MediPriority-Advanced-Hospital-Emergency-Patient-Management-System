#ifndef MEDIPRIORITY_TRIAGESERVICE_H
#define MEDIPRIORITY_TRIAGESERVICE_H

extern "C" {
#include "heap.h"
}
#include "../repositories/EmergencyCaseRepository.h"
#include "../models/EmergencyCase.h"
#include <string>
#include <vector>
#include <mutex>

namespace medipriority {

struct TriageResult {
    bool success = false;
    std::string message;
    int caseId = -1;
};

/*
 * TriageService is the C++/C integration point for Module 3.
 *
 * The C MinHeap (heap.c) is the live, in-memory priority queue: every
 * waiting emergency case is ALSO present in the heap, keyed exactly as
 * heap.c computes it (severity*1e9 + arrival_time). MySQL is the durable
 * record of every case ever created (including ones already treated).
 *
 * On server startup, loadFromDatabase() rebuilds the heap from all
 * currently-"Waiting" cases in MySQL, so the in-memory structure and the
 * database agree even after a restart -- the heap is a cache/index over
 * the "Waiting" subset, not a replacement for the table.
 */
class TriageService {
private:
    EmergencyCaseRepository &repo_;
    MinHeap *heap_;
    std::mutex mutex_; // Crow is multithreaded; the C heap is not internally thread-safe

public:
    explicit TriageService(EmergencyCaseRepository &repo);
    ~TriageService();

    /* Re-populates the heap from every 'Waiting' case in MySQL. Called once at startup. */
    void loadFromDatabase();

    /* Validates input, inserts into MySQL (source of truth) AND the C heap
     * (live priority order), in that order so a heap-only case never exists
     * without a durable DB row. */
    TriageResult admitCase(int patientId, int severity, const std::string &category);

    /* Peeks the most urgent case without removing it from the heap. */
    bool peekNext(EmergencyCase &out);

    /* Extracts (removes) the most urgent case from the heap and marks it
     * 'In Treatment' in MySQL. */
    TriageResult extractNext();

    /* Condition worsens/improves: updates both the heap ordering and MySQL. */
    TriageResult updateSeverity(int caseId, int newSeverity);

    int waitingCount();
};

} // namespace medipriority

#endif // MEDIPRIORITY_TRIAGESERVICE_H
