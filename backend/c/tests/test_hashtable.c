#include "hashtable.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

int main(void) {
    /* Small bucket count on purpose, to force collisions and verify chaining works. */
    PatientHashTable *ht = ht_create(4);
    assert(ht != NULL);

    PatientRecord r1 = {.patient_id = 1001, .age = 34};
    strcpy(r1.name, "Aditi Sharma");
    strcpy(r1.phone, "9876500001");

    PatientRecord r2 = {.patient_id = 1005, .age = 51}; /* likely collides with 1001 in a 4-bucket table */
    strcpy(r2.name, "Rohan Verma");
    strcpy(r2.phone, "9876500002");

    PatientRecord r3 = {.patient_id = 1009, .age = 22};
    strcpy(r3.name, "Meera Nair");
    strcpy(r3.phone, "9876500003");

    assert(ht_insert(ht, r1) == 1);
    assert(ht_insert(ht, r2) == 1);
    assert(ht_insert(ht, r3) == 1);
    assert(ht_count(ht) == 3);

    PatientRecord found;
    assert(ht_search(ht, 1005, &found) == 1);
    printf("found patient 1005: %s, age %d, phone %s\n", found.name, found.age, found.phone);
    assert(strcmp(found.name, "Rohan Verma") == 0);

    /* Edge case: searching for a patient_id that was never inserted. */
    assert(ht_search(ht, 9999, &found) == 0);

    /* Update-in-place: re-inserting the same patient_id must overwrite, not duplicate. */
    PatientRecord r1_updated = r1;
    r1_updated.age = 35;
    strcpy(r1_updated.phone, "9876500099");
    assert(ht_insert(ht, r1_updated) == 1);
    assert(ht_count(ht) == 3); /* count unchanged: overwrite, not a new entry */
    assert(ht_search(ht, 1001, &found) == 1);
    assert(found.age == 35);

    /* Deletion. */
    assert(ht_delete(ht, 1005) == 1);
    assert(ht_count(ht) == 2);
    assert(ht_search(ht, 1005, &found) == 0);
    assert(ht_delete(ht, 1005) == 0); /* deleting again fails cleanly */

    ht_destroy(ht);
    ht_destroy(NULL);

    printf("test_hashtable: ALL TESTS PASSED\n");
    return 0;
}
