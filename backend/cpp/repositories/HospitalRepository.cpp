#include "HospitalRepository.h"
#include <cppconn/resultset.h>
#include <memory>

namespace medipriority {

std::vector<Hospital> HospitalRepository::findAll() {
    auto stmt = db_.prepare("SELECT hospital_id, name, location FROM hospitals WHERE in_route_network = TRUE ORDER BY hospital_id");
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    std::vector<Hospital> out;
    while (rs->next()) {
        out.emplace_back(rs->getInt("hospital_id"), rs->getString("name"),
                          rs->isNull("location") ? "" : rs->getString("location"));
    }
    return out;
}

std::vector<std::tuple<int,int,double>> HospitalRepository::findAllRoutes() {
    auto stmt = db_.prepare("SELECT from_hospital_id, to_hospital_id, distance_cost FROM hospital_routes");
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    std::vector<std::tuple<int,int,double>> out;
    while (rs->next()) {
        out.emplace_back(rs->getInt("from_hospital_id"), rs->getInt("to_hospital_id"),
                          rs->getDouble("distance_cost"));
    }
    return out;
}

} // namespace medipriority
