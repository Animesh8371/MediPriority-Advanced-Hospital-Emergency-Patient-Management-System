#ifndef MEDIPRIORITY_HOSPITALREPOSITORY_H
#define MEDIPRIORITY_HOSPITALREPOSITORY_H

#include "../utils/Database.h"
#include "../models/Hospital.h"
#include <vector>
#include <tuple>

namespace medipriority {

class HospitalRepository {
private:
    Database &db_;
public:
    explicit HospitalRepository(Database &db) : db_(db) {}

    std::vector<Hospital> findAll();

    /* Returns all routes as (from_hospital_id, to_hospital_id, distance_cost)
     * so the service layer can load them straight into the C graph module. */
    std::vector<std::tuple<int,int,double>> findAllRoutes();
};

} // namespace medipriority
#endif // MEDIPRIORITY_HOSPITALREPOSITORY_H
