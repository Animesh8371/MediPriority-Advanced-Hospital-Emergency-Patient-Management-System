#ifndef MEDIPRIORITY_EMERGENCYCASE_H
#define MEDIPRIORITY_EMERGENCYCASE_H

#include <string>

namespace medipriority {

/* Severity scale matches the C heap module exactly:
 * 1 = Critical, 2 = High, 3 = Moderate, 4 = Low.
 * This is a SIMULATED academic severity score, not a real clinical triage value. */
enum class Severity { Critical = 1, High = 2, Moderate = 3, Low = 4 };

inline std::string severityLabel(int severity) {
    switch (severity) {
        case 1: return "Critical";
        case 2: return "High";
        case 3: return "Moderate";
        case 4: return "Low";
        default: return "Unknown";
    }
}

class EmergencyCase {
private:
    int caseId_;
    int patientId_;
    int severity_;       // 1-4, see Severity enum
    long arrivalTime_;    // unix-like timestamp
    std::string category_;      // e.g. "Trauma", "Cardiac", "Respiratory" (simulated)
    int assignedDoctorId_;      // 0 = unassigned
    int assignedBedId_;         // 0 = unassigned
    int assignedAmbulanceId_;   // 0 = unassigned
    std::string status_;        // "Waiting", "In Treatment", "Discharged"

public:
    EmergencyCase(int caseId, int patientId, int severity, long arrivalTime,
                  std::string category, std::string status = "Waiting")
        : caseId_(caseId), patientId_(patientId), severity_(severity),
          arrivalTime_(arrivalTime), category_(std::move(category)),
          assignedDoctorId_(0), assignedBedId_(0), assignedAmbulanceId_(0),
          status_(std::move(status)) {}

    int caseId() const { return caseId_; }
    int patientId() const { return patientId_; }
    int severity() const { return severity_; }
    long arrivalTime() const { return arrivalTime_; }
    const std::string &category() const { return category_; }
    int assignedDoctorId() const { return assignedDoctorId_; }
    int assignedBedId() const { return assignedBedId_; }
    int assignedAmbulanceId() const { return assignedAmbulanceId_; }
    const std::string &status() const { return status_; }

    void assignDoctor(int doctorId) { assignedDoctorId_ = doctorId; }
    void assignBed(int bedId) { assignedBedId_ = bedId; }
    void assignAmbulance(int ambulanceId) { assignedAmbulanceId_ = ambulanceId; }
    void setStatus(const std::string &status) { status_ = status; }
    void setSeverity(int severity) { severity_ = severity; }
};

} // namespace medipriority

#endif // MEDIPRIORITY_EMERGENCYCASE_H
