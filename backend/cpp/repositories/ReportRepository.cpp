#include "ReportRepository.h"
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <memory>
#include <ctime>
#include <unordered_map>
#include <sstream>
#include <iomanip>

namespace medipriority {

static int scalarInt(Database &db, const std::string &sql) {
    auto stmt = db.prepare(sql);
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    rs->next();
    return rs->getInt(1);
}

/* "YYYY-MM-DD" for a unix timestamp, in local time (matches MySQL's
 * FROM_UNIXTIME()/DATE() used on the query side, so the two line up). */
static std::string dateLabel(std::time_t t) {
    std::tm tmBuf{};
#if defined(_WIN32)
    localtime_s(&tmBuf, &t);
#else
    localtime_r(&t, &tmBuf);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tmBuf, "%Y-%m-%d");
    return oss.str();
}

DashboardSummary ReportRepository::getSummary() {
    DashboardSummary s;

    s.totalPatients = scalarInt(db_, "SELECT COUNT(*) FROM patients");
    s.totalEmergencyCases = scalarInt(db_, "SELECT COUNT(*) FROM emergency_cases WHERE status = 'Waiting'");

    s.criticalCount = scalarInt(db_, "SELECT COUNT(*) FROM emergency_cases WHERE status='Waiting' AND severity=1");
    s.highCount     = scalarInt(db_, "SELECT COUNT(*) FROM emergency_cases WHERE status='Waiting' AND severity=2");
    s.moderateCount = scalarInt(db_, "SELECT COUNT(*) FROM emergency_cases WHERE status='Waiting' AND severity=3");
    s.lowCount      = scalarInt(db_, "SELECT COUNT(*) FROM emergency_cases WHERE status='Waiting' AND severity=4");

    s.totalDoctors = scalarInt(db_, "SELECT COUNT(*) FROM doctors");
    s.availableDoctors = scalarInt(db_, "SELECT COUNT(*) FROM doctors WHERE available = TRUE");

    s.occupiedBeds = scalarInt(db_, "SELECT COUNT(*) FROM beds WHERE occupied = TRUE");
    s.availableBeds = scalarInt(db_, "SELECT COUNT(*) FROM beds WHERE occupied = FALSE");
    s.occupiedIcuBeds = scalarInt(db_, "SELECT COUNT(*) FROM beds WHERE occupied = TRUE AND bed_type = 'ICU'");
    s.availableIcuBeds = scalarInt(db_, "SELECT COUNT(*) FROM beds WHERE occupied = FALSE AND bed_type = 'ICU'");

    s.totalAmbulances = scalarInt(db_, "SELECT COUNT(*) FROM ambulances");
    s.availableAmbulances = scalarInt(db_, "SELECT COUNT(*) FROM ambulances WHERE status = 'Available'");

    s.upcomingAppointments = scalarInt(db_, "SELECT COUNT(*) FROM appointments WHERE status = 'Scheduled'");

    return s;
}

