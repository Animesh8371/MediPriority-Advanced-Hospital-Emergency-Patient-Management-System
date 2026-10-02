#include "hashtable.h"
#include <stdlib.h>
#include <string.h>

static unsigned int hash_patient_id(int patient_id, int bucket_count) {
    /* Simple multiplicative hash, always non-negative. */
    unsigned int key = (unsigned int)patient_id;
    key = key * 2654435761u; /* Knuth's multiplicative constant */
    return key % (unsigned int)bucket_count;
}

PatientHashTable *ht_create(int bucket_count) {
    if (bucket_count < 8) bucket_count = 8;
    PatientHashTable *ht = (PatientHashTable *)malloc(sizeof(PatientHashTable));
    if (!ht) return NULL;
    ht->buckets = (HTNode **)calloc((size_t)bucket_count, sizeof(HTNode *));
    if (!ht->buckets) {
        free(ht);
        return NULL;
    }
    ht->bucket_count = bucket_count;
    ht->count = 0;
    return ht;
}

void ht_destroy(PatientHashTable *ht) {
    if (!ht) return;
    for (int i = 0; i < ht->bucket_count; i++) {
        HTNode *node = ht->buckets[i];
        while (node) {
            HTNode *next = node->next;
            free(node);
            node = next;
        }
    }
    free(ht->buckets);
    free(ht);
}

int ht_insert(PatientHashTable *ht, PatientRecord r) {
    if (!ht) return 0;
    unsigned int idx = hash_patient_id(r.patient_id, ht->bucket_count);

    /* If already present, overwrite in place (do not create a duplicate node). */
    for (HTNode *node = ht->buckets[idx]; node; node = node->next) {
        if (node->record.patient_id == r.patient_id) {
            node->record = r;
            return 1;
        }
    }

    HTNode *new_node = (HTNode *)malloc(sizeof(HTNode));
    if (!new_node) return 0;
    new_node->record = r;
    new_node->next = ht->buckets[idx];
    ht->buckets[idx] = new_node;
    ht->count++;
    return 1;
}

int ht_search(PatientHashTable *ht, int patient_id, PatientRecord *out) {
    if (!ht) return 0;
    unsigned int idx = hash_patient_id(patient_id, ht->bucket_count);
    for (HTNode *node = ht->buckets[idx]; node; node = node->next) {
        if (node->record.patient_id == patient_id) {
            if (out) *out = node->record;
            return 1;
        }
    }
    return 0;
}

int ht_delete(PatientHashTable *ht, int patient_id) {
    if (!ht) return 0;
    unsigned int idx = hash_patient_id(patient_id, ht->bucket_count);
    HTNode *node = ht->buckets[idx];
    HTNode *prev = NULL;
    while (node) {
        if (node->record.patient_id == patient_id) {
            if (prev) prev->next = node->next;
            else ht->buckets[idx] = node->next;
            free(node);
            ht->count--;
            return 1;
        }
        prev = node;
        node = node->next;
    }
    return 0;
}

int ht_count(PatientHashTable *ht) {
    return ht ? ht->count : 0;
}

/*
 * Complexity notes (average case, assuming a reasonable load factor):
 *   ht_insert O(1) average, O(n) worst case (all keys collide)
 *   ht_search O(1) average, O(n) worst case
 *   ht_delete O(1) average, O(n) worst case
 * Collisions are resolved by separate chaining (linked list per bucket).
 */
