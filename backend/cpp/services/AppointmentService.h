#ifndef MEDIPRIORITY_APPOINTMENTSERVICE_H
#define MEDIPRIORITY_APPOINTMENTSERVICE_H

extern "C" {
#include "queue.h"
}
#include "../repositories/AppointmentRepository.h"
#include <mutex>
#include <string>
#include <vector>

namespace medipriority {

struct AppointmentBookResult {
    bool success = false;
    std::string message;
    int appointmentId = -1;
};

/*
 * AppointmentService integrates the C FIFO AppointmentQueue (queue.c) --
 * demonstrating a strict "next in line" booking order, distinct from the
 * emergency heap's reprioritization -- with MySQL persistence. The queue
 * holds only appointments scheduled during this run; MySQL holds the full
 * durable appointment history, including completed/cancelled ones.
 */
class AppointmentService {
private:
    AppointmentRepository &repo_;
    AppointmentQueue *queue_;
    std::mutex mutex_;

public:
    explicit AppointmentService(AppointmentRepository &repo);
    ~AppointmentService();

    void loadFromDatabase(); // re-enqueue upcoming appointments at startup

    AppointmentBookResult bookAppointment(int patientId, int doctorId, long scheduledTime);

    /* Removes (and returns) the next appointment in FIFO order from the
     * in-memory queue -- e.g. for a front-desk "call next patient" flow. */
    bool callNext(Appointment &out);

    std::vector<Appointment> listUpcoming();
    bool cancelAppointment(int appointmentId);
};

} // namespace medipriority

#endif // MEDIPRIORITY_APPOINTMENTSERVICE_H
