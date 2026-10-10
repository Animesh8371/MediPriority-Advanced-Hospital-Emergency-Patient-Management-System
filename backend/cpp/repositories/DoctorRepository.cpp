#include "DoctorRepository.h"
#include <cppconn/resultset.h>
#include <memory>

namespace medipriority {

/* current_patients is computed, never stored: it is the number of
 * patient_assignments rows for this doctor that have not been released.
 * Computing it on read means it can never drift out of sync when a bed is
 * released or a patient is discharged. */
static const char *kDoctorSelect =
    "SELECT d.doctor_id, d.name, d.age, d.gender, d.specialization, d.available, d.max_patients, "
    "(SELECT COUNT(*) FROM patient_assignments pa "
    "  WHERE pa.doctor_id = d.doctor_id AND pa.released_at IS NULL) AS current_patients "
    "FROM doctors d ";

static Doctor rowToDoctor(sql::ResultSet *rs) {
    return Doctor(
        rs->getInt("doctor_id"), rs->getString("name"), rs->getInt("age"),
        rs->getString("gender"), rs->getString("specialization"),
        rs->getBoolean("available"),
        rs->getInt("max_patients"), rs->getInt("current_patients")
    );
}

std::optional<Doctor> DoctorRepository::findById(int doctorId) {
    auto stmt = db_.prepare(std::string(kDoctorSelect) + "WHERE d.doctor_id = ?");
    stmt->setInt(1, doctorId);
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    if (rs->next()) return rowToDoctor(rs.get());
    return std::nullopt;
}

std::vector<Doctor> DoctorRepository::findAll() {
    auto stmt = db_.prepare(std::string(kDoctorSelect) + "ORDER BY d.name");
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    std::vector<Doctor> out;
    while (rs->next()) out.push_back(rowToDoctor(rs.get()));
    return out;
}

std::vector<Doctor> DoctorRepository::findAvailable() {
    auto stmt = db_.prepare(std::string(kDoctorSelect) + "WHERE d.available = TRUE ORDER BY d.name");
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    std::vector<Doctor> out;
    while (rs->next()) out.push_back(rowToDoctor(rs.get()));
    return out;
}

std::vector<Doctor> DoctorRepository::findAcceptingPatients() {
    auto stmt = db_.prepare(std::string(kDoctorSelect) +
        "WHERE d.available = TRUE "
        "HAVING current_patients < max_patients "
        "ORDER BY current_patients ASC, d.name ASC");
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    std::vector<Doctor> out;
    while (rs->next()) out.push_back(rowToDoctor(rs.get()));
    return out;
}

std::vector<DoctorPatientRow> DoctorRepository::findActivePatients(int doctorId) {
    auto stmt = db_.prepare(
        "SELECT pa.patient_id, p.name AS patient_name, pa.bed_id, b.bed_type, "
        "       CAST(pa.assigned_at AS CHAR) AS assigned_at "
        "FROM patient_assignments pa "
        "JOIN patients p ON p.patient_id = pa.patient_id "
        "LEFT JOIN beds b ON b.bed_id = pa.bed_id "
        "WHERE pa.doctor_id = ? AND pa.released_at IS NULL "
        "ORDER BY pa.assigned_at DESC");
    stmt->setInt(1, doctorId);
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    std::vector<DoctorPatientRow> out;
    while (rs->next()) {
        DoctorPatientRow row;
        row.patientId = rs->getInt("patient_id");
        row.patientName = rs->getString("patient_name");
        row.bedId = rs->isNull("bed_id") ? 0 : rs->getInt("bed_id");
        row.bedType = rs->isNull("bed_type") ? std::string("") : std::string(rs->getString("bed_type"));
        row.assignedAt = rs->getString("assigned_at");
        out.push_back(row);
    }
    return out;
}

int DoctorRepository::create(const Doctor &doctor) {
    auto stmt = db_.prepare(
        "INSERT INTO doctors (name, age, gender, specialization, available, max_patients) "
        "VALUES (?, ?, ?, ?, ?, ?)");
    stmt->setString(1, doctor.name());
    stmt->setInt(2, doctor.age());
    stmt->setString(3, doctor.gender());
    stmt->setString(4, doctor.specialization());
    stmt->setBoolean(5, doctor.isAvailable());
    stmt->setInt(6, doctor.maxPatients());
    stmt->executeUpdate();

    auto idStmt = db_.prepare("SELECT LAST_INSERT_ID() AS id");
    std::unique_ptr<sql::ResultSet> rs(idStmt->executeQuery());
    rs->next();
    return rs->getInt("id");
}

bool DoctorRepository::update(const Doctor &doctor) {
    auto stmt = db_.prepare(
        "UPDATE doctors SET name = ?, age = ?, gender = ?, specialization = ?, "
        "available = ?, max_patients = ? WHERE doctor_id = ?");
    stmt->setString(1, doctor.name());
    stmt->setInt(2, doctor.age());
    stmt->setString(3, doctor.gender());
    stmt->setString(4, doctor.specialization());
    stmt->setBoolean(5, doctor.isAvailable());
    stmt->setInt(6, doctor.maxPatients());
    stmt->setInt(7, doctor.id());
    return stmt->executeUpdate() > 0;
}

bool DoctorRepository::remove(int doctorId) {
    auto stmt = db_.prepare("DELETE FROM doctors WHERE doctor_id = ?");
    stmt->setInt(1, doctorId);
    return stmt->executeUpdate() > 0;
}

bool DoctorRepository::setAvailability(int doctorId, bool available) {
    auto stmt = db_.prepare("UPDATE doctors SET available = ? WHERE doctor_id = ?");
    stmt->setBoolean(1, available);
    stmt->setInt(2, doctorId);
    return stmt->executeUpdate() > 0;
}

} // namespace medipriority
