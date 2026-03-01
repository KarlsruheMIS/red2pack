//
// Created by Jannick Borowitz on 24.10.24.
//

#pragma once

#include <omp.h>
#include <red2pack/data_structure/m2s_graph_access.h>
#include <red2pack/data_structure/sized_vector.h>

#include <algorithm>
#include <limits>

namespace chils {

extern "C" {
#include <graph.h>
#include <local_search.h>
#include <chils.h>
}

struct chils_config {
        bool verbose = true, warm_start = false;
        int run_chils = 1, max_queue = 32;
        double timeout = 3600, step = 5;
        long long il = std::numeric_limits<long long>::max();
        int path_offset = 0, path_end = 0;
        unsigned seed = 0;
};

inline graph *build_graph(red2pack::m2s_graph_access &input_graph) {
        using namespace red2pack;

        RED2PACK_ASSERT_TRUE(input_graph.number_of_nodes() <= std::numeric_limits<int>::max());
        RED2PACK_ASSERT_TRUE(input_graph.number_of_edges() + input_graph.number_of_links() <=
                             std::numeric_limits<int>::max());

        int N = static_cast<int>(input_graph.number_of_nodes());
        long long M = static_cast<int>(input_graph.number_of_edges() + input_graph.number_of_links());

        int *V = (int *)malloc(sizeof(int) * (N + 1));
        int *E = (int *)malloc(sizeof(int) * M);

        long long *W = (long long *)malloc(sizeof(long long) * N);

        int ei = 0;
        forall_nodes (input_graph, u) {
                RED2PACK_ASSERT_TRUE(input_graph.getNodeWeight(u) <= std::numeric_limits<long long>::max());
                *(W + u) = static_cast<long long>(input_graph.getNodeWeight(u));

                V[u] = ei;
                forall_out_edges (input_graph, e, u) {
                        E[ei++] = input_graph.getEdgeTarget(e);
                }
                endfor
                forall_out_links(input_graph, e, u) { E[ei++] = input_graph.getLinkTarget(e); }
                endfor

                std::sort(E + V[u], E + ei);
        }
        endfor
        V[N] = ei;

        graph *g = (graph *)malloc(sizeof(graph));
        *g = (graph){.n = N, .V = V, .E = E, .W = W};

        return g;
}

inline graph *build_induced_subgraph(const red2pack::m2s_graph_access &input_graph, const std::vector<bool> &mask,
                                     const red2pack::sized_vector<red2pack::NodeID> &reverse_mapping,
                                     const red2pack::sized_vector<red2pack::NodeID> &mapping) {
        using namespace red2pack;

        RED2PACK_ASSERT_TRUE(input_graph.number_of_nodes() <= std::numeric_limits<int>::max());
        RED2PACK_ASSERT_TRUE(input_graph.number_of_edges() + input_graph.number_of_links() <=
                             std::numeric_limits<int>::max());

        int N = static_cast<int>(reverse_mapping.size());

        long long M = 0;
        for (NodeID u : reverse_mapping) {
                forall_out_edges (input_graph, e, u) {
                        if (mask[input_graph.getEdgeTarget(e)]) {
                                M++;
                        }
                }
                endfor
                forall_out_links(input_graph, e, u) {
                        if (mask[input_graph.getLinkTarget(e)]) {
                                M++;
                        }
                }
                endfor
        }

        int *V = (int *)malloc(sizeof(int) * (N + 1));
        int *E = (int *)malloc(sizeof(int) * M);

        long long *W = (long long *)malloc(sizeof(long long) * N);

        int ei = 0;
        for (NodeID i = 0; i < reverse_mapping.size(); ++i) {
                NodeID u = reverse_mapping[i];
                RED2PACK_ASSERT_TRUE(input_graph.getNodeWeight(u) <= std::numeric_limits<long long>::max());
                *(W + i) = static_cast<long long>(input_graph.getNodeWeight(u));

                V[i] = ei;
                forall_out_edges (input_graph, e, u) {
                        if (mask[input_graph.getEdgeTarget(e)]) {
                                E[ei++] = mapping[input_graph.getEdgeTarget(e)];
                        }
                }
                endfor
                forall_out_links(input_graph, e, u) {
                        if (mask[input_graph.getLinkTarget(e)]) {
                                E[ei++] = mapping[input_graph.getLinkTarget(e)];
                        }
                }
                endfor

                std::sort(E + V[i], E + ei);
        }
        V[N] = ei;

        graph *g = (graph *)malloc(sizeof(graph));
        *g = (graph){.n = N, .V = V, .E = E, .W = W};

        return g;
}

inline long long mwis_validate(graph *g, int *independent_set) {
        long long cost = 0;
        for (int u = 0; u < g->n; u++) {
                if (!independent_set[u]) continue;

                cost += g->W[u];
                for (int i = g->V[u]; i < g->V[u + 1]; i++) {
                        int v = g->E[i];
                        if (independent_set[v]) return -1;
                }
        }
        return cost;
}
inline void run_chils_impl(graph *g, std::vector<bool> &solution_indicator, double &time_best,
                           const chils_config &cfg) {
        // run CHILS

        int *reverse_mapping = NULL;
        int *original_solution = NULL;
        graph *original_graph = NULL;
        long long offset = 0;

        int *initial_solution = NULL;
        if (cfg.warm_start) {
                initial_solution = (int *)malloc(sizeof(int) * g->n);
                for (int i = 0; i < g->n; ++i) {
                        initial_solution[i] = (int)solution_indicator[i];
                }
                if (mwis_validate(g, initial_solution) < 0) {
                        fprintf(stderr, "Error: Warm-Start solution is not independent set.\n");
                        exit(1);
                }
        }

        int *solution = (int *)malloc(sizeof(int) * g->n);

        if (cfg.run_chils > 1) {
                omp_set_num_threads(1);

                chils *p = chils_init(g, cfg.run_chils, cfg.seed);
                p->step_time = cfg.step;
                p->step_count = cfg.il;
                if (initial_solution != NULL) chils_set_solution(g, p, initial_solution);

                for (int i = 0; i < cfg.run_chils; i++) p->LS[i]->max_queue = cfg.max_queue + (4 * i);

                chils_run(g, p, cfg.timeout, cfg.verbose, offset);

                time_best = p->time;

                int *best = chils_get_best_independent_set(p);
                for (int i = 0; i < g->n; i++) solution[i] = best[i];

                chils_free(p);
        } else {
                local_search *ls = local_search_init(g, cfg.seed);

                if (initial_solution != NULL)
                        for (int u = 0; u < g->n; u++)
                                if (initial_solution[u]) local_search_add_vertex(g, ls, u);

                ls->max_queue = cfg.max_queue;
                local_search_explore(g, ls, cfg.timeout, cfg.il, cfg.verbose);

                time_best = ls->time;

                for (int i = 0; i < g->n; i++) solution[i] = ls->independent_set[i];

                local_search_free(ls);
        }

        if (original_solution != NULL) {
                for (int u = 0; u < g->n; u++)
                        if (solution[u]) original_solution[reverse_mapping[u]] = 1;
        }


        if (original_solution == NULL) {
                for (int i = 0; i < g->n; i++)
                        if (solution[i])
                                solution_indicator[i] = true;
                        else
                                solution_indicator[i] = false;
        } else {
                for (int i = 0; i < original_graph->n; i++)
                        if (original_solution[i])
                                solution_indicator[i] = true;
                        else
                                solution_indicator[i] = false;
        }

        free(solution);
        free(original_solution);
        free(reverse_mapping);
        free(initial_solution);
        graph_free(original_graph);
}
inline void run_chils(red2pack::m2s_graph_access &input_graph, std::vector<bool> &solution, double &time_best,
                      const chils_config &cfg) {
        graph *g = build_graph(input_graph);
        run_chils_impl(g, solution, time_best, cfg);
        graph_free(g);
}

}  // namespace chils
