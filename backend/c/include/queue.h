#ifndef MEDIPRIORITY_QUEUE_H
#define MEDIPRIORITY_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

/* A normal (non-emergency) appointment, ordered strictly FIFO
 * by the order it was booked -- unlike emergency cases, appointments
 * do not get reprioritized. */
typedef struct {
    int  appointment_id;
    int  patient_id;
    int  doctor_id;
    long scheduled_time; /* unix-like timestamp or simulated counter */
} Appointment;

/* Circular array-based FIFO queue, auto-resizing. */
typedef struct {
    Appointment *data;
    int front;
    int rear;
    int size;
    int capacity;
} AppointmentQueue;

AppointmentQueue *queue_create(int initial_capacity);
void queue_destroy(AppointmentQueue *q);

/* Add an appointment to the back of the queue. Returns 1 on success. */
int queue_enqueue(AppointmentQueue *q, Appointment a);

/* Remove and return the appointment at the front. Returns 1 on success, 0 if empty. */
int queue_dequeue(AppointmentQueue *q, Appointment *out);

/* Look at the front appointment without removing it. Returns 1 on success, 0 if empty. */
int queue_peek(AppointmentQueue *q, Appointment *out);

int queue_is_empty(AppointmentQueue *q);
int queue_size(AppointmentQueue *q);

#ifdef __cplusplus
}
#endif

#endif /* MEDIPRIORITY_QUEUE_H */
