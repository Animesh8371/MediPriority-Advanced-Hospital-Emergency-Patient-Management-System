#ifndef MEDIPRIORITY_EMERGENCYCONTROLLER_H
#define MEDIPRIORITY_EMERGENCYCONTROLLER_H

#include "crow_all.h"
#include "JsonHelpers.h"
#include "AuthMiddleware.h"
#include "../services/TriageService.h"
#include "../services/AuthService.h"

namespace medipriority {

template <typename AppT>
inline void registerEmergencyRoutes(AppT &app, TriageService &triageService,
                                     AuthService &authService) {

    // POST /api/emergency-cases  { patient_id, severity (1-4), category }
    CROW_ROUTE(app, "/api/emergency-cases").methods("POST"_method)
    ([&triageService, &authService](const crow::request &req) {
        if (!authenticate(req, authService)) return errorResponse(401, "Unauthorized");

        auto body = crow::json::load(req.body);
        if (!body) return errorResponse(400, "Invalid JSON body");

        int patientId = body.has("patient_id") ? (int)body["patient_id"].i() : 0;
        int severity = body.has("severity") ? (int)body["severity"].i() : 0;
        std::string category = body.has("category") ? std::string(body["category"].s()) : std::string("");

        auto result = triageService.admitCase(patientId, severity, category);
        crow::json::wvalue j;
        j["success"] = result.success;
        j["message"] = result.message;
        if (result.success) j["case_id"] = result.caseId;
        return crow::response(result.success ? 201 : 400, j);
    });

    // GET /api/emergency-cases/priority-queue  (peek at the most urgent waiting case)
    CROW_ROUTE(app, "/api/emergency-cases/priority-queue").methods("GET"_method)
    ([&triageService, &authService](const crow::request &req) {
        if (!authenticate(req, authService)) return errorResponse(401, "Unauthorized");

        EmergencyCase next(0, 0, 0, 0, "");
        crow::json::wvalue j;
        j["waiting_count"] = triageService.waitingCount();
        if (triageService.peekNext(next)) {
            j["next_case"]["case_id"] = next.caseId();
            j["next_case"]["patient_id"] = next.patientId();
            j["next_case"]["severity"] = next.severity();
            j["next_case"]["severity_label"] = severityLabel(next.severity());
        } else {
            j["next_case"] = nullptr;
        }
        j["success"] = true;
        return crow::response(200, j);
    });

    // POST /api/triage/prioritize  (extract the most urgent case -> "In Treatment")
    CROW_ROUTE(app, "/api/triage/prioritize").methods("POST"_method)
    ([&triageService, &authService](const crow::request &req) {
        if (!authenticate(req, authService)) return errorResponse(401, "Unauthorized");

        auto result = triageService.extractNext();
        crow::json::wvalue j;
        j["success"] = result.success;
        j["message"] = result.message;
        if (result.success) j["case_id"] = result.caseId;
        return crow::response(result.success ? 200 : 404, j);
    });

    // PUT /api/emergency-cases/{id}/severity  { severity }
    CROW_ROUTE(app, "/api/emergency-cases/<int>/severity").methods("PUT"_method)
    ([&triageService, &authService](const crow::request &req, int caseId) {
        if (!authenticate(req, authService)) return errorResponse(401, "Unauthorized");

        auto body = crow::json::load(req.body);
        if (!body || !body.has("severity")) return errorResponse(400, "severity field required");

        int severity = (int)body["severity"].i();
        auto result = triageService.updateSeverity(caseId, severity);
        crow::json::wvalue j;
        j["success"] = result.success;
        j["message"] = result.message;
        return crow::response(result.success ? 200 : 400, j);
    });
}

} // namespace medipriority

#endif // MEDIPRIORITY_EMERGENCYCONTROLLER_H
