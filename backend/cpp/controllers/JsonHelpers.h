#ifndef MEDIPRIORITY_JSONHELPERS_H
#define MEDIPRIORITY_JSONHELPERS_H

#include "crow_all.h"
#include "../models/Patient.h"
#include "../models/Doctor.h"
#include "../models/EmergencyCase.h"
#include "../models/Appointment.h"
#include "../models/MedicalRecord.h"
#include "../models/Ambulance.h"
#include "../models/Bed.h"
#include "../repositories/ReportRepository.h"

namespace medipriority {

/* Converts model objects into crow::json::wvalue for API responses.
 * Kept in one place so every controller formats the same entity the
 * same way, and so database internals (auth tokens, raw column names)
 * never leak straight into a response by accident. */

inline crow::json::wvalue toJson(const Patient &p) {
    crow::json::wvalue j;
    j["patient_id"] = p.id();
    j["name"] = p.name();
    j["age"] = p.age();
    j["gender"] = p.gender();
    j["phone"] = p.phone();
    j["blood_group"] = p.bloodGroup();
    j["emergency_contact"] = p.emergencyContact();
    j["date_of_birth"] = p.dateOfBirth();
    return j;
}

inline crow::json::wvalue toJson(const Doctor &d) {
    crow::json::wvalue j;
    j["doctor_id"] = d.id();
    j["name"] = d.name();
    j["age"] = d.age();
    j["gender"] = d.gender();
    j["specialization"] = d.specialization();
    j["available"] = d.isAvailable();
    j["max_patients"] = d.maxPatients();
    j["current_patients"] = d.currentPatients();
    j["free_slots"] = d.freeSlots();
    j["accepting"] = d.acceptingPatients();
    return j;
}

inline crow::json::wvalue toJson(const EmergencyCase &e) {
    crow::json::wvalue j;
    j["case_id"] = e.caseId();
    j["patient_id"] = e.patientId();
    j["severity"] = e.severity();
    j["severity_label"] = severityLabel(e.severity());
    j["category"] = e.category();
    j["arrival_time"] = e.arrivalTime();
    j["status"] = e.status();
    j["assigned_doctor_id"] = e.assignedDoctorId();
    j["assigned_bed_id"] = e.assignedBedId();
    j["assigned_ambulance_id"] = e.assignedAmbulanceId();
    return j;
}

inline crow::json::wvalue toJson(const Appointment &a) {
    crow::json::wvalue j;
    j["appointment_id"] = a.appointmentId();
    j["patient_id"] = a.patientId();
    j["doctor_id"] = a.doctorId();
    j["scheduled_time"] = a.scheduledTime();
    j["status"] = a.status();
    return j;
}

inline crow::json::wvalue toJson(const MedicalRecord &m) {
    crow::json::wvalue j;
    j["record_id"] = m.recordId();
    j["patient_id"] = m.patientId();
    j["diagnosis"] = m.diagnosis();
    j["notes"] = m.notes();
    j["timestamp"] = m.timestamp();
    return j;
}

inline crow::json::wvalue toJson(const Ambulance &a) {
    crow::json::wvalue j;
    j["ambulance_id"] = a.ambulanceId();
    j["vehicle_number"] = a.vehicleNumber();
    j["status"] = a.status();
    return j;
}

inline crow::json::wvalue toJson(const Bed &b) {
    crow::json::wvalue j;
    j["bed_id"] = b.bedId();
    j["bed_type"] = b.bedType();
    j["occupied"] = b.isOccupied();
    j["occupied_by_patient_id"] = b.occupiedByPatientId();
    return j;
}

inline crow::json::wvalue toJson(const DashboardSummary &s) {
    crow::json::wvalue j;
    j["total_patients"] = s.totalPatients;
    j["total_emergency_cases_waiting"] = s.totalEmergencyCases;
    j["priority_counts"]["critical"] = s.criticalCount;
    j["priority_counts"]["high"] = s.highCount;
    j["priority_counts"]["moderate"] = s.moderateCount;
    j["priority_counts"]["low"] = s.lowCount;
    j["doctors"]["total"] = s.totalDoctors;
    j["doctors"]["available"] = s.availableDoctors;
    j["beds"]["occupied"] = s.occupiedBeds;
    j["beds"]["available"] = s.availableBeds;
    j["icu_beds"]["occupied"] = s.occupiedIcuBeds;
    j["icu_beds"]["available"] = s.availableIcuBeds;
    j["ambulances"]["total"] = s.totalAmbulances;
    j["ambulances"]["available"] = s.availableAmbulances;
    j["appointments"]["upcoming"] = s.upcomingAppointments;
    return j;
}

inline crow::json::wvalue toJson(const AnalyticsData &a) {
    crow::json::wvalue j;

    std::vector<crow::json::wvalue> daily;
    for (const auto &d : a.dailyArrivals) {
        crow::json::wvalue row;
        row["date"] = d.date;
        row["count"] = d.count;
        daily.push_back(std::move(row));
    }
    j["daily_arrivals"] = std::move(daily);

    std::vector<crow::json::wvalue> workload;
    for (const auto &w : a.topDoctorWorkload) {
        crow::json::wvalue row;
        row["doctor_name"] = w.doctorName;
        row["active_cases"] = w.activeCases;
        workload.push_back(std::move(row));
    }
    j["doctor_workload"] = std::move(workload);

    std::vector<crow::json::wvalue> waits;
    for (const auto &wtime : a.waitTimesBySeverity) {
        crow::json::wvalue row;
        row["severity"] = wtime.severity;
        row["severity_label"] = severityLabel(wtime.severity);
        row["avg_wait_minutes"] = wtime.avgWaitMinutes;
        row["waiting_count"] = wtime.waitingCount;
        waits.push_back(std::move(row));
    }
    j["wait_times"] = std::move(waits);

    std::vector<crow::json::wvalue> beds;
    for (const auto &b : a.bedOccupancyByType) {
        crow::json::wvalue row;
        row["bed_type"] = b.bedType;
        row["occupied"] = b.occupied;
        row["total"] = b.total;
        beds.push_back(std::move(row));
    }
    j["bed_occupancy_by_type"] = std::move(beds);

    j["appointments"]["scheduled"] = a.scheduledAppointments;
    j["appointments"]["completed"] = a.completedAppointments;
    j["appointments"]["cancelled"] = a.cancelledAppointments;

    return j;
}

inline crow::json::wvalue toJson(const std::vector<Alert> &alerts) {
    std::vector<crow::json::wvalue> rows;
    for (const auto &al : alerts) {
        crow::json::wvalue row;
        row["level"] = al.level;
        row["message"] = al.message;
        rows.push_back(std::move(row));
    }
    crow::json::wvalue j;
    j = std::move(rows);
    return j;
}

inline crow::response errorResponse(int code, const std::string &message) {
    crow::json::wvalue j;
    j["success"] = false;
    j["message"] = message;
    crow::response res(code, j);
    res.set_header("Content-Type", "application/json");
    return res;
}

} // namespace medipriority

#endif // MEDIPRIORITY_JSONHELPERS_H
