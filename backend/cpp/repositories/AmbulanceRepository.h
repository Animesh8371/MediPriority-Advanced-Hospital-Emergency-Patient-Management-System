#ifndef MEDIPRIORITY_AMBULANCEREPOSITORY_H
#define MEDIPRIORITY_AMBULANCEREPOSITORY_H

#include "../utils/Database.h"
#include "../models/Ambulance.h"
#include <optional>
#include <vector>

namespace medipriority {

struct AmbulanceActionResult {
    bool success = false;
    std::string message;
};

class AmbulanceRepository {
private:
    Database &db_;
public:
    explicit AmbulanceRepository(Database &db) : db_(db) {}

    std::optional<Ambulance> findById(int ambulanceId);
    std::vector<Ambulance> findAll();
    std::vector<Ambulance> findAvailable();
    int create(const std::string &vehicleNumber);
    AmbulanceActionResult assign(int ambulanceId);
    AmbulanceActionResult release(int ambulanceId);
};

} // namespace medipriority
#endif // MEDIPRIORITY_AMBULANCEREPOSITORY_H
