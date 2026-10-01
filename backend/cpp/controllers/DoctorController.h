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

        auto result = doctorService.addDoctor(name, age, gender, specialization);
        crow::json::wvalue j;
        j["success"] = result.success;
        j["message"] = result.message;
        if (result.success) j["doctor_id"] = result.doctorId;
        return crow::response(result.success ? 201 : 400, j);
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
