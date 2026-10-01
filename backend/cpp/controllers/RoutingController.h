#ifndef MEDIPRIORITY_ROUTINGCONTROLLER_H
#define MEDIPRIORITY_ROUTINGCONTROLLER_H

#include "crow_all.h"
#include "JsonHelpers.h"
#include "AuthMiddleware.h"
#include "../services/RoutingService.h"
#include "../services/AuthService.h"

namespace medipriority {

template <typename AppT>
inline void registerRoutingRoutes(AppT &app, RoutingService &routingService,
                                   AuthService &authService) {

    CROW_ROUTE(app, "/api/hospitals").methods("GET"_method)
    ([&routingService, &authService](const crow::request &req) {
        if (!authenticate(req, authService)) return errorResponse(401, "Unauthorized");
        auto hospitals = routingService.listHospitals();
        crow::json::wvalue j;
        j["success"] = true;
        std::vector<crow::json::wvalue> arr;
        for (const auto &[id, name] : hospitals) {
            crow::json::wvalue h;
            h["hospital_id"] = id;
            h["name"] = name;
            arr.push_back(std::move(h));
        }
        j["hospitals"] = std::move(arr);
        return crow::response(200, j);
    });

    // POST /api/routing/shortest-path  { from_hospital_id, to_hospital_id }
    CROW_ROUTE(app, "/api/routing/shortest-path").methods("POST"_method)
    ([&routingService, &authService](const crow::request &req) {
        if (!authenticate(req, authService)) return errorResponse(401, "Unauthorized");
        auto body = crow::json::load(req.body);
        if (!body || !body.has("from_hospital_id") || !body.has("to_hospital_id"))
            return errorResponse(400, "from_hospital_id and to_hospital_id are required");

        int fromId = (int)body["from_hospital_id"].i();
        int toId = (int)body["to_hospital_id"].i();

        auto result = routingService.findShortestPath(fromId, toId);
        crow::json::wvalue j;
        j["success"] = result.success;
        j["message"] = result.message;
        if (result.success) {
            std::vector<crow::json::wvalue> path;
            for (const auto &name : result.hospitalNames) path.push_back(name);
            j["route"] = std::move(path);
            j["total_cost"] = result.totalCost;
            j["note"] = "Simulated academic transfer network -- not real-time or medically optimized routing";
        }
        return crow::response(result.success ? 200 : 404, j);
    });
}

} // namespace medipriority

#endif // MEDIPRIORITY_ROUTINGCONTROLLER_H
