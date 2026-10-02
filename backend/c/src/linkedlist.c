#include "linkedlist.h"
#include <stdlib.h>

MedicalHistoryList *mhlist_create(void) {
    MedicalHistoryList *list = (MedicalHistoryList *)malloc(sizeof(MedicalHistoryList));
    if (!list) return NULL;
    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
    return list;
}

void mhlist_destroy(MedicalHistoryList *list) {
    if (!list) return;
    MHNode *node = list->head;
    while (node) {
        MHNode *next = node->next;
        free(node);
        node = next;
    }
    free(list);
}

int mhlist_append(MedicalHistoryList *list, MedicalHistoryEntry e) {
    if (!list) return 0;
    MHNode *node = (MHNode *)malloc(sizeof(MHNode));
    if (!node) return 0;
    node->entry = e;
    node->next = NULL;

    if (!list->head) {
        list->head = node;
        list->tail = node;
    } else {
        list->tail->next = node;
        list->tail = node;
    }
    list->count++;
    return 1;
}

int mhlist_get_for_patient(MedicalHistoryList *list, int patient_id,
                            MedicalHistoryEntry *out_array, int max_out) {
    if (!list || !out_array || max_out <= 0) return 0;
    int found = 0;
    for (MHNode *node = list->head; node && found < max_out; node = node->next) {
        if (node->entry.patient_id == patient_id) {
            out_array[found] = node->entry;
            found++;
        }
    }
    return found;
}

int mhlist_count(MedicalHistoryList *list) {
    return list ? list->count : 0;
}

/*
 * Complexity notes:
 *   mhlist_append           O(1) (tail pointer maintained)
 *   mhlist_get_for_patient  O(n) -- must scan all entries to filter by patient
 * n = total medical history entries across all patients in this in-memory list.
 * (MySQL, in contrast, can index by patient_id for O(log n) retrieval --
 * this is exactly the trade-off explained in Module 2/9: the C structure
 * demonstrates the algorithmic concept, MySQL is the real persistent store.)
 */
