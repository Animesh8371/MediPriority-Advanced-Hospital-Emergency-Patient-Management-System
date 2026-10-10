#ifndef MEDIPRIORITY_DOCTORREPOSITORY_H
#define MEDIPRIORITY_DOCTORREPOSITORY_H

#include "../utils/Database.h"
#include "../models/Doctor.h"
#include <optional>
#include <string>
#include <vector>

namespace medipriority {

/* One row of "which patients is this doctor currently handling". */
struct DoctorPatientRow {
    int patientId = 0;
    std::string patientName;
    int bedId = 0;            // 0 if the assignment has no bed
    std::string bedType;      // "" if no bed
    std::string assignedAt;
};

class DoctorRepository {
private:
    Database &db_;
public:
    explicit DoctorRepository(Database &db) : db_(db) {}

    std::optional<Doctor> findById(int doctorId);
    std::vector<Doctor> findAll();
    std::vector<Doctor> findAvailable();

    /* On-duty doctors who are still below their max_patients capacity,
     * ordered least-loaded first (then by name) so callers that take the
     * first match automatically balance the workload. */
    std::vector<Doctor> findAcceptingPatients();

    /* Patients this doctor currently has (active assignments, newest first). */
    std::vector<DoctorPatientRow> findActivePatients(int doctorId);

    int create(const Doctor &doctor);
    bool update(const Doctor &doctor);
    bool remove(int doctorId);
    bool setAvailability(int doctorId, bool available);
};

} // namespace medipriority
#endif // MEDIPRIORITY_DOCTORREPOSITORY_H
