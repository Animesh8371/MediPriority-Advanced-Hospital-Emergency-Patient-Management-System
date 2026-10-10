#include "heap.h"
#include <stdlib.h>

static long compute_priority_key(int severity, long arrival_time) {
    return (long)severity * 1000000000L + arrival_time;
}

static void swap_cases(EmergencyCase *a, EmergencyCase *b) {
    EmergencyCase tmp = *a;
    *a = *b;
    *b = tmp;
}

static void sift_up(MinHeap *h, int idx) {
    while (idx > 0) {
        int parent = (idx - 1) / 2;
        if (h->data[parent].priority_key <= h->data[idx].priority_key) break;
        swap_cases(&h->data[parent], &h->data[idx]);
        idx = parent;
    }
}

static void sift_down(MinHeap *h, int idx) {
    for (;;) {
        int left = 2 * idx + 1;
        int right = 2 * idx + 2;
        int smallest = idx;

        if (left < h->size && h->data[left].priority_key < h->data[smallest].priority_key)
            smallest = left;
        if (right < h->size && h->data[right].priority_key < h->data[smallest].priority_key)
            smallest = right;

        if (smallest == idx) break;
        swap_cases(&h->data[idx], &h->data[smallest]);
        idx = smallest;
    }
}

static int find_index_by_case_id(MinHeap *h, int case_id) {
    /* Linear scan: O(n). Acceptable for MVP scale (a hospital's live
     * emergency queue is small). A production system might keep a
     * case_id -> index map alongside the heap for O(1) lookup. */
    for (int i = 0; i < h->size; i++) {
        if (h->data[i].case_id == case_id) return i;
    }
    return -1;
}

MinHeap *heap_create(int initial_capacity) {
    if (initial_capacity < 4) initial_capacity = 4;
    MinHeap *h = (MinHeap *)malloc(sizeof(MinHeap));
    if (!h) return NULL;
    h->data = (EmergencyCase *)malloc(sizeof(EmergencyCase) * initial_capacity);
    if (!h->data) {
        free(h);
        return NULL;
    }
    h->size = 0;
    h->capacity = initial_capacity;
    return h;
}

void heap_destroy(MinHeap *h) {
    if (!h) return;
    free(h->data);
    free(h);
}

static int ensure_capacity(MinHeap *h) {
    if (h->size < h->capacity) return 1;
    int new_capacity = h->capacity * 2;
    EmergencyCase *new_data = (EmergencyCase *)realloc(h->data, sizeof(EmergencyCase) * new_capacity);
    if (!new_data) return 0;
    h->data = new_data;
    h->capacity = new_capacity;
    return 1;
}

int heap_insert(MinHeap *h, EmergencyCase ec) {
    if (!h) return 0;
    if (!ensure_capacity(h)) return 0;

    ec.priority_key = compute_priority_key(ec.severity, ec.arrival_time);
    h->data[h->size] = ec;
    sift_up(h, h->size);
    h->size++;
    return 1;
}

int heap_extract_min(MinHeap *h, EmergencyCase *out) {
    if (!h || h->size == 0) return 0;
    if (out) *out = h->data[0];
    h->size--;
    h->data[0] = h->data[h->size];
    if (h->size > 0) sift_down(h, 0);
    return 1;
}

int heap_peek(MinHeap *h, EmergencyCase *out) {
    if (!h || h->size == 0) return 0;
    if (out) *out = h->data[0];
    return 1;
}

int heap_is_empty(MinHeap *h) {
    return (!h || h->size == 0) ? 1 : 0;
}

int heap_size(MinHeap *h) {
    return h ? h->size : 0;
}

int heap_update_priority(MinHeap *h, int case_id, int new_severity) {
    if (!h) return 0;
    int idx = find_index_by_case_id(h, case_id);
    if (idx < 0) return 0;

    long old_key = h->data[idx].priority_key;
    h->data[idx].severity = new_severity;
    h->data[idx].priority_key = compute_priority_key(new_severity, h->data[idx].arrival_time);

    if (h->data[idx].priority_key < old_key) {
        sift_up(h, idx);
    } else {
        sift_down(h, idx);
    }
    return 1;
}

int heap_remove_case(MinHeap *h, int case_id) {
    if (!h) return 0;
    int idx = find_index_by_case_id(h, case_id);
    if (idx < 0) return 0;

    h->size--;
    if (idx != h->size) {
        h->data[idx] = h->data[h->size];
        sift_down(h, idx);
        sift_up(h, idx);
    }
    return 1;
}

/*
 * Complexity notes:
 *   heap_insert          O(log n)
 *   heap_extract_min     O(log n)
 *   heap_peek            O(1)
 *   heap_update_priority O(n) lookup + O(log n) re-heapify
 *   heap_remove_case     O(n) lookup + O(log n) re-heapify
 * n = number of cases currently in the emergency queue.
 */