AnalyticsData ReportRepository::getAnalytics() {
    AnalyticsData a;

    // --- Emergency arrivals per day, last 7 days (today inclusive) ---
    // Build the 7 date labels first so days with zero cases still show up
    // as 0 instead of being silently missing from the trend line.
    std::time_t now = std::time(nullptr);
    std::unordered_map<std::string, int> byDate;
    std::vector<std::string> orderedDates;
    for (int i = 6; i >= 0; --i) {
        std::time_t day = now - static_cast<std::time_t>(i) * 86400;
        std::string label = dateLabel(day);
        orderedDates.push_back(label);
        byDate[label] = 0;
    }
    {
        auto stmt = db_.prepare(
            "SELECT DATE(FROM_UNIXTIME(arrival_time)) AS d, COUNT(*) AS c "
            "FROM emergency_cases "
            "WHERE arrival_time >= UNIX_TIMESTAMP(CURDATE() - INTERVAL 6 DAY) "
            "GROUP BY d");
        std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
        while (rs->next()) {
            std::string d = rs->getString("d");
            int c = rs->getInt("c");
            auto it = byDate.find(d);
            if (it != byDate.end()) it->second = c;
        }
    }
    for (const auto &d : orderedDates) {
        a.dailyArrivals.push_back({d, byDate[d]});
    }

    // --- Busiest doctors right now (active "In Treatment" cases) ---
    {
        auto stmt = db_.prepare(
            "SELECT d.name AS name, COUNT(*) AS c "
            "FROM emergency_cases ec "
            "JOIN doctors d ON d.doctor_id = ec.assigned_doctor_id "
            "WHERE ec.status = 'In Treatment' "
            "GROUP BY d.doctor_id, d.name "
            "ORDER BY c DESC, name ASC "
            "LIMIT 5");
        std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
        while (rs->next()) {
            a.topDoctorWorkload.push_back({rs->getString("name"), rs->getInt("c")});
        }
    }

    // --- Average wait time (minutes) for currently-waiting cases, by severity ---
    {
        auto stmt = db_.prepare(
            "SELECT severity, "
            "       AVG(UNIX_TIMESTAMP() - arrival_time) / 60.0 AS avg_minutes, "
            "       COUNT(*) AS cnt "
            "FROM emergency_cases "
            "WHERE status = 'Waiting' "
            "GROUP BY severity");
        std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
        std::unordered_map<int, WaitTimeStat> bySeverity;
        while (rs->next()) {
            int sev = rs->getInt("severity");
            bySeverity[sev] = WaitTimeStat{sev, static_cast<double>(rs->getDouble("avg_minutes")), static_cast<int>(rs->getInt("cnt"))};
        }
        for (int sev = 1; sev <= 4; ++sev) {
            auto it = bySeverity.find(sev);
            if (it != bySeverity.end()) a.waitTimesBySeverity.push_back(it->second);
            else a.waitTimesBySeverity.push_back({sev, 0.0, 0});
        }
    }

    // --- Bed occupancy by type ---
    {
        auto stmt = db_.prepare(
            "SELECT bed_type, SUM(occupied) AS occ, COUNT(*) AS total "
            "FROM beds GROUP BY bed_type");
        std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
        while (rs->next()) {
            a.bedOccupancyByType.push_back({
                rs->getString("bed_type"), rs->getInt("occ"), rs->getInt("total")
            });
        }
    }

    // --- Appointment status breakdown ---
    a.scheduledAppointments = scalarInt(db_, "SELECT COUNT(*) FROM appointments WHERE status = 'Scheduled'");
    a.completedAppointments = scalarInt(db_, "SELECT COUNT(*) FROM appointments WHERE status = 'Completed'");
    a.cancelledAppointments = scalarInt(db_, "SELECT COUNT(*) FROM appointments WHERE status = 'Cancelled'");

    return a;
}

std::vector<Alert> ReportRepository::getAlerts() {
    std::vector<Alert> alerts;

    // Critical (severity 1) cases that have been waiting more than 15 minutes.
    {
        auto stmt = db_.prepare(
            "SELECT COUNT(*) AS c FROM emergency_cases "
            "WHERE status = 'Waiting' AND severity = 1 "
            "AND (UNIX_TIMESTAMP() - arrival_time) > 900");
        std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
        rs->next();
        int c = rs->getInt("c");
        if (c > 0) {
            alerts.push_back({"critical",
                std::to_string(c) + (c == 1 ? " critical case has" : " critical cases have") +
                " been waiting over 15 minutes"});
        }
    }

    // ICU beds at or above 90% occupied.
    {
        auto stmt = db_.prepare(
            "SELECT SUM(occupied) AS occ, COUNT(*) AS total FROM beds WHERE bed_type = 'ICU'");
        std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
        rs->next();
        int occ = rs->getInt("occ");
        int total = rs->getInt("total");
        if (total > 0 && (double)occ / total >= 0.9) {
            alerts.push_back({"warning",
                "ICU beds are at " + std::to_string(occ) + "/" + std::to_string(total) +
                " capacity"});
        }
    }

    // No ambulances currently available.
    {
        auto stmt = db_.prepare(
            "SELECT SUM(status = 'Available') AS avail, COUNT(*) AS total FROM ambulances");
        std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
        rs->next();
        int avail = rs->getInt("avail");
        int total = rs->getInt("total");
        if (total > 0 && avail == 0) {
            alerts.push_back({"critical", "No ambulances are currently available"});
        }
    }

    // Fewer than 20% of doctors currently available.
    {
        auto stmt = db_.prepare(
            "SELECT SUM(available) AS avail, COUNT(*) AS total FROM doctors");
        std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
        rs->next();
        int avail = rs->getInt("avail");
        int total = rs->getInt("total");
        if (total > 0 && (double)avail / total < 0.2) {
            alerts.push_back({"warning",
                "Only " + std::to_string(avail) + "/" + std::to_string(total) +
                " doctors are currently available"});
        }
    }

    // Any waiting emergency cases at all is worth a quiet heads-up, not an alarm.
    {
        int waiting = scalarInt(db_, "SELECT COUNT(*) FROM emergency_cases WHERE status = 'Waiting'");
        if (waiting > 0 && alerts.empty()) {
            alerts.push_back({"info",
                std::to_string(waiting) + " case(s) currently waiting in triage"});
        }
    }

    return alerts;
}

} // namespace medipriority
