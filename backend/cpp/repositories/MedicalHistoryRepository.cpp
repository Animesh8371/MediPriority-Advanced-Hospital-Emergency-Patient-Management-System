#include "MedicalHistoryRepository.h"
#include <cppconn/resultset.h>
#include <memory>

namespace medipriority {

int MedicalHistoryRepository::create(const MedicalRecord &record) {
    auto stmt = db_.prepare(
        "INSERT INTO medical_history (patient_id, diagnosis, notes, timestamp) "
        "VALUES (?, ?, ?, ?)");
    stmt->setInt(1, record.patientId());
    stmt->setString(2, record.diagnosis());
    stmt->setString(3, record.notes());
    stmt->setInt64(4, record.timestamp());
    stmt->executeUpdate();
    auto idStmt = db_.prepare("SELECT LAST_INSERT_ID() AS id");
    std::unique_ptr<sql::ResultSet> rs(idStmt->executeQuery());
    rs->next();
    return rs->getInt("id");
}

std::vector<MedicalRecord> MedicalHistoryRepository::findByPatient(int patientId) {
    auto stmt = db_.prepare(
        "SELECT record_id, patient_id, diagnosis, notes, timestamp FROM medical_history "
        "WHERE patient_id = ? ORDER BY timestamp ASC");
    stmt->setInt(1, patientId);
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    std::vector<MedicalRecord> out;
    while (rs->next()) {
        out.emplace_back(
            rs->getInt("record_id"), rs->getInt("patient_id"),
            rs->getString("diagnosis"), rs->getString("notes"), rs->getInt64("timestamp")
        );
    }
    return out;
}

} // namespace medipriority
