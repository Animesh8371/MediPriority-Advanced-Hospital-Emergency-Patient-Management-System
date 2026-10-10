#include "AmbulanceRepository.h"
#include <cppconn/resultset.h>
#include <cppconn/exception.h>
#include <memory>

namespace medipriority {

static Ambulance rowToAmbulance(sql::ResultSet *rs) {
    return Ambulance(rs->getInt("ambulance_id"), rs->getString("vehicle_number"),
                      rs->getString("status"));
}

std::optional<Ambulance> AmbulanceRepository::findById(int ambulanceId) {
    auto stmt = db_.prepare("SELECT ambulance_id, vehicle_number, status FROM ambulances WHERE ambulance_id = ?");
    stmt->setInt(1, ambulanceId);
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    if (rs->next()) return rowToAmbulance(rs.get());
    return std::nullopt;
}

std::vector<Ambulance> AmbulanceRepository::findAll() {
    auto stmt = db_.prepare("SELECT ambulance_id, vehicle_number, status FROM ambulances ORDER BY ambulance_id");
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    std::vector<Ambulance> out;
    while (rs->next()) out.push_back(rowToAmbulance(rs.get()));
    return out;
}

std::vector<Ambulance> AmbulanceRepository::findAvailable() {
    auto stmt = db_.prepare("SELECT ambulance_id, vehicle_number, status FROM ambulances WHERE status = 'Available' ORDER BY ambulance_id");
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    std::vector<Ambulance> out;
    while (rs->next()) out.push_back(rowToAmbulance(rs.get()));
    return out;
}

int AmbulanceRepository::create(const std::string &vehicleNumber) {
    auto stmt = db_.prepare("INSERT INTO ambulances (vehicle_number, status) VALUES (?, 'Available')");
    stmt->setString(1, vehicleNumber);
    stmt->executeUpdate();
    auto idStmt = db_.prepare("SELECT LAST_INSERT_ID() AS id");
    std::unique_ptr<sql::ResultSet> rs(idStmt->executeQuery());
    rs->next();
    return rs->getInt("id");
}

AmbulanceActionResult AmbulanceRepository::assign(int ambulanceId) {
    AmbulanceActionResult result;
    try {
        db_.beginTransaction();
        auto checkStmt = db_.prepare("SELECT status FROM ambulances WHERE ambulance_id = ? FOR UPDATE");
        checkStmt->setInt(1, ambulanceId);
        std::unique_ptr<sql::ResultSet> rs(checkStmt->executeQuery());
        if (!rs->next()) { db_.rollback(); result.message = "Ambulance not found"; return result; }
        if (rs->getString("status") != "Available") {
            db_.rollback();
            result.message = "Ambulance is not available (status: " + rs->getString("status") + ")";
            return result;
        }
        auto updateStmt = db_.prepare("UPDATE ambulances SET status = 'Assigned' WHERE ambulance_id = ?");
        updateStmt->setInt(1, ambulanceId);
        updateStmt->executeUpdate();
        db_.commit();
        result.success = true;
        result.message = "Ambulance assigned";
    } catch (sql::SQLException &e) {
        db_.rollback();
        result.message = std::string("Assignment failed: ") + e.what();
    }
    return result;
}

AmbulanceActionResult AmbulanceRepository::release(int ambulanceId) {
    AmbulanceActionResult result;
    auto stmt = db_.prepare("UPDATE ambulances SET status = 'Available' WHERE ambulance_id = ?");
    stmt->setInt(1, ambulanceId);
    int rows = stmt->executeUpdate();
    result.success = rows > 0;
    result.message = rows > 0 ? "Ambulance released" : "Ambulance not found";
    return result;
}

} // namespace medipriority
