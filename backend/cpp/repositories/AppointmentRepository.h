#ifndef MEDIPRIORITY_APPOINTMENTREPOSITORY_H
#define MEDIPRIORITY_APPOINTMENTREPOSITORY_H

#include "../utils/Database.h"
#include "../models/Appointment.h"
#include <optional>
#include <vector>

namespace medipriority {

class AppointmentRepository {
private:
    Database &db_;
public:
    explicit AppointmentRepository(Database &db) : db_(db) {}

    int create(const Appointment &appt);
    std::optional<Appointment> findById(int appointmentId);
    std::vector<Appointment> findUpcoming(int limit = 50);
    std::vector<Appointment> findByPatient(int patientId);

    /* True if this doctor already has a non-cancelled appointment at
     * exactly this scheduled_time (simple conflict check for the MVP). */
    bool hasConflict(int doctorId, long scheduledTime);

    bool cancel(int appointmentId);
    bool complete(int appointmentId);
};

} // namespace medipriority
#endif // MEDIPRIORITY_APPOINTMENTREPOSITORY_H
