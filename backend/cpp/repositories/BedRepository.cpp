#include "BedRepository.h"
#include <cppconn/resultset.h>
#include <cppconn/exception.h>
#include <memory>

namespace medipriority {

static Bed rowToBed(sql::ResultSet *rs) {
    return Bed(
        rs->getInt("bed_id"), rs->getString("bed_type"), rs->getBoolean("occupied"),
        rs->isNull("occupied_by_patient_id") ? 0 : rs->getInt("occupied_by_patient_id")
    );
}

std::optional<Bed> BedRepository::findById(int bedId) {
    auto stmt = db_.prepare(
        "SELECT bed_id, bed_type, occupied, occupied_by_patient_id "
        "FROM beds WHERE bed_id = ?");
    stmt->setInt(1, bedId);
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    if (rs->next()) return rowToBed(rs.get());
    return std::nullopt;
}

std::vector<Bed> BedRepository::findAll() {
    auto stmt = db_.prepare(
        "SELECT bed_id, bed_type, occupied, occupied_by_patient_id FROM beds ORDER BY bed_id");
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    std::vector<Bed> out;
    while (rs->next()) out.push_back(rowToBed(rs.get()));
    return out;
}

std::vector<Bed> BedRepository::findAvailable(const std::string &bedType) {
    auto stmt = db_.prepare(
        "SELECT bed_id, bed_type, occupied, occupied_by_patient_id FROM beds "
        "WHERE occupied = FALSE AND bed_type = ? ORDER BY bed_id");
    stmt->setString(1, bedType);
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    std::vector<Bed> out;
    while (rs->next()) out.push_back(rowToBed(rs.get()));
    return out;
}

int BedRepository::create(const std::string &bedType) {
    auto stmt = db_.prepare("INSERT INTO beds (bed_type, occupied) VALUES (?, FALSE)");
    stmt->setString(1, bedType);
    stmt->executeUpdate();
    auto idStmt = db_.prepare("SELECT LAST_INSERT_ID() AS id");
    std::unique_ptr<sql::ResultSet> rs(idStmt->executeQuery());
    rs->next();
    return rs->getInt("id");
}

BedActionResult BedRepository::allocate(int bedId, int patientId, int doctorId) {
    BedActionResult result;
    try {
        db_.beginTransaction();

        // Re-check occupancy status inside the transaction to prevent a
        // race condition between two near-simultaneous allocation requests.
        auto checkStmt = db_.prepare("SELECT occupied FROM beds WHERE bed_id = ? FOR UPDATE");
        checkStmt->setInt(1, bedId);
        std::unique_ptr<sql::ResultSet> rs(checkStmt->executeQuery());
        if (!rs->next()) {
            db_.rollback();
            result.message = "Bed not found";
            return result;
        }
        if (rs->getBoolean("occupied")) {
            db_.rollback();
            result.message = "Bed is already occupied";
            return result;
        }

        auto updateStmt = db_.prepare(
            "UPDATE beds SET occupied = TRUE, occupied_by_patient_id = ? WHERE bed_id = ?");
        updateStmt->setInt(1, patientId);
        updateStmt->setInt(2, bedId);
        updateStmt->executeUpdate();

        auto assignStmt = db_.prepare(
            "INSERT INTO patient_assignments (patient_id, bed_id, doctor_id) VALUES (?, ?, ?)");
        assignStmt->setInt(1, patientId);
        assignStmt->setInt(2, bedId);
        if (doctorId > 0) assignStmt->setInt(3, doctorId);
        else assignStmt->setNull(3, sql::DataType::INTEGER);
        assignStmt->executeUpdate();

        db_.commit();
        result.success = true;
        result.message = "Bed allocated successfully";
    } catch (sql::SQLException &e) {
        db_.rollback();
        result.message = std::string("Allocation failed: ") + e.what();
    }
    return result;
}

BedActionResult BedRepository::release(int bedId) {
    BedActionResult result;
    try {
        db_.beginTransaction();

        auto updateStmt = db_.prepare(
            "UPDATE beds SET occupied = FALSE, occupied_by_patient_id = NULL WHERE bed_id = ?");
        updateStmt->setInt(1, bedId);
        int rows = updateStmt->executeUpdate();

        if (rows == 0) {
            db_.rollback();
            result.message = "Bed not found";
            return result;
        }

        auto closeStmt = db_.prepare(
            "UPDATE patient_assignments SET released_at = NOW() "
            "WHERE bed_id = ? AND released_at IS NULL");
        closeStmt->setInt(1, bedId);
        closeStmt->executeUpdate();

        db_.commit();
        result.success = true;
        result.message = "Bed released successfully";
    } catch (sql::SQLException &e) {
        db_.rollback();
        result.message = std::string("Release failed: ") + e.what();
    }
    return result;
}

} // namespace medipriority
