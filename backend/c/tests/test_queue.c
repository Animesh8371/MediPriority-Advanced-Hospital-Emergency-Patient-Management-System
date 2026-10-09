#include "queue.h"
#include <stdio.h>
#include <assert.h>

int main(void) {
    AppointmentQueue *q = queue_create(2); /* small capacity to exercise auto-resize */
    assert(q != NULL);
    assert(queue_is_empty(q) == 1);

    Appointment out;

    /* Edge case: dequeue/peek on an empty queue must fail cleanly. */
    assert(queue_dequeue(q, &out) == 0);
    assert(queue_peek(q, &out) == 0);

    queue_enqueue(q, (Appointment){.appointment_id = 1, .patient_id = 201, .doctor_id = 1, .scheduled_time = 100});
    queue_enqueue(q, (Appointment){.appointment_id = 2, .patient_id = 202, .doctor_id = 2, .scheduled_time = 110});
    queue_enqueue(q, (Appointment){.appointment_id = 3, .patient_id = 203, .doctor_id = 1, .scheduled_time = 120});
    /* 3rd insert forces the internal array to grow past initial capacity of 2. */
    assert(queue_size(q) == 3);

    assert(queue_peek(q, &out) == 1);
    assert(out.appointment_id == 1); /* FIFO: first booked, still first in line */

    assert(queue_dequeue(q, &out) == 1);
    printf("dequeued: appointment_id=%d patient_id=%d\n", out.appointment_id, out.patient_id);
    assert(out.appointment_id == 1);

    assert(queue_dequeue(q, &out) == 1);
    printf("dequeued: appointment_id=%d patient_id=%d\n", out.appointment_id, out.patient_id);
    assert(out.appointment_id == 2);

    queue_enqueue(q, (Appointment){.appointment_id = 4, .patient_id = 204, .doctor_id = 3, .scheduled_time = 130});

    assert(queue_dequeue(q, &out) == 1);
    assert(out.appointment_id == 3);
    assert(queue_dequeue(q, &out) == 1);
    assert(out.appointment_id == 4);

    assert(queue_is_empty(q) == 1);
    assert(queue_dequeue(q, &out) == 0); /* still fails cleanly once empty again */

    queue_destroy(q);
    queue_destroy(NULL);

    printf("test_queue: ALL TESTS PASSED\n");
    return 0;
}
