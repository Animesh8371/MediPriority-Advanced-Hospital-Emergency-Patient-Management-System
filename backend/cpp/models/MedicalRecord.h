#ifndef MEDIPRIORITY_MEDICALRECORD_H
#define MEDIPRIORITY_MEDICALRECORD_H

#include <string>

namespace medipriority {

/* A single medical history entry. This is a SIMULATED, non-authoritative
 * academic record -- not a complete or clinically authoritative medical record. */
class MedicalRecord {
private:
    int recordId_;
    int patientId_;
    std::string diagnosis_;
    std::string notes_;
    long timestamp_;

public:
    MedicalRecord(int recordId, int patientId, std::string diagnosis,
                  std::string notes, long timestamp)
        : recordId_(recordId), patientId_(patientId),
          diagnosis_(std::move(diagnosis)), notes_(std::move(notes)),
          timestamp_(timestamp) {}

    int recordId() const { return recordId_; }
    int patientId() const { return patientId_; }
    const std::string &diagnosis() const { return diagnosis_; }
    const std::string &notes() const { return notes_; }
    long timestamp() const { return timestamp_; }
};

} // namespace medipriority

#endif // MEDIPRIORITY_MEDICALRECORD_H
