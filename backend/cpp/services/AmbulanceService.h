#ifndef MEDIPRIORITY_AMBULANCESERVICE_H
#define MEDIPRIORITY_AMBULANCESERVICE_H

#include "../repositories/AmbulanceRepository.h"
#include <string>

namespace medipriority {

class AmbulanceService {
private:
    AmbulanceRepository &repo_;
public:
    explicit AmbulanceService(AmbulanceRepository &repo) : repo_(repo) {}

    int registerAmbulance(const std::string &vehicleNumber) { return repo_.create(vehicleNumber); }
    std::vector<Ambulance> listAll() { return repo_.findAll(); }
    std::vector<Ambulance> listAvailable() { return repo_.findAvailable(); }
    AmbulanceActionResult assign(int ambulanceId) { return repo_.assign(ambulanceId); }
    AmbulanceActionResult release(int ambulanceId) { return repo_.release(ambulanceId); }
};

} // namespace medipriority
#endif // MEDIPRIORITY_AMBULANCESERVICE_H
