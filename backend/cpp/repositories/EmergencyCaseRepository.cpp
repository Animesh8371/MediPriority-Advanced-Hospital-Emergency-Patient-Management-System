#include "EmergencyCaseRepository.h"
#include <cppconn/resultset.h>
#include <memory>

namespace medipriority {

static EmergencyCase rowToCase(sql::ResultSet *rs) {
    EmergencyCase ec(
        rs->getInt("case_id"), rs->getInt("patient_id"), rs->getInt("severity"),
        rs->getInt64("arrival_time"), rs->getString("category"), rs->getString("status")
    );
    if (!rs->isNull("assigned_doctor_id")) ec.assignDoctor(rs->getInt("assigned_doctor_id"));
    if (!rs->isNull("assigned_bed_id")) ec.assignBed(rs->getInt("assigned_bed_id"));
    if (!rs->isNull("assigned_ambulance_id")) ec.assignAmbulance(rs->getInt("assigned_ambulance_id"));
    return ec;
}

int EmergencyCaseRepository::create(const EmergencyCase &ec) {
    auto stmt = db_.prepare(
        "INSERT INTO emergency_cases (patient_id, severity, category, arrival_time, status) "
        "VALUES (?, ?, ?, ?, ?)");
    stmt->setInt(1, ec.patientId());
    stmt->setInt(2, ec.severity());
    stmt->setString(3, ec.category());
    stmt->setInt64(4, ec.arrivalTime());
    stmt->setString(5, ec.status());
    stmt->executeUpdate();
    auto idStmt = db_.prepare("SELECT LAST_INSERT_ID() AS id");
    std::unique_ptr<sql::ResultSet> rs(idStmt->executeQuery());
    rs->next();
    return rs->getInt("id");
}

std::optional<EmergencyCase> EmergencyCaseRepository::findById(int caseId) {
    auto stmt = db_.prepare(
        "SELECT case_id, patient_id, severity, category, arrival_time, status, "
        "assigned_doctor_id, assigned_bed_id, assigned_ambulance_id "
        "FROM emergency_cases WHERE case_id = ?");
    stmt->setInt(1, caseId);
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    if (rs->next()) return rowToCase(rs.get());
    return std::nullopt;
}

std::vector<EmergencyCase> EmergencyCaseRepository::findWaiting() {
    auto stmt = db_.prepare(
        "SELECT case_id, patient_id, severity, category, arrival_time, status, "
        "assigned_doctor_id, assigned_bed_id, assigned_ambulance_id "
        "FROM emergency_cases WHERE status = 'Waiting' ORDER BY severity, arrival_time");
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    std::vector<EmergencyCase> out;
    while (rs->next()) out.push_back(rowToCase(rs.get()));
    return out;
}

bool EmergencyCaseRepository::assignDoctor(int caseId, int doctorId) {
    auto stmt = db_.prepare("UPDATE emergency_cases SET assigned_doctor_id = ? WHERE case_id = ?");
    stmt->setInt(1, doctorId);
    stmt->setInt(2, caseId);
    return stmt->executeUpdate() > 0;
}

bool EmergencyCaseRepository::assignBed(int caseId, int bedId) {
    auto stmt = db_.prepare("UPDATE emergency_cases SET assigned_bed_id = ? WHERE case_id = ?");
    stmt->setInt(1, bedId);
    stmt->setInt(2, caseId);
    return stmt->executeUpdate() > 0;
}

bool EmergencyCaseRepository::assignAmbulance(int caseId, int ambulanceId) {
    auto stmt = db_.prepare("UPDATE emergency_cases SET assigned_ambulance_id = ? WHERE case_id = ?");
    stmt->setInt(1, ambulanceId);
    stmt->setInt(2, caseId);
    return stmt->executeUpdate() > 0;
}

bool EmergencyCaseRepository::updateStatus(int caseId, const std::string &status) {
    auto stmt = db_.prepare("UPDATE emergency_cases SET status = ? WHERE case_id = ?");
    stmt->setString(1, status);
    stmt->setInt(2, caseId);
    return stmt->executeUpdate() > 0;
}

bool EmergencyCaseRepository::updateSeverity(int caseId, int severity) {
    auto stmt = db_.prepare("UPDATE emergency_cases SET severity = ? WHERE case_id = ?");
    stmt->setInt(1, severity);
    stmt->setInt(2, caseId);
    return stmt->executeUpdate() > 0;
}

std::vector<std::pair<int,int>> EmergencyCaseRepository::countBySeverity() {
    auto stmt = db_.prepare(
        "SELECT severity, COUNT(*) AS cnt FROM emergency_cases "
        "WHERE status = 'Waiting' GROUP BY severity ORDER BY severity");
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    std::vector<std::pair<int,int>> out;
    while (rs->next()) out.push_back({rs->getInt("severity"), rs->getInt("cnt")});
    return out;
}

} // namespace medipriority
