#include "DoctorService.h"
#include "../utils/Validation.h"

namespace medipriority {

DoctorActionResult DoctorService::addDoctor(const std::string &name, int age,
                                             const std::string &gender,
                                             const std::string &specialization,
                                             int maxPatients) {
    DoctorActionResult result;
    if (!Validation::isValidName(name)) { result.message = "Invalid name"; return result; }
    if (!Validation::isValidAge(age)) { result.message = "Invalid age"; return result; }
    if (!Validation::isValidGender(gender)) { result.message = "Invalid gender"; return result; }
    if (specialization.empty()) { result.message = "Specialization required"; return result; }
    if (maxPatients < 1 || maxPatients > 50) { result.message = "max_patients must be between 1 and 50"; return result; }

    Doctor doctor(0, name, age, gender, specialization, true, maxPatients, 0);
    result.doctorId = repo_.create(doctor);
    result.success = true;
    result.message = "Doctor added successfully";
    return result;
}

DoctorActionResult DoctorService::assignDoctor(int doctorId) {
    DoctorActionResult result;
    auto doctorOpt = repo_.findById(doctorId);
    if (!doctorOpt.has_value()) { result.message = "Doctor not found"; return result; }
    if (!doctorOpt->isAvailable()) { result.message = "Doctor is not available"; return result; }

    repo_.setAvailability(doctorId, false);
    result.success = true;
    result.doctorId = doctorId;
    result.message = "Doctor assigned";
    return result;
}

DoctorActionResult DoctorService::releaseDoctor(int doctorId) {
    DoctorActionResult result;
    if (!repo_.setAvailability(doctorId, true)) { result.message = "Doctor not found"; return result; }
    result.success = true;
    result.doctorId = doctorId;
    result.message = "Doctor released and marked available";
    return result;
}

} // namespace medipriority
