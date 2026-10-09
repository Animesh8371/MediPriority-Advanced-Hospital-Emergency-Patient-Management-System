#include "heap.h"
#include <stdio.h>
#include <assert.h>

/* severity: 1=Critical 2=High 3=Moderate 4=Low */

int main(void) {
    MinHeap *h = heap_create(4);
    assert(h != NULL);
    assert(heap_is_empty(h) == 1);

    /* Insert out of order: expect extraction in severity order,
     * and FIFO among equal severities (tie-break by arrival_time). */
    heap_insert(h, (EmergencyCase){.case_id = 1, .patient_id = 101, .severity = 3, .arrival_time = 10});
    heap_insert(h, (EmergencyCase){.case_id = 2, .patient_id = 102, .severity = 1, .arrival_time = 20});
    heap_insert(h, (EmergencyCase){.case_id = 3, .patient_id = 103, .severity = 1, .arrival_time = 5});
    heap_insert(h, (EmergencyCase){.case_id = 4, .patient_id = 104, .severity = 2, .arrival_time = 15});
    heap_insert(h, (EmergencyCase){.case_id = 5, .patient_id = 105, .severity = 4, .arrival_time = 1});

    assert(heap_size(h) == 5);

    EmergencyCase ec;

    /* Both case 2 and case 3 are Critical (severity=1). Case 3 arrived
     * earlier (arrival_time=5 < 20), so it must come out first. */
    assert(heap_extract_min(h, &ec) == 1);
    printf("1st out: case_id=%d severity=%d arrival=%ld\n", ec.case_id, ec.severity, ec.arrival_time);
    assert(ec.case_id == 3);

    assert(heap_extract_min(h, &ec) == 1);
    printf("2nd out: case_id=%d severity=%d arrival=%ld\n", ec.case_id, ec.severity, ec.arrival_time);
    assert(ec.case_id == 2);

    /* Next should be the High severity case. */
    assert(heap_extract_min(h, &ec) == 1);
    printf("3rd out: case_id=%d severity=%d arrival=%ld\n", ec.case_id, ec.severity, ec.arrival_time);
    assert(ec.case_id == 4);

    /* Worsening: case 5 changes from Low to Critical. */
    assert(heap_update_priority(h, 5, 1) == 1);
    assert(heap_peek(h, &ec) == 1);
    assert(ec.case_id == 5);
    assert(ec.severity == 1);

    /* Improving: case 5 changes from Critical to Low. */
    assert(heap_update_priority(h, 5, 4) == 1);

    /* Case 1 is now the remaining Moderate case and should be next. */
    assert(heap_peek(h, &ec) == 1);
    assert(ec.case_id == 1);
    assert(ec.severity == 3);

    /* Updating a case that does not exist must fail. */
    assert(heap_update_priority(h, 999, 1) == 0);
    /* Test remove_case: remove case_id=1 (Moderate) directly. */
    assert(heap_remove_case(h, 1) == 1);
    assert(heap_remove_case(h, 999) == 0); /* removing a non-existent id fails cleanly */

    /* Drain remaining and confirm heap becomes empty afterward. */
    int drained = 0;
    while (!heap_is_empty(h)) {
        assert(heap_extract_min(h, &ec) == 1);
        printf("drain: case_id=%d severity=%d\n", ec.case_id, ec.severity);
        drained++;
    }
    /* Only case 5 should remain: case 1 was removed, cases 2/3/4 already extracted. */
    assert(drained == 1);

    /* Edge case: extracting/peeking from an empty heap must fail cleanly, not crash. */
    assert(heap_is_empty(h) == 1);
    assert(heap_extract_min(h, &ec) == 0);
    assert(heap_peek(h, &ec) == 0);

    heap_destroy(h);
    heap_destroy(NULL); /* must not crash on NULL */

    printf("test_heap: ALL TESTS PASSED\n");
    return 0;
}
