#include "graph.h"
#include <float.h>
#include <string.h>

/*
 * Simple O(V^2) array-based Dijkstra: at each of V steps, linearly scan
 * all unvisited nodes to find the minimum-distance one. For MAX_HOSPITALS
 * = 32 this is fast and, importantly, easy to read and explain in a
 * viva/demo -- a binary-heap-based O((V+E) log V) version (reusing the
 * heap.c module) would be a natural "future scope" optimization for a
 * larger simulated network.
 */
int dijkstra_shortest_path(HospitalGraph *g, int source_idx, int dest_idx,
                            int *out_path, int *out_path_len, double *out_total_cost) {
    if (!g || !out_path || !out_path_len || !out_total_cost) return 0;
    int n = g->num_hospitals;
    if (source_idx < 0 || source_idx >= n) return 0;
    if (dest_idx < 0 || dest_idx >= n) return 0;

    double dist[MAX_HOSPITALS];
    int visited[MAX_HOSPITALS];
    int prev[MAX_HOSPITALS];

    for (int i = 0; i < n; i++) {
        dist[i] = DBL_MAX;
        visited[i] = 0;
        prev[i] = -1;
    }
    dist[source_idx] = 0.0;

    for (int iter = 0; iter < n; iter++) {
        /* Pick the unvisited node with the smallest known distance. */
        int u = -1;
        double best = DBL_MAX;
        for (int i = 0; i < n; i++) {
            if (!visited[i] && dist[i] < best) {
                best = dist[i];
                u = i;
            }
        }
        if (u == -1) break; /* remaining nodes are unreachable */
        visited[u] = 1;

        if (u == dest_idx) break; /* shortest path to destination is finalized */

        for (int v = 0; v < n; v++) {
            if (visited[v]) continue;
            double weight = g->adj[u][v];
            if (weight == GRAPH_NO_EDGE) continue;
            double candidate = dist[u] + weight;
            if (candidate < dist[v]) {
                dist[v] = candidate;
                prev[v] = u;
            }
        }
    }

    if (dist[dest_idx] == DBL_MAX) {
        /* Destination unreachable from source in the simulated network. */
        return 0;
    }

    /* Reconstruct path by walking prev[] backwards from dest to source. */
    int path_rev[MAX_HOSPITALS];
    int len = 0;
    int cur = dest_idx;
    while (cur != -1) {
        path_rev[len++] = cur;
        if (cur == source_idx) break;
        cur = prev[cur];
    }

    /* Reverse into out_path so it reads source -> ... -> destination. */
    for (int i = 0; i < len; i++) {
        out_path[i] = path_rev[len - 1 - i];
    }
    *out_path_len = len;
    *out_total_cost = dist[dest_idx];
    return 1;
}

/*
 * Complexity: O(V^2) time, O(V) extra space, where V = number of hospitals
 * in the simulated network (V <= MAX_HOSPITALS = 32 here).
 * With a binary min-heap (as in heap.c) this could be reduced to
 * O((V + E) log V) for larger graphs.
 */
