#ifndef MEDIPRIORITY_HASHTABLE_H
#define MEDIPRIORITY_HASHTABLE_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * This hash table is an IN-MEMORY, per-process index used for fast
 * patient lookups during a running session (e.g. quickly checking
 * "have we already seen this patient ID today?"). MySQL remains the
 * single source of truth: on restart, this table is empty and must
 * be repopulated (e.g. by the C++ repository layer loading recent
 * patients on startup). This table is a cache/index, NOT a
 * replacement for the database.
 */
typedef struct {
    int  patient_id;
    char name[64];
    int  age;
    char phone[16];
} PatientRecord;

typedef struct HTNode {
    PatientRecord record;
    struct HTNode *next;
} HTNode;

typedef struct {
    HTNode **buckets;
    int bucket_count;
    int count;
} PatientHashTable;

PatientHashTable *ht_create(int bucket_count);
void ht_destroy(PatientHashTable *ht);

/* Insert or overwrite a patient record by patient_id. Returns 1 on success. */
int ht_insert(PatientHashTable *ht, PatientRecord r);

/* Look up by patient_id. Returns 1 and fills *out if found, else 0. */
int ht_search(PatientHashTable *ht, int patient_id, PatientRecord *out);

/* Remove a record by patient_id. Returns 1 if found and removed, else 0. */
int ht_delete(PatientHashTable *ht, int patient_id);

int ht_count(PatientHashTable *ht);

#ifdef __cplusplus
}
#endif

#endif /* MEDIPRIORITY_HASHTABLE_H */
