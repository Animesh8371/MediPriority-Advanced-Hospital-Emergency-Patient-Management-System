#include "PatientRepository.h"
#include <cppconn/resultset.h>
#include <cppconn/exception.h>
#include <memory>

namespace medipriority {

static Patient rowToPatient(sql::ResultSet *rs) {
    return Patient(
        rs->getInt("patient_id"),
        rs->getString("name"),
        rs->getInt("age"),
        rs->getString("gender"),
        rs->getString("phone"),
        rs->getString("blood_group"),
        rs->isNull("emergency_contact") ? "" : rs->getString("emergency_contact"),
        rs->isNull("date_of_birth") ? "" : rs->getString("date_of_birth")
    );
}

std::optional<Patient> PatientRepository::findById(int patientId) {
    auto stmt = db_.prepare(
        "SELECT patient_id, name, age, gender, phone, blood_group, "
        "emergency_contact, date_of_birth FROM patients WHERE patient_id = ?");
    stmt->setInt(1, patientId);
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    if (rs->next()) return rowToPatient(rs.get());
    return std::nullopt;
}

std::vector<Patient> PatientRepository::searchByName(const std::string &namePart, int limit) {
    auto stmt = db_.prepare(
        "SELECT patient_id, name, age, gender, phone, blood_group, "
        "emergency_contact, date_of_birth FROM patients "
        "WHERE name LIKE ? ORDER BY name LIMIT ?");
    stmt->setString(1, "%" + namePart + "%");
    stmt->setInt(2, limit);
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());

    std::vector<Patient> results;
    while (rs->next()) results.push_back(rowToPatient(rs.get()));
    return results;
}

bool PatientRepository::existsSimilar(const std::string &name, const std::string &phone) {
    auto stmt = db_.prepare(
        "SELECT COUNT(*) AS cnt FROM patients WHERE name = ? AND phone = ?");
    stmt->setString(1, name);
    stmt->setString(2, phone);
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    rs->next();
    return rs->getInt("cnt") > 0;
}

PatientCreateResult PatientRepository::create(const Patient &patient) {
    PatientCreateResult result;

    auto stmt = db_.prepare(
        "INSERT INTO patients (name, age, gender, phone, blood_group, "
        "emergency_contact, date_of_birth) VALUES (?, ?, ?, ?, ?, ?, ?)");
    stmt->setString(1, patient.name());
    stmt->setInt(2, patient.age());
    stmt->setString(3, patient.gender());
    stmt->setString(4, patient.phone());
    stmt->setString(5, patient.bloodGroup());
    if (patient.emergencyContact().empty()) stmt->setNull(6, sql::DataType::VARCHAR);
    else stmt->setString(6, patient.emergencyContact());
    if (patient.dateOfBirth().empty()) stmt->setNull(7, sql::DataType::DATE);
    else stmt->setString(7, patient.dateOfBirth());

    stmt->executeUpdate();

    auto idStmt = db_.prepare("SELECT LAST_INSERT_ID() AS id");
    std::unique_ptr<sql::ResultSet> rs(idStmt->executeQuery());
    rs->next();

    result.success = true;
    result.patientId = rs->getInt("id");
    result.message = "Patient registered successfully";
    return result;
}

std::vector<Patient> PatientRepository::findAll(int limit) {
    auto stmt = db_.prepare(
        "SELECT patient_id, name, age, gender, phone, blood_group, "
        "emergency_contact, date_of_birth FROM patients ORDER BY patient_id DESC LIMIT ?");
    stmt->setInt(1, limit);
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());

    std::vector<Patient> results;
    while (rs->next()) results.push_back(rowToPatient(rs.get()));
    return results;
}

} // namespace medipriority
