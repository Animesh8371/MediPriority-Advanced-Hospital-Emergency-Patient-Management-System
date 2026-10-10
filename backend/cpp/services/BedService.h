#ifndef MEDIPRIORITY_BEDSERVICE_H
#define MEDIPRIORITY_BEDSERVICE_H

#include "../repositories/BedRepository.h"
#include <string>

namespace medipriority {

class BedService {
private:
    BedRepository &repo_;
public:
    explicit BedService(BedRepository &repo) : repo_(repo) {}

    int addBed(const std::string &bedType) { return repo_.create(bedType); }
    std::vector<Bed> listAll() { return repo_.findAll(); }
    std::vector<Bed> listAvailable(const std::string &bedType) { return repo_.findAvailable(bedType); }

    BedActionResult allocateBed(int bedId, int patientId, int doctorId) {
        return repo_.allocate(bedId, patientId, doctorId);
    }
    BedActionResult releaseBed(int bedId) { return repo_.release(bedId); }
};

} // namespace medipriority
#endif // MEDIPRIORITY_BEDSERVICE_H
