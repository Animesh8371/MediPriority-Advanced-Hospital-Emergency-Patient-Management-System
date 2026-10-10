#ifndef MEDIPRIORITY_DOCTORCONTROLLER_H
#define MEDIPRIORITY_DOCTORCONTROLLER_H

#include "crow_all.h"
#include "JsonHelpers.h"
#include "AuthMiddleware.h"
#include "../services/DoctorService.h"
#include "../services/AuthService.h"

namespace medipriority {

template <typename AppT>
inline void registerDoctorRoutes(AppT &app, DoctorService &doctorService,
                                  AuthService &authService) {

    CROW_ROUTE(app, "/api/doctors").methods("GET"_method)
    ([&doctorService, &authService](const crow::request &req) {
        if (!authenticate(req, authService)) return errorResponse(401, "Unauthorized");
        auto doctors = doctorService.listAll();
        crow::json::wvalue j;
        j["success"] = true;
        std::vector<crow::json::wvalue> arr;
        for (const auto &d : doctors) arr.push_back(toJson(d));
        j["doctors"] = std::move(arr);
        return crow::response(200, j);
    });

    CROW_ROUTE(app, "/api/doctors").methods("POST"_method)
    ([&doctorService, &authService](const crow::request &req) {
        if (!authenticate(req, authService)) return errorResponse(401, "Unauthorized");
        auto body = crow::json::load(req.body);
        if (!body) return errorResponse(400, "Invalid JSON body");

        std::string name = body.has("name") ? std::string(body["name"].s()) : std::string("");
        int age = body.has("age") ? (int)body["age"].i() : 0;
        std::string gender = body.has("gender") ? std::string(body["gender"].s()) : std::string("");
        std::string specialization = body.has("specialization") ? std::string(body["specialization"].s()) : std::string("");

        int maxPatients = body.has("max_patients") ? (int)body["max_patients"].i() : 5;

        auto result = doctorService.addDoctor(name, age, gender, specialization, maxPatients);
        crow::json::wvalue j;
        j["success"] = result.success;
        j["message"] = result.message;
        if (result.success) j["doctor_id"] = result.doctorId;
        return crow::response(result.success ? 201 : 400, j);
    });

    // GET /api/doctors/{id}/patients -> every patient this doctor currently handles
    CROW_ROUTE(app, "/api/doctors/<int>/patients").methods("GET"_method)
    ([&doctorService, &authService](const crow::request &req, int id) {
        if (!authenticate(req, authService)) return errorResponse(401, "Unauthorized");
        auto doctor = doctorService.getById(id);
        if (!doctor.has_value()) return errorResponse(404, "Doctor not found");

        crow::json::wvalue j;
        j["success"] = true;
        j["doctor"] = toJson(doctor.value());
        std::vector<crow::json::wvalue> arr;
        for (const auto &row : doctorService.activePatients(id)) {
            crow::json::wvalue r;
            r["patient_id"] = row.patientId;
            r["patient_name"] = row.patientName;
            r["bed_id"] = row.bedId;
            r["bed_type"] = row.bedType;
            r["assigned_at"] = row.assignedAt;
            arr.push_back(std::move(r));
        }
        j["patients"] = std::move(arr);
        return crow::response(200, j);
    });

    CROW_ROUTE(app, "/api/doctors/<int>/assign").methods("POST"_method)
    ([&doctorService, &authService](const crow::request &req, int id) {
        if (!authenticate(req, authService)) return errorResponse(401, "Unauthorized");
        auto result = doctorService.assignDoctor(id);
        crow::json::wvalue j;
        j["success"] = result.success;
        j["message"] = result.message;
        return crow::response(result.success ? 200 : 400, j);
    });

    CROW_ROUTE(app, "/api/doctors/<int>/release").methods("POST"_method)
    ([&doctorService, &authService](const crow::request &req, int id) {
        if (!authenticate(req, authService)) return errorResponse(401, "Unauthorized");
        auto result = doctorService.releaseDoctor(id);
        crow::json::wvalue j;
        j["success"] = result.success;
        j["message"] = result.message;
        return crow::response(result.success ? 200 : 400, j);
    });
}

} // namespace medipriority

#endif // MEDIPRIORITY_DOCTORCONTROLLER_H
