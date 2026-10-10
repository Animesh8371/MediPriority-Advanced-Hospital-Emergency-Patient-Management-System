#include "queue.h"
#include <stdlib.h>

AppointmentQueue *queue_create(int initial_capacity) {
    if (initial_capacity < 4) initial_capacity = 4;
    AppointmentQueue *q = (AppointmentQueue *)malloc(sizeof(AppointmentQueue));
    if (!q) return NULL;
    q->data = (Appointment *)malloc(sizeof(Appointment) * initial_capacity);
    if (!q->data) {
        free(q);
        return NULL;
    }
    q->front = 0;
    q->rear = -1;
    q->size = 0;
    q->capacity = initial_capacity;
    return q;
}

void queue_destroy(AppointmentQueue *q) {
    if (!q) return;
    free(q->data);
    free(q);
}

static int grow(AppointmentQueue *q) {
    int new_capacity = q->capacity * 2;
    Appointment *new_data = (Appointment *)malloc(sizeof(Appointment) * new_capacity);
    if (!new_data) return 0;

    /* Copy elements out in logical (FIFO) order starting at front. */
    for (int i = 0; i < q->size; i++) {
        new_data[i] = q->data[(q->front + i) % q->capacity];
    }
    free(q->data);
    q->data = new_data;
    q->front = 0;
    q->rear = q->size - 1;
    q->capacity = new_capacity;
    return 1;
}

int queue_enqueue(AppointmentQueue *q, Appointment a) {
    if (!q) return 0;
    if (q->size == q->capacity) {
        if (!grow(q)) return 0;
    }
    q->rear = (q->rear + 1) % q->capacity;
    q->data[q->rear] = a;
    q->size++;
    return 1;
}

int queue_dequeue(AppointmentQueue *q, Appointment *out) {
    if (!q || q->size == 0) return 0;
    if (out) *out = q->data[q->front];
    q->front = (q->front + 1) % q->capacity;
    q->size--;
    return 1;
}

int queue_peek(AppointmentQueue *q, Appointment *out) {
    if (!q || q->size == 0) return 0;
    if (out) *out = q->data[q->front];
    return 1;
}

int queue_is_empty(AppointmentQueue *q) {
    return (!q || q->size == 0) ? 1 : 0;
}

int queue_size(AppointmentQueue *q) {
    return q ? q->size : 0;
}

/*
 * Complexity notes:
 *   queue_enqueue O(1) amortized (occasional O(n) resize)
 *   queue_dequeue O(1)
 *   queue_peek    O(1)
 * n = number of appointments currently queued.
 */
