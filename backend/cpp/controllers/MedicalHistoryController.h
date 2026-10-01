#ifndef MEDIPRIORITY_MEDICALHISTORYCONTROLLER_H
#define MEDIPRIORITY_MEDICALHISTORYCONTROLLER_H

#include "crow_all.h"
#include "JsonHelpers.h"
#include "AuthMiddleware.h"
#include "../services/MedicalHistoryService.h"
#include "../services/AuthService.h"

namespace medipriority {

template <typename AppT>
inline void registerMedicalHistoryRoutes(AppT &app, MedicalHistoryService &historyService,
                                          AuthService &authService) {

    // POST /api/medical-history  { patient_id, diagnosis, notes }
    CROW_ROUTE(app, "/api/medical-history").methods("POST"_method)
    ([&historyService, &authService](const crow::request &req) {
        if (!authenticate(req, authService)) return errorResponse(401, "Unauthorized");
        auto body = crow::json::load(req.body);
        if (!body) return errorResponse(400, "Invalid JSON body");

        int patientId = body.has("patient_id") ? (int)body["patient_id"].i() : 0;
        std::string diagnosis = body.has("diagnosis") ? std::string(body["diagnosis"].s()) : std::string("");
        std::string notes = body.has("notes") ? std::string(body["notes"].s()) : std::string("");

        auto result = historyService.addEntry(patientId, diagnosis, notes);
        crow::json::wvalue j;
        j["success"] = result.success;
        j["message"] = result.message;
        if (result.success) j["record_id"] = result.recordId;
        return crow::response(result.success ? 201 : 400, j);
    });

    // GET /api/medical-history/{patient_id}
    CROW_ROUTE(app, "/api/medical-history/<int>").methods("GET"_method)
    ([&historyService, &authService](const crow::request &req, int patientId) {
        if (!authenticate(req, authService)) return errorResponse(401, "Unauthorized");
        auto history = historyService.viewPatientHistory(patientId);
        crow::json::wvalue j;
        j["success"] = true;
        std::vector<crow::json::wvalue> arr;
        for (const auto &m : history) arr.push_back(toJson(m));
        j["history"] = std::move(arr);
        return crow::response(200, j);
    });
}

} // namespace medipriority

#endif // MEDIPRIORITY_MEDICALHISTORYCONTROLLER_H
