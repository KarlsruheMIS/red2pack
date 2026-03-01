//
// Created by Jannick Borowitz on 12.01.24.
//

#include "red2pack-mmwis/algorithms/weighted_rnt_mmwis.h"

#include <red2pack/algorithms/kernel/weighted_reduce_algorithm.h>
#include <red2pack/algorithms/convert_graph.h>
#include <red2pack/tools/scoped_timer.h>

#include <branch_and_reduce_algorithm.h>
#include <mmwis_config.h>
#include <mmwis_log.h>
#include <reduction_evolution.h>

namespace red2pack {



void weighted_rnt_mmwis::attach(std::unique_ptr<m2s_graph_access> G, M2SConfig m2s_cfg, ::mmwis::MISConfig mmwis_cfg) {
        reduce_and_transform::attach(std::move(G), std::move(m2s_cfg));
        this->mmwis_config = std::move(mmwis_cfg);
}

void weighted_rnt_mmwis::init_reducer() {
        reducer = std::make_unique<weighted_reduce_algorithm>(*graph, m2s_cfg);
}
bool weighted_rnt_mmwis::use_reducer() { return m2s_cfg.use_weighted_reductions(); }

bool weighted_rnt_mmwis::solve_mis(double mis_solve_time_limit, m2s_graph_access& reduced_graph) {
        RED2PACK_SCOPED_TIMER("Solve MWIS");

        timer total_solve_mis;
        total_solve_mis.restart();

        graph_access transformed_graph;
        convert_sorted(reduced_graph, transformed_graph);

        NodeWeight weight_offset = 0;


        mmwis_config.time_limit = mis_solve_time_limit - total_solve_mis.elapsed();
        mmwis_config.evo_time_limit = 0.1*mmwis_config.time_limit;

        ::mmwis::reduction_evolution<::mmwis::branch_and_reduce_algorithm> solver;
        std::vector<bool> independent_set(transformed_graph.number_of_nodes(), false);
        std::vector<::NodeID> best_nodes;
        std::vector<::NodeID> worse_nodes;
        bool solved_optimally = false;

        double time_until_mmwis = m2s_log::instance()->get_timer();
        std::cout << "time (s) until wmis solver applied: " << time_until_mmwis << std::endl;
        solver.perform_mis_search(mmwis_config, transformed_graph, independent_set, best_nodes, worse_nodes, solved_optimally, false);
        m2s_log::instance()->set_best_size(get_solution_size()+ mmwis::mmwis_log::instance()->get_best_weight(),
                                           time_until_mmwis + mmwis::mmwis_log::instance()->get_best_time());
        ASSERT_TRUE(is_IS(transformed_graph));

        for (NodeID node = 0; node < transformed_graph.number_of_nodes(); node++) {
                if (independent_set[node] && !solution_status[former_node_id[node]]) {
                        solution_status[former_node_id[node]] = true;
                        mis_solution_size+=transformed_graph.getNodeWeight(node);
                } else if (!independent_set[node] && solution_status[former_node_id[node]]) {
                        solution_status[former_node_id[node]] = false;
                        mis_solution_size-=transformed_graph.getNodeWeight(node);;
                }
        }

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
                ASSERT_TRUE(!solution_status[former_node_id[node]]);
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


        return solved_optimally;
}

}  // namespace red2pack
