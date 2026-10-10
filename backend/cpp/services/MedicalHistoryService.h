#ifndef MEDIPRIORITY_MEDICALHISTORYSERVICE_H
#define MEDIPRIORITY_MEDICALHISTORYSERVICE_H

extern "C" {
#include "linkedlist.h"
}
#include "../repositories/MedicalHistoryRepository.h"
#include <mutex>
#include <string>
#include <vector>

namespace medipriority {

struct HistoryAddResult {
    bool success = false;
    std::string message;
    int recordId = -1;
};

/*
 * MedicalHistoryService integrates the C singly linked list (linkedlist.c)
 * for Module 9. The list holds entries added during this run in
 * append-only, chronological order; MySQL is queried directly for
 * viewPatientHistory() so that history is complete and correct even for
 * entries added in a previous run (before this in-memory list existed).
 */
class MedicalHistoryService {
private:
    MedicalHistoryRepository &repo_;
    MedicalHistoryList *list_;
    std::mutex mutex_;

public:
    explicit MedicalHistoryService(MedicalHistoryRepository &repo);
    ~MedicalHistoryService();

    HistoryAddResult addEntry(int patientId, const std::string &diagnosis, const std::string &notes);

    /* Always reads the complete, authoritative history from MySQL. */
    std::vector<MedicalRecord> viewPatientHistory(int patientId);
};

} // namespace medipriority

#endif // MEDIPRIORITY_MEDICALHISTORYSERVICE_H
