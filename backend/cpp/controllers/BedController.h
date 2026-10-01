#ifndef MEDIPRIORITY_BEDCONTROLLER_H
#define MEDIPRIORITY_BEDCONTROLLER_H

#include "crow_all.h"
#include "JsonHelpers.h"
#include "AuthMiddleware.h"
#include "../services/BedService.h"
#include "../services/AuthService.h"

namespace medipriority {

template <typename AppT>
inline void registerBedRoutes(AppT &app, BedService &bedService, AuthService &authService) {

    CROW_ROUTE(app, "/api/beds").methods("GET"_method)
    ([&bedService, &authService](const crow::request &req) {
        if (!authenticate(req, authService)) return errorResponse(401, "Unauthorized");
        auto beds = bedService.listAll();
        crow::json::wvalue j;
        j["success"] = true;
        std::vector<crow::json::wvalue> arr;
        for (const auto &b : beds) arr.push_back(toJson(b));
        j["beds"] = std::move(arr);
        return crow::response(200, j);
    });

    CROW_ROUTE(app, "/api/beds").methods("POST"_method)
    ([&bedService, &authService](const crow::request &req) {
        if (!authenticate(req, authService)) return errorResponse(401, "Unauthorized");
        auto body = crow::json::load(req.body);
        if (!body || !body.has("bed_type")) return errorResponse(400, "bed_type field required");
        int id = bedService.addBed(body["bed_type"].s());
        crow::json::wvalue j;
        j["success"] = true;
        j["bed_id"] = id;
        return crow::response(201, j);
    });

    CROW_ROUTE(app, "/api/beds/allocate").methods("POST"_method)
    ([&bedService, &authService](const crow::request &req) {
        if (!authenticate(req, authService)) return errorResponse(401, "Unauthorized");
        auto body = crow::json::load(req.body);
        if (!body || !body.has("bed_id") || !body.has("patient_id"))
            return errorResponse(400, "bed_id and patient_id are required");

        int bedId = (int)body["bed_id"].i();
        int patientId = (int)body["patient_id"].i();
        int doctorId = body.has("doctor_id") ? (int)body["doctor_id"].i() : 0;

        auto result = bedService.allocateBed(bedId, patientId, doctorId);
        crow::json::wvalue j;
        j["success"] = result.success;
        j["message"] = result.message;
        return crow::response(result.success ? 200 : 409, j);
    });

    CROW_ROUTE(app, "/api/beds/release").methods("POST"_method)
    ([&bedService, &authService](const crow::request &req) {
        if (!authenticate(req, authService)) return errorResponse(401, "Unauthorized");
        auto body = crow::json::load(req.body);
        if (!body || !body.has("bed_id")) return errorResponse(400, "bed_id required");

        auto result = bedService.releaseBed((int)body["bed_id"].i());
        crow::json::wvalue j;
        j["success"] = result.success;
        j["message"] = result.message;
        return crow::response(result.success ? 200 : 400, j);
    });
}

} // namespace medipriority

#endif // MEDIPRIORITY_BEDCONTROLLER_H
