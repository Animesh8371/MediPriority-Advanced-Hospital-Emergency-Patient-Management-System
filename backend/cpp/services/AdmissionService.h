#ifndef MEDIPRIORITY_ADMISSIONSERVICE_H
#define MEDIPRIORITY_ADMISSIONSERVICE_H

#include "PatientService.h"
#include "TriageService.h"
#include "../repositories/BedRepository.h"
#include "../repositories/DoctorRepository.h"
#include "../repositories/EmergencyCaseRepository.h"
#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace medipriority {

struct AdmissionRequest {
    std::string name;
    int age = 0;
    std::string gender;
    std::string phone;
    std::string bloodGroup = "Unknown";
    std::string emergencyContact;
    std::string dateOfBirth;
    int severity = 0;          // 1 Critical .. 4 Low
    std::string category;      // e.g. "Cardiac", "Trauma"
};

struct AdmissionResult {
    bool success = false;
    std::string message;

    int patientId = -1;
    std::string patientCode;   // human-friendly form of the primary key, e.g. MP-000042
    bool duplicateWarning = false;

    int caseId = -1;
    int severity = 0;

    bool bedAllocated = false;
    int bedId = 0;
    std::string bedType;

    bool doctorAssigned = false;
    int doctorId = 0;
    std::string doctorName;
    std::string doctorSpecialization;
    int doctorLoad = 0;        // patients the doctor holds AFTER this admission
    int doctorCapacity = 0;

    bool waitlisted = false;   // true when no suitable bed was free
};

/* What the allocator WOULD do right now, without changing anything.
 * Powers the live "allocation preview" panel on the registration page. */
struct AllocationPreview {
    std::vector<std::string> bedPreference;   // wards tried in order
    std::string targetBedType;                // first ward with a free bed ("" if none)
    int freeBedsInTarget = 0;
    bool doctorFound = false;
    int doctorId = 0;
    std::string doctorName;
    std::string doctorSpecialization;
    int doctorLoad = 0;
    int doctorCapacity = 0;
};

/*
 * AdmissionService is the "advanced registration" workflow:
 *
 *   register patient (gets the unique patient_id primary key)
 *     -> create the emergency case (goes into the C min-heap priority queue)
 *     -> choose a ward from the severity (Critical -> ICU, ...)
 *     -> choose a doctor (matching specialty, least-loaded, below capacity)
 *     -> allocate the bed inside the existing transactional BedRepository
 *
 * If every suitable ward is full the patient is still registered and queued,
 * but flagged `waitlisted`; allocateForCase() can be retried later once a bed
 * has been released.
 *
 * A mutex serialises whole admissions. The Database wrapper owns a single
 * connection, so two admissions interleaving their SELECT-then-allocate steps
 * could otherwise pick the same doctor/bed candidates at the same instant.
 */
class AdmissionService {
private:
    PatientService &patients_;
    TriageService &triage_;
    BedRepository &beds_;
    DoctorRepository &doctors_;
    EmergencyCaseRepository &cases_;
    std::mutex mutex_;

    std::optional<Doctor> pickDoctor(const std::string &category);
    void allocate(AdmissionResult &result, const std::string &category);

public:
    AdmissionService(PatientService &patients, TriageService &triage, BedRepository &beds,
                     DoctorRepository &doctors, EmergencyCaseRepository &cases)
        : patients_(patients), triage_(triage), beds_(beds), doctors_(doctors), cases_(cases) {}

    /* Wards to try, best first, for a severity (1..4). */
    static std::vector<std::string> bedPreference(int severity);

    /* Specializations to prefer, best first, for an emergency category. */
    static std::vector<std::string> specialtyPreference(const std::string &category);

    static std::string patientCode(int patientId);

    AdmissionResult admitNewPatient(const AdmissionRequest &req);

    /* Retry bed + doctor allocation for an already-registered, waitlisted case. */
    AdmissionResult allocateForCase(int caseId);

    AllocationPreview preview(int severity, const std::string &category);
};

} // namespace medipriority

#endif // MEDIPRIORITY_ADMISSIONSERVICE_H
