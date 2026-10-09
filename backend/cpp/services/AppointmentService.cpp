#include "AppointmentService.h"

namespace medipriority {

AppointmentService::AppointmentService(AppointmentRepository &repo) : repo_(repo) {
    queue_ = queue_create(16);
}

AppointmentService::~AppointmentService() {
    queue_destroy(queue_);
}

void AppointmentService::loadFromDatabase() {
    std::lock_guard<std::mutex> lock(mutex_);
    auto upcoming = repo_.findUpcoming(200);
    for (const auto &a : upcoming) {
        ::Appointment cAppt;
        cAppt.appointment_id = a.appointmentId();
        cAppt.patient_id = a.patientId();
        cAppt.doctor_id = a.doctorId();
        cAppt.scheduled_time = a.scheduledTime();
        queue_enqueue(queue_, cAppt);
    }
}

AppointmentBookResult AppointmentService::bookAppointment(int patientId, int doctorId, long scheduledTime) {
    AppointmentBookResult result;

    if (repo_.hasConflict(doctorId, scheduledTime)) {
        result.message = "This doctor already has an appointment at that time";
        return result;
    }

    Appointment appt(0, patientId, doctorId, scheduledTime);
    int appointmentId = repo_.create(appt);

    {
        std::lock_guard<std::mutex> lock(mutex_);
        ::Appointment cAppt;
        cAppt.appointment_id = appointmentId;
        cAppt.patient_id = patientId;
        cAppt.doctor_id = doctorId;
        cAppt.scheduled_time = scheduledTime;
        queue_enqueue(queue_, cAppt);
    }

    result.success = true;
    result.appointmentId = appointmentId;
    result.message = "Appointment booked";
    return result;
}

bool AppointmentService::callNext(Appointment &out) {
    std::lock_guard<std::mutex> lock(mutex_);
    ::Appointment cAppt;
    if (!queue_dequeue(queue_, &cAppt)) return false;
    out = Appointment(cAppt.appointment_id, cAppt.patient_id, cAppt.doctor_id, cAppt.scheduled_time);
    return true;
}

std::vector<Appointment> AppointmentService::listUpcoming() {
    return repo_.findUpcoming();
}

bool AppointmentService::cancelAppointment(int appointmentId) {
    return repo_.cancel(appointmentId);
}

} // namespace medipriority
