#ifndef MEDIPRIORITY_PATIENTREPOSITORY_H
#define MEDIPRIORITY_PATIENTREPOSITORY_H

#include "../utils/Database.h"
#include "../models/Patient.h"
#include <optional>
#include <vector>
#include <string>

namespace medipriority {

struct PatientCreateResult {
    bool success = false;
    int patientId = -1;
    bool duplicateDetected = false; // true if a near-identical record already exists
    std::string message;
};

class PatientRepository {
private:
    Database &db_;

public:
    explicit PatientRepository(Database &db) : db_(db) {}

    std::optional<Patient> findById(int patientId);

    /* Case-insensitive partial name match. */
    std::vector<Patient> searchByName(const std::string &namePart, int limit = 20);

    /* Returns true if a patient with the same name + phone already exists
     * (used to detect likely duplicate registrations before inserting). */
    bool existsSimilar(const std::string &name, const std::string &phone);

    /* Inserts a new patient. If existsSimilar() is true, the caller (service
     * layer) is expected to have already decided whether to proceed --
     * this method does NOT silently overwrite anything; it always inserts
     * a new row (patients are only ever updated via explicit patient_id). */
    PatientCreateResult create(const Patient &patient);

    std::vector<Patient> findAll(int limit = 100);
};

} // namespace medipriority

#endif // MEDIPRIORITY_PATIENTREPOSITORY_H
