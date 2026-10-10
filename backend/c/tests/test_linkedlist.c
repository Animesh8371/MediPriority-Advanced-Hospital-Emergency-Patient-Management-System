#include "linkedlist.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

int main(void) {
    MedicalHistoryList *list = mhlist_create();
    assert(list != NULL);
    assert(mhlist_count(list) == 0);

    MedicalHistoryEntry e1 = {.record_id = 1, .patient_id = 501, .timestamp = 100};
    strcpy(e1.diagnosis, "Seasonal flu");
    strcpy(e1.notes, "Prescribed rest and fluids");

    MedicalHistoryEntry e2 = {.record_id = 2, .patient_id = 501, .timestamp = 200};
    strcpy(e2.diagnosis, "Follow-up checkup");
    strcpy(e2.notes, "Fully recovered");

    MedicalHistoryEntry e3 = {.record_id = 3, .patient_id = 777, .timestamp = 150};
    strcpy(e3.diagnosis, "Sprained ankle");
    strcpy(e3.notes, "Recommended rest, ice, compression");

    assert(mhlist_append(list, e1) == 1);
    assert(mhlist_append(list, e3) == 1); /* different patient, interleaved */
    assert(mhlist_append(list, e2) == 1);

    assert(mhlist_count(list) == 3);

    MedicalHistoryEntry out[10];
    int n = mhlist_get_for_patient(list, 501, out, 10);
    assert(n == 2);
    /* Chronological order preserved for this patient's entries specifically. */
    printf("patient 501 history: [%s @ %ld] -> [%s @ %ld]\n",
           out[0].diagnosis, out[0].timestamp, out[1].diagnosis, out[1].timestamp);
    assert(out[0].timestamp < out[1].timestamp);
    assert(strcmp(out[0].diagnosis, "Seasonal flu") == 0);
    assert(strcmp(out[1].diagnosis, "Follow-up checkup") == 0);

    /* Edge case: patient with no history at all. */
    n = mhlist_get_for_patient(list, 12345, out, 10);
    assert(n == 0);

    /* Existing records must be preserved, never overwritten by new appends. */
    assert(mhlist_count(list) == 3);

    mhlist_destroy(list);
    mhlist_destroy(NULL);

    printf("test_linkedlist: ALL TESTS PASSED\n");
    return 0;
}
