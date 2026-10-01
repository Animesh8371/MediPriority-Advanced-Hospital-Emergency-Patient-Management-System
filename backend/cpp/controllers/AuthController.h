#ifndef MEDIPRIORITY_AUTHCONTROLLER_H
#define MEDIPRIORITY_AUTHCONTROLLER_H

#include "crow_all.h"
#include "JsonHelpers.h"
#include "../services/AuthService.h"

namespace medipriority {

template <typename AppT>
inline void registerAuthRoutes(AppT &app, AuthService &authService) {

    // POST /api/auth/register  { username, password, role }
    CROW_ROUTE(app, "/api/auth/register").methods("POST"_method)
    ([&authService](const crow::request &req) {
        auto body = crow::json::load(req.body);
        if (!body) return errorResponse(400, "Invalid JSON body");

        std::string username = body.has("username") ? std::string(body["username"].s()) : std::string("");
        std::string password = body.has("password") ? std::string(body["password"].s()) : std::string("");
        std::string role = body.has("role") ? std::string(body["role"].s()) : std::string("staff");

        auto result = authService.registerUser(username, password, role);
        crow::json::wvalue j;
        j["success"] = result.success;
        j["message"] = result.message;
        if (result.success) j["user_id"] = result.userId;
        return crow::response(result.success ? 201 : 400, j);
    });

    // POST /api/auth/login  { username, password }
    CROW_ROUTE(app, "/api/auth/login").methods("POST"_method)
    ([&authService](const crow::request &req) {
        auto body = crow::json::load(req.body);
        if (!body) return errorResponse(400, "Invalid JSON body");

        std::string username = body.has("username") ? std::string(body["username"].s()) : std::string("");
        std::string password = body.has("password") ? std::string(body["password"].s()) : std::string("");

        auto result = authService.login(username, password);
        crow::json::wvalue j;
        j["success"] = result.success;
        j["message"] = result.message;
        if (result.success) {
            j["token"] = result.token;
            j["user_id"] = result.userId;
            j["role"] = result.role;
        }
        return crow::response(result.success ? 200 : 401, j);
    });

    // POST /api/auth/logout  (Authorization: Bearer <token>)
    CROW_ROUTE(app, "/api/auth/logout").methods("POST"_method)
    ([&authService](const crow::request &req) {
        std::string header = req.get_header_value("Authorization");
        const std::string prefix = "Bearer ";
        if (header.size() > prefix.size()) {
            authService.logout(header.substr(prefix.size()));
        }
        crow::json::wvalue j;
        j["success"] = true;
        j["message"] = "Logged out";
        return crow::response(200, j);
    });
}

} // namespace medipriority

#endif // MEDIPRIORITY_AUTHCONTROLLER_H
