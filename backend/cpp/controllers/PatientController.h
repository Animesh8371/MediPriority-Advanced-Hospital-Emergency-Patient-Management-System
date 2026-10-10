#ifndef MEDIPRIORITY_PATIENTCONTROLLER_H
#define MEDIPRIORITY_PATIENTCONTROLLER_H

#include "crow_all.h"
#include "JsonHelpers.h"
#include "AuthMiddleware.h"
#include "../services/PatientService.h"
#include "../services/AuthService.h"

namespace medipriority {

template <typename AppT>
inline void registerPatientRoutes(AppT &app, PatientService &patientService,
                                   AuthService &authService) {

    // POST /api/patients  (register a new patient)
    CROW_ROUTE(app, "/api/patients").methods("POST"_method)
    ([&patientService, &authService](const crow::request &req) {
        if (!authenticate(req, authService)) return errorResponse(401, "Unauthorized");

        auto body = crow::json::load(req.body);
        if (!body) return errorResponse(400, "Invalid JSON body");

        std::string name = body.has("name") ? std::string(body["name"].s()) : std::string("");
        int age = body.has("age") ? (int)body["age"].i() : 0;
        std::string gender = body.has("gender") ? std::string(body["gender"].s()) : std::string("");
        std::string phone = body.has("phone") ? std::string(body["phone"].s()) : std::string("");
        std::string bloodGroup = body.has("blood_group") ? std::string(body["blood_group"].s()) : std::string("Unknown");
        std::string emergencyContact = body.has("emergency_contact") ? std::string(body["emergency_contact"].s()) : std::string("");
        std::string dob = body.has("date_of_birth") ? std::string(body["date_of_birth"].s()) : std::string("");

        auto result = patientService.registerPatient(name, age, gender, phone, bloodGroup,
                                                       emergencyContact, dob);
        crow::json::wvalue j;
        j["success"] = result.success;
        j["message"] = result.message;
        if (result.success) {
            j["patient_id"] = result.patientId;
            j["duplicate_warning"] = result.duplicateWarning;
        }
        return crow::response(result.success ? 201 : 400, j);
    });

    // GET /api/patients/{id}
    CROW_ROUTE(app, "/api/patients/<int>").methods("GET"_method)
    ([&patientService, &authService](const crow::request &req, int id) {
        if (!authenticate(req, authService)) return errorResponse(401, "Unauthorized");

        auto patient = patientService.findById(id);
        if (!patient.has_value()) return errorResponse(404, "Patient not found");

        crow::json::wvalue j;
        j["success"] = true;
        j["patient"] = toJson(patient.value());
        return crow::response(200, j);
    });

    // GET /api/patients/search?name=...
    CROW_ROUTE(app, "/api/patients/search").methods("GET"_method)
    ([&patientService, &authService](const crow::request &req) {
        if (!authenticate(req, authService)) return errorResponse(401, "Unauthorized");

        std::string namePart = req.url_params.get("name") ? req.url_params.get("name") : "";
        if (namePart.empty()) return errorResponse(400, "Query parameter 'name' is required");

        auto results = patientService.searchByName(namePart);
        crow::json::wvalue j;
        j["success"] = true;
        std::vector<crow::json::wvalue> arr;
        for (const auto &p : results) arr.push_back(toJson(p));
        j["patients"] = std::move(arr);
        j["count"] = (int)results.size();
        return crow::response(200, j);
    });
}

} // namespace medipriority

#endif // MEDIPRIORITY_PATIENTCONTROLLER_H
