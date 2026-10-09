#ifndef MEDIPRIORITY_DOCTORSERVICE_H
#define MEDIPRIORITY_DOCTORSERVICE_H

#include "../repositories/DoctorRepository.h"
#include <string>

namespace medipriority {

struct DoctorActionResult {
    bool success = false;
    std::string message;
    int doctorId = -1;
};

class DoctorService {
private:
    DoctorRepository &repo_;
public:
    explicit DoctorService(DoctorRepository &repo) : repo_(repo) {}

    DoctorActionResult addDoctor(const std::string &name, int age, const std::string &gender,
                                  const std::string &specialization, int maxPatients = 5);
    std::vector<Doctor> listAll() { return repo_.findAll(); }
    std::vector<Doctor> listAvailable() { return repo_.findAvailable(); }
    std::optional<Doctor> getById(int id) { return repo_.findById(id); }
    std::vector<DoctorPatientRow> activePatients(int id) { return repo_.findActivePatients(id); }
    bool updateDoctor(const Doctor &doctor) { return repo_.update(doctor); }
    bool deleteDoctor(int id) { return repo_.remove(id); }

    /* Manual duty flag. "Assign" marks a doctor busy/off-duty (the automatic
     * allocator will skip them); "release" marks them free again. Normal
     * patient load is tracked separately via max_patients, so a doctor can
     * hold several patients at once without ever being flagged busy. */
    DoctorActionResult assignDoctor(int doctorId);
    DoctorActionResult releaseDoctor(int doctorId);
};

} // namespace medipriority
#endif // MEDIPRIORITY_DOCTORSERVICE_H
