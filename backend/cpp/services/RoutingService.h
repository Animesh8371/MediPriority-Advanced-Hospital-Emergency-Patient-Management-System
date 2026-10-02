#ifndef MEDIPRIORITY_ROUTINGSERVICE_H
#define MEDIPRIORITY_ROUTINGSERVICE_H

extern "C" {
#include "graph.h"
}
#include "../repositories/HospitalRepository.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>

namespace medipriority {

struct RouteResult {
    bool success = false;
    std::string message;
    std::vector<std::string> hospitalNames; // in path order, source first
    double totalCost = 0.0;
};

/*
 * RoutingService integrates the C graph + Dijkstra module (graph.c,
 * dijkstra.c) for Module 7: a SIMULATED hospital transfer network. This
 * is NOT real-time or medically-optimized routing, and is not connected
 * to any live mapping service -- it operates purely on the simulated
 * hospitals/hospital_routes tables in MySQL.
 */
class RoutingService {
private:
    HospitalRepository &repo_;
    HospitalGraph *graph_;
    std::unordered_map<int, int> hospitalIdToGraphIndex_;
    std::unordered_map<int, std::string> graphIndexToName_;
    std::mutex mutex_;

public:
    explicit RoutingService(HospitalRepository &repo);
    ~RoutingService();

    /* Loads all hospitals and routes from MySQL into the C graph. Called
     * once at startup (the simulated network is treated as mostly static). */
    void loadFromDatabase();

    RouteResult findShortestPath(int fromHospitalId, int toHospitalId);
    std::vector<std::pair<int, std::string>> listHospitals(); // (hospital_id, name)
};

} // namespace medipriority

#endif // MEDIPRIORITY_ROUTINGSERVICE_H
