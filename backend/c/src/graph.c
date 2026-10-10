#include "graph.h"
#include <stdlib.h>
#include <string.h>

HospitalGraph *graph_create(void) {
    HospitalGraph *g = (HospitalGraph *)malloc(sizeof(HospitalGraph));
    if (!g) return NULL;
    g->num_hospitals = 0;
    for (int i = 0; i < MAX_HOSPITALS; i++) {
        for (int j = 0; j < MAX_HOSPITALS; j++) {
            g->adj[i][j] = GRAPH_NO_EDGE;
        }
    }
    return g;
}

void graph_destroy(HospitalGraph *g) {
    free(g);
}

int graph_add_hospital(HospitalGraph *g, const char *name) {
    if (!g || !name || g->num_hospitals >= MAX_HOSPITALS) return -1;
    int idx = g->num_hospitals;
    strncpy(g->names[idx], name, sizeof(g->names[idx]) - 1);
    g->names[idx][sizeof(g->names[idx]) - 1] = '\0';
    g->num_hospitals++;
    return idx;
}

int graph_add_edge(HospitalGraph *g, int from_idx, int to_idx, double weight) {
    if (!g) return 0;
    if (from_idx < 0 || from_idx >= g->num_hospitals) return 0;
    if (to_idx < 0 || to_idx >= g->num_hospitals) return 0;
    if (weight <= 0) return 0;

    g->adj[from_idx][to_idx] = weight;
    g->adj[to_idx][from_idx] = weight; /* undirected: transfer routes work both ways */
    return 1;
}
