#ifndef MEDIPRIORITY_MEDICALHISTORYREPOSITORY_H
#define MEDIPRIORITY_MEDICALHISTORYREPOSITORY_H

#include "../utils/Database.h"
#include "../models/MedicalRecord.h"
#include <vector>

namespace medipriority {

class MedicalHistoryRepository {
private:
    Database &db_;
public:
    explicit MedicalHistoryRepository(Database &db) : db_(db) {}

    int create(const MedicalRecord &record);

    /* Chronological (oldest -> newest) history for one patient. */
    std::vector<MedicalRecord> findByPatient(int patientId);
};

} // namespace medipriority
#endif // MEDIPRIORITY_MEDICALHISTORYREPOSITORY_H
