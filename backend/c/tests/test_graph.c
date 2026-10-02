#include "graph.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>

int main(void) {
    HospitalGraph *g = graph_create();
    assert(g != NULL);

    /* Simulated network of 5 hospitals: A-B-C-D connected, E isolated. */
    int A = graph_add_hospital(g, "City General");
    int B = graph_add_hospital(g, "St. Mary's");
    int C = graph_add_hospital(g, "Metro ICU Center");
    int D = graph_add_hospital(g, "Riverside Hospital");
    int E = graph_add_hospital(g, "Northgate Clinic"); /* deliberately unreachable */

    assert(A == 0 && B == 1 && C == 2 && D == 3 && E == 4);

    assert(graph_add_edge(g, A, B, 5.0) == 1);
    assert(graph_add_edge(g, B, C, 3.0) == 1);
    assert(graph_add_edge(g, A, C, 10.0) == 1); /* longer direct route, should lose to A->B->C */
    assert(graph_add_edge(g, C, D, 2.0) == 1);

    /* Invalid edge attempts must fail cleanly, not crash. */
    assert(graph_add_edge(g, A, 99, 1.0) == 0);  /* bad index */
    assert(graph_add_edge(g, A, B, -1.0) == 0);  /* non-positive weight */

    int path[MAX_HOSPITALS];
    int path_len;
    double total_cost;

    /* A -> D shortest path should be A -> B -> C -> D, cost 5+3+2 = 10,
     * not the "obvious" A -> C -> D (10+2=12). */
    assert(dijkstra_shortest_path(g, A, D, path, &path_len, &total_cost) == 1);
    printf("A->D path: ");
    for (int i = 0; i < path_len; i++) printf("%s%s", g->names[path[i]], (i < path_len - 1) ? " -> " : "\n");
    printf("A->D total cost: %.1f\n", total_cost);
    assert(path_len == 4);
    assert(path[0] == A && path[1] == B && path[2] == C && path[3] == D);
    assert(fabs(total_cost - 10.0) < 1e-9);

    /* Same-node path: source == destination. */
    assert(dijkstra_shortest_path(g, A, A, path, &path_len, &total_cost) == 1);
    assert(path_len == 1 && path[0] == A);
    assert(fabs(total_cost - 0.0) < 1e-9);

    /* Edge case: destination is unreachable (E has no edges at all). */
    int reachable = dijkstra_shortest_path(g, A, E, path, &path_len, &total_cost);
    printf("A->E reachable: %s\n", reachable ? "yes" : "no (unreachable, as expected)");
    assert(reachable == 0);

    graph_destroy(g);
    graph_destroy(NULL);

    printf("test_graph: ALL TESTS PASSED\n");
    return 0;
}
