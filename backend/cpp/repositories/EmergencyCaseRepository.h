#ifndef MEDIPRIORITY_EMERGENCYCASEREPOSITORY_H
#define MEDIPRIORITY_EMERGENCYCASEREPOSITORY_H

#include "../utils/Database.h"
#include "../models/EmergencyCase.h"
#include <optional>
#include <vector>

namespace medipriority {

class EmergencyCaseRepository {
private:
    Database &db_;
public:
    explicit EmergencyCaseRepository(Database &db) : db_(db) {}

    int create(const EmergencyCase &ec);
    std::optional<EmergencyCase> findById(int caseId);
    std::vector<EmergencyCase> findWaiting();
    bool assignDoctor(int caseId, int doctorId);
    bool assignBed(int caseId, int bedId);
    bool assignAmbulance(int caseId, int ambulanceId);
    bool updateStatus(int caseId, const std::string &status);
    bool updateSeverity(int caseId, int severity);

    /* Counts of waiting cases grouped by severity (1..4), for the dashboard. */
    std::vector<std::pair<int,int>> countBySeverity();
};

} // namespace medipriority
#endif // MEDIPRIORITY_EMERGENCYCASEREPOSITORY_H
