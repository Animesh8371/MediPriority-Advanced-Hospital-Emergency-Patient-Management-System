#ifndef MEDIPRIORITY_ADMISSIONCONTROLLER_H
#define MEDIPRIORITY_ADMISSIONCONTROLLER_H

#include "crow_all.h"
#include <cstdlib>
#include "JsonHelpers.h"
#include "AuthMiddleware.h"
#include "../services/AdmissionService.h"
#include "../services/AuthService.h"

namespace medipriority {

inline crow::json::wvalue admissionToJson(const AdmissionResult &r) {
    crow::json::wvalue j;
    j["success"] = r.success;
    j["message"] = r.message;
    if (r.patientId > 0) {
        j["patient_id"] = r.patientId;
        j["patient_code"] = r.patientCode;
        j["duplicate_warning"] = r.duplicateWarning;
    }
    if (r.caseId > 0) {
        j["case_id"] = r.caseId;
        j["severity"] = r.severity;
        j["severity_label"] = severityLabel(r.severity);
        j["waitlisted"] = r.waitlisted;
    }
    if (r.bedAllocated) {
        j["bed"]["bed_id"] = r.bedId;
        j["bed"]["bed_type"] = r.bedType;
    } else {
        j["bed"] = nullptr;
    }
    if (r.doctorAssigned) {
        j["doctor"]["doctor_id"] = r.doctorId;
        j["doctor"]["name"] = r.doctorName;
        j["doctor"]["specialization"] = r.doctorSpecialization;
        j["doctor"]["current_patients"] = r.doctorLoad;
        j["doctor"]["max_patients"] = r.doctorCapacity;
    } else {
        j["doctor"] = nullptr;
    }
    return j;
}

template <typename AppT>
inline void registerAdmissionRoutes(AppT &app, AdmissionService &admissionService,
                                     AuthService &authService) {

    // POST /api/admissions
    // Registers the patient AND creates the emergency case AND allocates a bed + doctor.
    CROW_ROUTE(app, "/api/admissions").methods("POST"_method)
    ([&admissionService, &authService](const crow::request &req) {
        if (!authenticate(req, authService)) return errorResponse(401, "Unauthorized");

        auto body = crow::json::load(req.body);
        if (!body) return errorResponse(400, "Invalid JSON body");

        auto str = [&body](const char *key, const char *fallback) {
            return body.has(key) ? std::string(body[key].s()) : std::string(fallback);
        };

        AdmissionRequest in;
        in.name = str("name", "");
        in.age = body.has("age") ? (int)body["age"].i() : 0;
        in.gender = str("gender", "");
        in.phone = str("phone", "");
        in.bloodGroup = str("blood_group", "Unknown");
        in.emergencyContact = str("emergency_contact", "");
        in.dateOfBirth = str("date_of_birth", "");
        in.severity = body.has("severity") ? (int)body["severity"].i() : 0;
        in.category = str("category", "");

        auto result = admissionService.admitNewPatient(in);
        // A registered-but-unqueued patient (patientId set, success false) is still a server-side problem.
        return crow::response(result.success ? 201 : 400, admissionToJson(result));
    });

    // POST /api/admissions/{caseId}/allocate  (retry for a waitlisted case)
    CROW_ROUTE(app, "/api/admissions/<int>/allocate").methods("POST"_method)
    ([&admissionService, &authService](const crow::request &req, int caseId) {
        if (!authenticate(req, authService)) return errorResponse(401, "Unauthorized");
        auto result = admissionService.allocateForCase(caseId);
        return crow::response(result.success ? 200 : 400, admissionToJson(result));
    });

    // GET /api/admissions/preview?severity=1&category=Cardiac  (read-only)
    CROW_ROUTE(app, "/api/admissions/preview").methods("GET"_method)
    ([&admissionService, &authService](const crow::request &req) {
        if (!authenticate(req, authService)) return errorResponse(401, "Unauthorized");

        const char *sev = req.url_params.get("severity");
        const char *cat = req.url_params.get("category");
        int severity = sev ? std::atoi(sev) : 0;
        if (severity < 1 || severity > 4) return errorResponse(400, "severity must be 1-4");

        auto p = admissionService.preview(severity, cat ? cat : "");
        crow::json::wvalue j;
        j["success"] = true;
        j["severity_label"] = severityLabel(severity);

        std::vector<crow::json::wvalue> prefs;
        for (const auto &b : p.bedPreference) prefs.push_back(crow::json::wvalue(b));
        j["bed_preference"] = std::move(prefs);

        if (!p.targetBedType.empty()) {
            j["bed"]["bed_type"] = p.targetBedType;
            j["bed"]["free"] = p.freeBedsInTarget;
        } else {
            j["bed"] = nullptr;
        }
        if (p.doctorFound) {
            j["doctor"]["doctor_id"] = p.doctorId;
            j["doctor"]["name"] = p.doctorName;
            j["doctor"]["specialization"] = p.doctorSpecialization;
            j["doctor"]["current_patients"] = p.doctorLoad;
            j["doctor"]["max_patients"] = p.doctorCapacity;
        } else {
            j["doctor"] = nullptr;
        }
        return crow::response(200, j);
    });
}

} // namespace medipriority

#endif // MEDIPRIORITY_ADMISSIONCONTROLLER_H
