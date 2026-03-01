//
// Created by Jannick Borowitz on 12.01.24.
//

#include "htwis_utils.h"
#include "red2pack-htwis-hils/algorithms/weighted_rnt_htwis.h"

#include <htwis/Graph.h>
#include <red2pack/algorithms/kernel/weighted_reduce_algorithm.h>


namespace red2pack {


void weighted_rnt_htwis::init_reducer() {
        reducer = std::move(std::make_unique<weighted_reduce_algorithm>(*graph, m2s_cfg));
}
bool weighted_rnt_htwis::use_reducer() { return m2s_cfg.use_weighted_reductions(); }
bool weighted_rnt_htwis::solve_mis(double mis_solve_time_limit, m2s_graph_access& reduced_graph) {
        RED2PACK_SCOPED_TIMER("Solve MWIS");
        using htwis::Graph;

        // init HtWIS graph and convert graph
        Graph g;
        convert(reduced_graph, g);
        auto stop_t = std::chrono::system_clock::now();

        perform_htwis(g);
        mis_solution_size += g.total_weight;
        m2s_log::instance()->set_best_size(get_solution_size());

        // set solution
        for (NodeID i = 0; i < static_cast<NodeID>(g.n); ++i) {
                if (!g.head[i].del) {
                        solution_status[former_node_id[i]] = true;
                } else {
                        solution_status[former_node_id[i]] = false;
                }
        }

        // we observed that the HtWIS solution is not always maximal; maximize solution
        // maximize solution
        std::vector<NodeID> free_nodes;
        free_nodes.reserve(reduced_graph.number_of_nodes());
        for (NodeID v = 0; v < reduced_graph.number_of_nodes(); ++v) {
                if (!solution_status[former_node_id[v]]) {
                        // check for maximality
                        bool free_node=true;
                        forall_out_edges (reduced_graph, e, v) {
                                auto target = reduced_graph.getEdgeTarget(e);
                                if(solution_status[former_node_id[target]]) {
                                        free_node = false;
                                        break;
                                }
                        } endfor
                        if(free_node) {
                                forall_out_links (reduced_graph, e, v) {
                                        auto target = reduced_graph.getLinkTarget(e);
                                        if(solution_status[former_node_id[target]]) {
                                                free_node = false;
                                                break;
                                        }
                                } endfor
                        }
                        if(free_node) {

                                free_nodes.push_back(v);
                        }
                }
        }
        std::sort(free_nodes.begin(), free_nodes.end(), [&g=reduced_graph](NodeID first, NodeID second) {return g.getNodeWeight(first) > g.getNodeWeight(second);});
        for(NodeID node : free_nodes) {
                bool free_node=true;
                forall_out_edges (reduced_graph, e, node) {
                        auto target = reduced_graph.getEdgeTarget(e);
                        if(solution_status[former_node_id[target]]) {
                                free_node = false;
                                break;
                        }
                } endfor
                if(free_node) {
                        forall_out_links (reduced_graph, e, node) {
                                auto target = reduced_graph.getLinkTarget(e);
                                if(solution_status[former_node_id[target]]) {
                                        free_node = false;
                                        break;
                                }
                        } endfor
                }

                if(free_node) {
                        solution_status[former_node_id[node]] = true;
                        mis_solution_size += reduced_graph.getNodeWeight(node);
                }
        }
        m2s_log::instance()->set_best_size(get_solution_size());

        return false;
}

}  // namespace red2pack
