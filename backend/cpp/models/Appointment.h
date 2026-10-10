#ifndef MEDIPRIORITY_APPOINTMENT_H
#define MEDIPRIORITY_APPOINTMENT_H

#include <string>

namespace medipriority {

class Appointment {
private:
    int appointmentId_;
    int patientId_;
    int doctorId_;
    long scheduledTime_;
    std::string status_; // "Scheduled", "Completed", "Cancelled"

public:
    Appointment(int appointmentId, int patientId, int doctorId, long scheduledTime,
                std::string status = "Scheduled")
        : appointmentId_(appointmentId), patientId_(patientId), doctorId_(doctorId),
          scheduledTime_(scheduledTime), status_(std::move(status)) {}

    int appointmentId() const { return appointmentId_; }
    int patientId() const { return patientId_; }
    int doctorId() const { return doctorId_; }
    long scheduledTime() const { return scheduledTime_; }
    const std::string &status() const { return status_; }

    void cancel() { status_ = "Cancelled"; }
    void complete() { status_ = "Completed"; }
};

} // namespace medipriority

#endif // MEDIPRIORITY_APPOINTMENT_H
