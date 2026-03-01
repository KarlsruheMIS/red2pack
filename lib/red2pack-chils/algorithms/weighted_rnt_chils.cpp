//
// Created by Jannick Borowitz on 23.10.24.
//

#include "red2pack-chils/algorithms/weighted_rnt_chils.h"
#include "red2pack-chils/algorithms/chils_utils.h"

#include <red2pack/algorithms/kernel/weighted_reduce_algorithm.h>
#include <red2pack/tools/scoped_timer.h>
#include <limits>


namespace red2pack {

void weighted_rnt_chils::init_reducer() { reducer = std::make_unique<weighted_reduce_algorithm>(*graph, m2s_cfg); }
bool weighted_rnt_chils::use_reducer() { return m2s_cfg.use_weighted_reductions(); }

bool weighted_rnt_chils::solve_mis(double mis_solve_time_limit, m2s_graph_access& reduced_graph) {
        RED2PACK_SCOPED_TIMER("Solve MWIS");

        timer total_solve_mis;
        total_solve_mis.restart();
        
        std::vector<bool> solution(reduced_graph.number_of_nodes(), false);
        double time_until_chils = m2s_log::instance()->get_timer();
        double time_best = 0;
        chils::run_chils(reduced_graph, solution, time_best,
                         chils::chils_config{
                             .verbose = true,
                             .warm_start = false,
                             .run_chils = 16,
                             .max_queue = 32,
                             .timeout = mis_solve_time_limit,
                             .step = 10,
                             .il = std::numeric_limits<long long>::max(),
                             .path_offset = 0,
                             .path_end = 0,
                             .seed = static_cast<unsigned>(m2s_cfg.seed)
                         });

        for (NodeID v = 0; v < reduced_graph.number_of_nodes(); ++v) {
                solution_status[former_node_id[v]] = solution[v];
                if (solution[v]) {
                        mis_solution_size += reduced_graph.getNodeWeight(v);
                }
        }
        m2s_log::instance()->set_best_size(get_solution_size(), time_until_chils + time_best);

        // maximize solution
        std::vector<NodeID> free_nodes;
        free_nodes.reserve(reduced_graph.number_of_nodes());
        for (NodeID v = 0; v < reduced_graph.number_of_nodes(); ++v) {
                if (!solution_status[former_node_id[v]]) {
                        // check for maximality
                        bool free_node = true;
                        forall_out_edges (reduced_graph, e, v) {
                                auto target = reduced_graph.getEdgeTarget(e);
                                if (solution_status[former_node_id[target]]) {
                                        free_node = false;
                                        break;
                                }
                        }
                        endfor
                        if (free_node) {
                                forall_out_links(reduced_graph, e, v) {
                                        auto target = reduced_graph.getLinkTarget(e);
                                        if (solution_status[former_node_id[target]]) {
                                                free_node = false;
                                                break;
                                        }
                                }
                                endfor
                        }
                        if (free_node) {
                                free_nodes.push_back(v);
                        }
                }
        }
        std::sort(free_nodes.begin(), free_nodes.end(), [&g = reduced_graph](NodeID first, NodeID second) {
                return g.getNodeWeight(first) > g.getNodeWeight(second);
        });
        for (NodeID node : free_nodes) {
                RED2PACK_ASSERT_TRUE(!solution_status[former_node_id[node]]);
                bool free_node = true;
                forall_out_edges (reduced_graph, e, node) {
                        auto target = reduced_graph.getEdgeTarget(e);
                        if (solution_status[former_node_id[target]]) {
                                free_node = false;
                                break;
                        }
                }
                endfor
                if (free_node) {
                        forall_out_links(reduced_graph, e, node) {
                                auto target = reduced_graph.getLinkTarget(e);
                                if (solution_status[former_node_id[target]]) {
                                        free_node = false;
                                        break;
                                }
                        }
                        endfor
                }

                if (free_node) {
                        solution_status[former_node_id[node]] = true;
                        mis_solution_size += reduced_graph.getNodeWeight(node);
                }
        }
        m2s_log::instance()->set_best_size(get_solution_size());

        return false;
}

}  // namespace red2pack
