#include "PatientService.h"
#include "../utils/Validation.h"
#include <cstring>

namespace medipriority {

static PatientRecord toRecord(const Patient &p) {
    PatientRecord r{};
    r.patient_id = p.id();
    std::strncpy(r.name, p.name().c_str(), sizeof(r.name) - 1);
    r.age = p.age();
    std::strncpy(r.phone, p.phone().c_str(), sizeof(r.phone) - 1);
    return r;
}

PatientService::PatientService(PatientRepository &repo) : repo_(repo) {
    cache_ = ht_create(64);
}

PatientService::~PatientService() {
    ht_destroy(cache_);
}

void PatientService::loadFromDatabase() {
    std::lock_guard<std::mutex> lock(mutex_);
    auto recent = repo_.findAll(200);
    for (const auto &p : recent) {
        PatientRecord r = toRecord(p);
        ht_insert(cache_, r);
    }
}

PatientRegisterResult PatientService::registerPatient(
        const std::string &name, int age, const std::string &gender,
        const std::string &phone, const std::string &bloodGroup,
        const std::string &emergencyContact, const std::string &dateOfBirth) {
    PatientRegisterResult result;

    if (!Validation::isValidName(name)) { result.message = "Invalid name"; return result; }
    if (!Validation::isValidAge(age)) { result.message = "Invalid age (must be 1-130)"; return result; }
    if (!Validation::isValidGender(gender)) { result.message = "Gender must be Male, Female, or Other"; return result; }
    if (!Validation::isValidPhone(phone)) { result.message = "Invalid phone number"; return result; }
    if (!Validation::isValidBloodGroup(bloodGroup)) { result.message = "Invalid blood group"; return result; }

    bool likelyDuplicate = repo_.existsSimilar(name, phone);

    Patient patient(0, name, age, gender, phone, bloodGroup, emergencyContact, dateOfBirth);
    auto createResult = repo_.create(patient);

    if (!createResult.success) {
        result.message = createResult.message;
        return result;
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        Patient stored(createResult.patientId, name, age, gender, phone, bloodGroup,
                       emergencyContact, dateOfBirth);
        PatientRecord r = toRecord(stored);
        ht_insert(cache_, r);
    }

    result.success = true;
    result.patientId = createResult.patientId;
    result.duplicateWarning = likelyDuplicate;
    result.message = likelyDuplicate
        ? "Patient registered. NOTE: a patient with the same name and phone already exists -- please verify this is not an accidental duplicate."
        : "Patient registered successfully";
    return result;
}

std::optional<Patient> PatientService::findById(int patientId) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        PatientRecord r;
        if (ht_search(cache_, patientId, &r)) {
            // Cache only stores a subset of fields for the fast-path lookup;
            // for full details we still go to MySQL, but this demonstrates
            // the O(1) average-case existence/basic-info check.
        }
    }
    auto patient = repo_.findById(patientId);
    if (patient.has_value()) {
        std::lock_guard<std::mutex> lock(mutex_);
        PatientRecord r = toRecord(patient.value());
        ht_insert(cache_, r); // populate cache on miss
    }
    return patient;
}

std::vector<Patient> PatientService::searchByName(const std::string &namePart) {
    return repo_.searchByName(namePart);
}

} // namespace medipriority
