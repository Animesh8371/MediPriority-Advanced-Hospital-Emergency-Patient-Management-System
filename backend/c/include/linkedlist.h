#ifndef MEDIPRIORITY_LINKEDLIST_H
#define MEDIPRIORITY_LINKEDLIST_H

#ifdef __cplusplus
extern "C" {
#endif

/* One entry in a patient's medical history. This is a simulated,
 * non-authoritative record for academic demonstration only. */
typedef struct {
    int  record_id;
    int  patient_id;
    char diagnosis[128];
    char notes[256];
    long timestamp; /* used to keep chronological order */
} MedicalHistoryEntry;

typedef struct MHNode {
    MedicalHistoryEntry entry;
    struct MHNode *next;
} MHNode;

/* Singly linked list, append-only at the tail so traversal from head
 * to tail is naturally chronological (oldest -> newest), as long as
 * entries are appended with non-decreasing timestamps. */
typedef struct {
    MHNode *head;
    MHNode *tail;
    int count;
} MedicalHistoryList;

MedicalHistoryList *mhlist_create(void);
void mhlist_destroy(MedicalHistoryList *list);

/* Append a new entry at the tail (preserves prior entries). Returns 1 on success. */
int mhlist_append(MedicalHistoryList *list, MedicalHistoryEntry e);

/* Copy all entries belonging to patient_id, in chronological order, into
 * out_array (caller-allocated, capacity max_out). Returns the number copied. */
int mhlist_get_for_patient(MedicalHistoryList *list, int patient_id,
                            MedicalHistoryEntry *out_array, int max_out);

int mhlist_count(MedicalHistoryList *list);

#ifdef __cplusplus
}
#endif

#endif /* MEDIPRIORITY_LINKEDLIST_H */
