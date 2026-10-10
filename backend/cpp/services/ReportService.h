#ifndef MEDIPRIORITY_REPORTSERVICE_H
#define MEDIPRIORITY_REPORTSERVICE_H

#include "../repositories/ReportRepository.h"

namespace medipriority {

class ReportService {
private:
    ReportRepository &repo_;
public:
    explicit ReportService(ReportRepository &repo) : repo_(repo) {}

    /* Always pulls live counts from MySQL -- never fabricated. */
    DashboardSummary getDashboardSummary() { return repo_.getSummary(); }

    /* Trend/workload/wait-time/occupancy data for the Analytics page. */
    AnalyticsData getAnalytics() { return repo_.getAnalytics(); }

    /* Live operational alerts, recomputed on every call. */
    std::vector<Alert> getAlerts() { return repo_.getAlerts(); }
};

} // namespace medipriority
#endif // MEDIPRIORITY_REPORTSERVICE_H
