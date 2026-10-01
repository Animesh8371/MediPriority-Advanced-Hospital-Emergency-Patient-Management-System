#ifndef MEDIPRIORITY_REPORTCONTROLLER_H
#define MEDIPRIORITY_REPORTCONTROLLER_H

#include "crow_all.h"
#include "JsonHelpers.h"
#include "AuthMiddleware.h"
#include "../services/ReportService.h"
#include "../services/AuthService.h"
#include <sstream>
#include <iomanip>

namespace medipriority {

template <typename AppT>
inline void registerReportRoutes(AppT &app, ReportService &reportService,
                                  AuthService &authService) {

    CROW_ROUTE(app, "/api/reports/summary").methods("GET"_method)
    ([&reportService, &authService](const crow::request &req) {
        if (!authenticate(req, authService)) return errorResponse(401, "Unauthorized");
        auto summary = reportService.getDashboardSummary();
        crow::json::wvalue j;
        j["success"] = true;
        j["summary"] = toJson(summary);
        return crow::response(200, j);
    });

    // Trends, doctor workload, wait times, bed occupancy by type -- for the
    // Analytics page. Same "always live from MySQL" rule as the summary.
    CROW_ROUTE(app, "/api/reports/analytics").methods("GET"_method)
    ([&reportService, &authService](const crow::request &req) {
        if (!authenticate(req, authService)) return errorResponse(401, "Unauthorized");
        auto analytics = reportService.getAnalytics();
        crow::json::wvalue j;
        j["success"] = true;
        j["analytics"] = toJson(analytics);
        return crow::response(200, j);
    });

    // Downloadable CSV combining the summary + analytics, for handing a
    // snapshot to someone who isn't going to open the dashboard themselves.
    CROW_ROUTE(app, "/api/reports/export.csv").methods("GET"_method)
    ([&reportService, &authService](const crow::request &req) {
        if (!authenticate(req, authService)) return errorResponse(401, "Unauthorized");

        auto s = reportService.getDashboardSummary();
        auto a = reportService.getAnalytics();

        std::ostringstream csv;
        csv << "MediPriority Report Export\r\n\r\n";

        csv << "Summary\r\n";
        csv << "Metric,Value\r\n";
        csv << "Total patients," << s.totalPatients << "\r\n";
        csv << "Emergency cases waiting," << s.totalEmergencyCases << "\r\n";
        csv << "  Critical," << s.criticalCount << "\r\n";
        csv << "  High," << s.highCount << "\r\n";
        csv << "  Moderate," << s.moderateCount << "\r\n";
        csv << "  Low," << s.lowCount << "\r\n";
        csv << "Doctors available/total," << s.availableDoctors << "/" << s.totalDoctors << "\r\n";
        csv << "Beds available/occupied," << s.availableBeds << "/" << s.occupiedBeds << "\r\n";
        csv << "ICU beds available/occupied," << s.availableIcuBeds << "/" << s.occupiedIcuBeds << "\r\n";
        csv << "Ambulances available/total," << s.availableAmbulances << "/" << s.totalAmbulances << "\r\n";
        csv << "Upcoming appointments," << s.upcomingAppointments << "\r\n";

        csv << "\r\nEmergency arrivals - last 7 days\r\n";
        csv << "Date,Arrivals\r\n";
        for (const auto &d : a.dailyArrivals) csv << d.date << "," << d.count << "\r\n";

        csv << "\r\nBusiest doctors (active cases)\r\n";
        csv << "Doctor,Active Cases\r\n";
        for (const auto &w : a.topDoctorWorkload) csv << w.doctorName << "," << w.activeCases << "\r\n";

        csv << "\r\nAverage wait time by severity (minutes)\r\n";
        csv << "Severity,Waiting Count,Avg Wait (min)\r\n";
        for (const auto &wt : a.waitTimesBySeverity) {
            csv << severityLabel(wt.severity) << "," << wt.waitingCount << ","
                << std::fixed << std::setprecision(1) << wt.avgWaitMinutes << "\r\n";
        }

        csv << "\r\nBed occupancy by type\r\n";
        csv << "Bed Type,Occupied,Total\r\n";
        for (const auto &b : a.bedOccupancyByType) csv << b.bedType << "," << b.occupied << "," << b.total << "\r\n";

        crow::response res(200, csv.str());
        res.set_header("Content-Type", "text/csv; charset=utf-8");
        res.set_header("Content-Disposition", "attachment; filename=\"medipriority_report.csv\"");
        return res;
    });

    // Live operational alerts (long waits, near-full ICU, no free
    // ambulances, low doctor availability). Computed fresh every call --
    // nothing here is stored, so there's no "unread" state to manage.
    CROW_ROUTE(app, "/api/notifications").methods("GET"_method)
    ([&reportService, &authService](const crow::request &req) {
        if (!authenticate(req, authService)) return errorResponse(401, "Unauthorized");
        auto alerts = reportService.getAlerts();
        crow::json::wvalue j;
        j["success"] = true;
        j["alerts"] = toJson(alerts);
        return crow::response(200, j);
    });
}

} // namespace medipriority

#endif // MEDIPRIORITY_REPORTCONTROLLER_H
