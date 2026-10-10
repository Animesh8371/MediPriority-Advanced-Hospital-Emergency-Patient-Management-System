#ifndef MEDIPRIORITY_AMBULANCECONTROLLER_H
#define MEDIPRIORITY_AMBULANCECONTROLLER_H

#include "crow_all.h"
#include "JsonHelpers.h"
#include "AuthMiddleware.h"
#include "../services/AmbulanceService.h"
#include "../services/AuthService.h"

namespace medipriority {

template <typename AppT>
inline void registerAmbulanceRoutes(AppT &app, AmbulanceService &ambulanceService,
                                     AuthService &authService) {

    CROW_ROUTE(app, "/api/ambulances").methods("GET"_method)
    ([&ambulanceService, &authService](const crow::request &req) {
        if (!authenticate(req, authService)) return errorResponse(401, "Unauthorized");
        auto list = ambulanceService.listAll();
        crow::json::wvalue j;
        j["success"] = true;
        std::vector<crow::json::wvalue> arr;
        for (const auto &a : list) arr.push_back(toJson(a));
        j["ambulances"] = std::move(arr);
        return crow::response(200, j);
    });

    CROW_ROUTE(app, "/api/ambulances").methods("POST"_method)
    ([&ambulanceService, &authService](const crow::request &req) {
        if (!authenticate(req, authService)) return errorResponse(401, "Unauthorized");
        auto body = crow::json::load(req.body);
        if (!body || !body.has("vehicle_number")) return errorResponse(400, "vehicle_number required");
        int id = ambulanceService.registerAmbulance(body["vehicle_number"].s());
        crow::json::wvalue j;
        j["success"] = true;
        j["ambulance_id"] = id;
        return crow::response(201, j);
    });

    CROW_ROUTE(app, "/api/ambulances/assign").methods("POST"_method)
    ([&ambulanceService, &authService](const crow::request &req) {
        if (!authenticate(req, authService)) return errorResponse(401, "Unauthorized");
        auto body = crow::json::load(req.body);
        if (!body || !body.has("ambulance_id")) return errorResponse(400, "ambulance_id required");
        auto result = ambulanceService.assign((int)body["ambulance_id"].i());
        crow::json::wvalue j;
        j["success"] = result.success;
        j["message"] = result.message;
        return crow::response(result.success ? 200 : 409, j);
    });

    CROW_ROUTE(app, "/api/ambulances/release").methods("POST"_method)
    ([&ambulanceService, &authService](const crow::request &req) {
        if (!authenticate(req, authService)) return errorResponse(401, "Unauthorized");
        auto body = crow::json::load(req.body);
        if (!body || !body.has("ambulance_id")) return errorResponse(400, "ambulance_id required");
        auto result = ambulanceService.release((int)body["ambulance_id"].i());
        crow::json::wvalue j;
        j["success"] = result.success;
        j["message"] = result.message;
        return crow::response(result.success ? 200 : 400, j);
    });
}

} // namespace medipriority

#endif // MEDIPRIORITY_AMBULANCECONTROLLER_H
