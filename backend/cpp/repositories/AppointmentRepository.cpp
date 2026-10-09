#include "AppointmentRepository.h"
#include <cppconn/resultset.h>
#include <memory>

namespace medipriority {

static Appointment rowToAppt(sql::ResultSet *rs) {
    return Appointment(
        rs->getInt("appointment_id"), rs->getInt("patient_id"), rs->getInt("doctor_id"),
        rs->getInt64("scheduled_time"), rs->getString("status")
    );
}

int AppointmentRepository::create(const Appointment &appt) {
    auto stmt = db_.prepare(
        "INSERT INTO appointments (patient_id, doctor_id, scheduled_time, status) "
        "VALUES (?, ?, ?, ?)");
    stmt->setInt(1, appt.patientId());
    stmt->setInt(2, appt.doctorId());
    stmt->setInt64(3, appt.scheduledTime());
    stmt->setString(4, appt.status());
    stmt->executeUpdate();
    auto idStmt = db_.prepare("SELECT LAST_INSERT_ID() AS id");
    std::unique_ptr<sql::ResultSet> rs(idStmt->executeQuery());
    rs->next();
    return rs->getInt("id");
}

std::optional<Appointment> AppointmentRepository::findById(int appointmentId) {
    auto stmt = db_.prepare(
        "SELECT appointment_id, patient_id, doctor_id, scheduled_time, status "
        "FROM appointments WHERE appointment_id = ?");
    stmt->setInt(1, appointmentId);
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    if (rs->next()) return rowToAppt(rs.get());
    return std::nullopt;
}

std::vector<Appointment> AppointmentRepository::findUpcoming(int limit) {
    auto stmt = db_.prepare(
        "SELECT appointment_id, patient_id, doctor_id, scheduled_time, status "
        "FROM appointments WHERE status = 'Scheduled' ORDER BY scheduled_time LIMIT ?");
    stmt->setInt(1, limit);
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    std::vector<Appointment> out;
    while (rs->next()) out.push_back(rowToAppt(rs.get()));
    return out;
}

std::vector<Appointment> AppointmentRepository::findByPatient(int patientId) {
    auto stmt = db_.prepare(
        "SELECT appointment_id, patient_id, doctor_id, scheduled_time, status "
        "FROM appointments WHERE patient_id = ? ORDER BY scheduled_time");
    stmt->setInt(1, patientId);
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    std::vector<Appointment> out;
    while (rs->next()) out.push_back(rowToAppt(rs.get()));
    return out;
}

bool AppointmentRepository::hasConflict(int doctorId, long scheduledTime) {
    auto stmt = db_.prepare(
        "SELECT COUNT(*) AS cnt FROM appointments "
        "WHERE doctor_id = ? AND scheduled_time = ? AND status != 'Cancelled'");
    stmt->setInt(1, doctorId);
    stmt->setInt64(2, scheduledTime);
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    rs->next();
    return rs->getInt("cnt") > 0;
}

bool AppointmentRepository::cancel(int appointmentId) {
    auto stmt = db_.prepare("UPDATE appointments SET status = 'Cancelled' WHERE appointment_id = ?");
    stmt->setInt(1, appointmentId);
    return stmt->executeUpdate() > 0;
}

bool AppointmentRepository::complete(int appointmentId) {
    auto stmt = db_.prepare("UPDATE appointments SET status = 'Completed' WHERE appointment_id = ?");
    stmt->setInt(1, appointmentId);
    return stmt->executeUpdate() > 0;
}

} // namespace medipriority
