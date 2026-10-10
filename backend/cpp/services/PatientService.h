#ifndef MEDIPRIORITY_PATIENTSERVICE_H
#define MEDIPRIORITY_PATIENTSERVICE_H

extern "C" {
#include "hashtable.h"
}
#include "../repositories/PatientRepository.h"
#include "../models/Patient.h"
#include <mutex>
#include <optional>
#include <vector>
#include <string>

namespace medipriority {

struct PatientRegisterResult {
    bool success = false;
    std::string message;
    int patientId = -1;
    bool duplicateWarning = false;
};

/*
 * PatientService is the C++/C integration point for Module 1 and 2.
 *
 * The C PatientHashTable (hashtable.c) is a SESSION-LOCAL cache keyed by
 * patient_id, used for fast repeated lookups during a running session
 * (e.g. quickly re-verifying a returning patient's basic details without
 * a round trip to MySQL every time). MySQL remains the single persistent
 * source of truth: on restart, the hash table is empty and gets refilled
 * lazily as patients are looked up or registered, via loadFromDatabase()
 * at startup for the most recent patients.
 */
class PatientService {
private:
    PatientRepository &repo_;
    PatientHashTable *cache_;
    std::mutex mutex_;

public:
    explicit PatientService(PatientRepository &repo);
    ~PatientService();

    void loadFromDatabase(); // warms the cache with recent patients at startup

    PatientRegisterResult registerPatient(const std::string &name, int age,
                                           const std::string &gender, const std::string &phone,
                                           const std::string &bloodGroup,
                                           const std::string &emergencyContact,
                                           const std::string &dateOfBirth);

    /* Fast path: checks the in-memory hash table first; falls back to
     * MySQL on a cache miss (and populates the cache on the way back). */
    std::optional<Patient> findById(int patientId);

    std::vector<Patient> searchByName(const std::string &namePart);
};

} // namespace medipriority

#endif // MEDIPRIORITY_PATIENTSERVICE_H
