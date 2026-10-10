#ifndef MEDIPRIORITY_REPORTREPOSITORY_H
#define MEDIPRIORITY_REPORTREPOSITORY_H

#include "../utils/Database.h"
#include <string>
#include <vector>

namespace medipriority {

struct DashboardSummary {
    int totalPatients = 0;
    int totalEmergencyCases = 0;
    int criticalCount = 0, highCount = 0, moderateCount = 0, lowCount = 0;
    int availableDoctors = 0, totalDoctors = 0;
    int occupiedBeds = 0, availableBeds = 0;
    int occupiedIcuBeds = 0, availableIcuBeds = 0;
    int availableAmbulances = 0, totalAmbulances = 0;
    int upcomingAppointments = 0;
};

/* One point on the "emergency arrivals per day" trend line. */
struct DailyCount {
    std::string date; // "YYYY-MM-DD"
    int count = 0;
};

/* One row of the "busiest doctors right now" table. */
struct DoctorWorkload {
    std::string doctorName;
    int activeCases = 0;
};

/* Average time (in minutes) that currently-waiting cases of a given
 * severity have been waiting, plus how many are waiting at that severity. */
struct WaitTimeStat {
    int severity = 0;
    double avgWaitMinutes = 0.0;
    int waitingCount = 0;
};

/* Occupied vs total beds for one bed_type ('General' | 'Emergency' | 'ICU'). */
struct BedTypeOccupancy {
    std::string bedType;
    int occupied = 0;
    int total = 0;
};

struct AnalyticsData {
    std::vector<DailyCount> dailyArrivals;          // last 7 days, oldest first
    std::vector<DoctorWorkload> topDoctorWorkload;   // top 5, busiest first
    std::vector<WaitTimeStat> waitTimesBySeverity;   // severities 1..4
    std::vector<BedTypeOccupancy> bedOccupancyByType;
    int scheduledAppointments = 0;
    int completedAppointments = 0;
    int cancelledAppointments = 0;
};

/* A live, computed operational alert -- never stored, always derived fresh
 * from current MySQL state each time it's requested. level is one of
 * "critical" | "warning" | "info". */
struct Alert {
    std::string level;
    std::string message;
};

class ReportRepository {
private:
    Database &db_;
public:
    explicit ReportRepository(Database &db) : db_(db) {}

    /* Pulls ACTUAL current counts straight from MySQL -- never fabricated
     * or hardcoded, per the project's dashboard requirement. */
    DashboardSummary getSummary();

    /* Extended analytics for the Analytics dashboard: trends, workload,
     * wait times, occupancy by bed type. Also pulled live from MySQL. */
    AnalyticsData getAnalytics();

    /* Computes operational alerts (long waits, near-full ICU, no free
     * ambulances, low doctor availability) from current MySQL state. */
    std::vector<Alert> getAlerts();
};

} // namespace medipriority
#endif // MEDIPRIORITY_REPORTREPOSITORY_H
