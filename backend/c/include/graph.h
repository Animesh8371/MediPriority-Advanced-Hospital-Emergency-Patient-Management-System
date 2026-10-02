#ifndef MEDIPRIORITY_GRAPH_H
#define MEDIPRIORITY_GRAPH_H

#ifdef __cplusplus
extern "C" {
#endif

/* Maximum number of hospitals in the SIMULATED transfer network.
 * This is an academic-scale constant, not a real hospital network. */
#define MAX_HOSPITALS 32

/* A value used to represent "no direct edge" between two hospitals. */
#define GRAPH_NO_EDGE -1.0

typedef struct {
    int    num_hospitals;
    char   names[MAX_HOSPITALS][64];
    double adj[MAX_HOSPITALS][MAX_HOSPITALS]; /* adj[i][j] = weight, or GRAPH_NO_EDGE */
} HospitalGraph;

HospitalGraph *graph_create(void);
void graph_destroy(HospitalGraph *g);

/* Add a hospital, returns its index (0-based), or -1 if the graph is full
 * or the name is invalid. */
int graph_add_hospital(HospitalGraph *g, const char *name);

/* Add an undirected edge (a transfer route works both ways) with a
 * simulated distance/cost weight (must be > 0). Returns 1 on success. */
int graph_add_edge(HospitalGraph *g, int from_idx, int to_idx, double weight);

/*
 * Runs Dijkstra's algorithm from source_idx to dest_idx.
 * On success (a path exists): fills out_path (array of hospital indices,
 * source first, destination last), sets *out_path_len to the number of
 * hospitals in the path, sets *out_total_cost to the total path weight,
 * and returns 1.
 * If the destination is unreachable, returns 0 and leaves the out
 * parameters unspecified.
 * out_path must have capacity >= MAX_HOSPITALS.
 */
int dijkstra_shortest_path(HospitalGraph *g, int source_idx, int dest_idx,
                            int *out_path, int *out_path_len, double *out_total_cost);

#ifdef __cplusplus
}
#endif

#endif /* MEDIPRIORITY_GRAPH_H */
