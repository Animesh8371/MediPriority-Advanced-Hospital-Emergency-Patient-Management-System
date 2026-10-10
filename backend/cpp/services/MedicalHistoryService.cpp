#include "MedicalHistoryService.h"
#include <cstring>
#include <ctime>

namespace medipriority {

MedicalHistoryService::MedicalHistoryService(MedicalHistoryRepository &repo) : repo_(repo) {
    list_ = mhlist_create();
}

MedicalHistoryService::~MedicalHistoryService() {
    mhlist_destroy(list_);
}

HistoryAddResult MedicalHistoryService::addEntry(int patientId, const std::string &diagnosis,
                                                  const std::string &notes) {
    HistoryAddResult result;

    if (diagnosis.empty()) {
        result.message = "Diagnosis is required";
        return result;
    }

    long timestamp = static_cast<long>(std::time(nullptr));
    MedicalRecord record(0, patientId, diagnosis, notes, timestamp);
    int recordId = repo_.create(record);

    {
        std::lock_guard<std::mutex> lock(mutex_);
        ::MedicalHistoryEntry entry{};
        entry.record_id = recordId;
        entry.patient_id = patientId;
        std::strncpy(entry.diagnosis, diagnosis.c_str(), sizeof(entry.diagnosis) - 1);
        std::strncpy(entry.notes, notes.c_str(), sizeof(entry.notes) - 1);
        entry.timestamp = timestamp;
        mhlist_append(list_, entry); // preserves prior entries; never overwrites
    }

    result.success = true;
    result.recordId = recordId;
    result.message = "Medical history entry added";
    return result;
}

std::vector<MedicalRecord> MedicalHistoryService::viewPatientHistory(int patientId) {
    return repo_.findByPatient(patientId);
}

} // namespace medipriority
