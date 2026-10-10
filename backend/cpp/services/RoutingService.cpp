#include "RoutingService.h"

namespace medipriority {

RoutingService::RoutingService(HospitalRepository &repo) : repo_(repo) {
    graph_ = graph_create();
}

RoutingService::~RoutingService() {
    graph_destroy(graph_);
}

void RoutingService::loadFromDatabase() {
    std::lock_guard<std::mutex> lock(mutex_);

    hospitalIdToGraphIndex_.clear();
    graphIndexToName_.clear();

    auto hospitals = repo_.findAll();
    for (const auto &h : hospitals) {
        int idx = graph_add_hospital(graph_, h.name().c_str());
        if (idx >= 0) {
            hospitalIdToGraphIndex_[h.hospitalId()] = idx;
            graphIndexToName_[idx] = h.name();
        }
    }

    auto routes = repo_.findAllRoutes();
    for (const auto &[fromId, toId, cost] : routes) {
        auto fromIt = hospitalIdToGraphIndex_.find(fromId);
        auto toIt = hospitalIdToGraphIndex_.find(toId);
        if (fromIt != hospitalIdToGraphIndex_.end() && toIt != hospitalIdToGraphIndex_.end()) {
            graph_add_edge(graph_, fromIt->second, toIt->second, cost);
        }
    }
}

RouteResult RoutingService::findShortestPath(int fromHospitalId, int toHospitalId) {
    RouteResult result;
    std::lock_guard<std::mutex> lock(mutex_);

    auto fromIt = hospitalIdToGraphIndex_.find(fromHospitalId);
    auto toIt = hospitalIdToGraphIndex_.find(toHospitalId);
    if (fromIt == hospitalIdToGraphIndex_.end() || toIt == hospitalIdToGraphIndex_.end()) {
        result.message = "Unknown hospital id";
        return result;
    }

    int path[MAX_HOSPITALS];
    int pathLen = 0;
    double totalCost = 0.0;

    if (!dijkstra_shortest_path(graph_, fromIt->second, toIt->second, path, &pathLen, &totalCost)) {
        result.message = "No route exists between these hospitals in the simulated network "
                          "(destination is unreachable)";
        return result;
    }

    for (int i = 0; i < pathLen; i++) {
        result.hospitalNames.push_back(graphIndexToName_[path[i]]);
    }
    result.totalCost = totalCost;
    result.success = true;
    result.message = "Shortest simulated route found";
    return result;
}

std::vector<std::pair<int, std::string>> RoutingService::listHospitals() {
    std::vector<std::pair<int, std::string>> out;
    auto hospitals = repo_.findAll();
    for (const auto &h : hospitals) out.push_back({h.hospitalId(), h.name()});
    return out;
}

} // namespace medipriority
