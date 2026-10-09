#ifndef MEDIPRIORITY_APPOINTMENTCONTROLLER_H
#define MEDIPRIORITY_APPOINTMENTCONTROLLER_H

#include "crow_all.h"
#include "JsonHelpers.h"
#include "AuthMiddleware.h"
#include "../services/AppointmentService.h"
#include "../services/AuthService.h"

namespace medipriority {

template <typename AppT>
inline void registerAppointmentRoutes(AppT &app, AppointmentService &apptService,
                                       AuthService &authService) {

    CROW_ROUTE(app, "/api/appointments").methods("GET"_method)
    ([&apptService, &authService](const crow::request &req) {
        if (!authenticate(req, authService)) return errorResponse(401, "Unauthorized");
        auto list = apptService.listUpcoming();
        crow::json::wvalue j;
        j["success"] = true;
        std::vector<crow::json::wvalue> arr;
        for (const auto &a : list) arr.push_back(toJson(a));
        j["appointments"] = std::move(arr);
        return crow::response(200, j);
    });

    // POST /api/appointments  { patient_id, doctor_id, scheduled_time }
    CROW_ROUTE(app, "/api/appointments").methods("POST"_method)
    ([&apptService, &authService](const crow::request &req) {
        if (!authenticate(req, authService)) return errorResponse(401, "Unauthorized");
        auto body = crow::json::load(req.body);
        if (!body) return errorResponse(400, "Invalid JSON body");

        int patientId = body.has("patient_id") ? (int)body["patient_id"].i() : 0;
        int doctorId = body.has("doctor_id") ? (int)body["doctor_id"].i() : 0;
        long scheduledTime = body.has("scheduled_time") ? (long)body["scheduled_time"].i() : 0;

        auto result = apptService.bookAppointment(patientId, doctorId, scheduledTime);
        crow::json::wvalue j;
        j["success"] = result.success;
        j["message"] = result.message;
        if (result.success) j["appointment_id"] = result.appointmentId;
        return crow::response(result.success ? 201 : 409, j);
    });

    // POST /api/appointments/call-next  (dequeue the next appointment in FIFO order)
    CROW_ROUTE(app, "/api/appointments/call-next").methods("POST"_method)
    ([&apptService, &authService](const crow::request &req) {
        if (!authenticate(req, authService)) return errorResponse(401, "Unauthorized");
        Appointment next(0, 0, 0, 0);
        crow::json::wvalue j;
        if (apptService.callNext(next)) {
            j["success"] = true;
            j["appointment"] = toJson(next);
        } else {
            j["success"] = false;
            j["message"] = "No appointments waiting in the queue";
        }
        return crow::response(200, j);
    });

    CROW_ROUTE(app, "/api/appointments/<int>/cancel").methods("POST"_method)
    ([&apptService, &authService](const crow::request &req, int id) {
        if (!authenticate(req, authService)) return errorResponse(401, "Unauthorized");
        bool ok = apptService.cancelAppointment(id);
        crow::json::wvalue j;
        j["success"] = ok;
        j["message"] = ok ? "Appointment cancelled" : "Appointment not found";
        return crow::response(ok ? 200 : 404, j);
    });
}

} // namespace medipriority

#endif // MEDIPRIORITY_APPOINTMENTCONTROLLER_H
