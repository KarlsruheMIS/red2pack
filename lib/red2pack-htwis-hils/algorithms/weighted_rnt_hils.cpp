//
// Created by Jannick Borowitz on 12.01.24.
//

#include "hils_utils.h"

#include "red2pack-htwis-hils/algorithms/weighted_rnt_hils.h"

#include <red2pack/algorithms/convert_graph.h>
#include <red2pack/algorithms/kernel/weighted_reduce_algorithm.h>



namespace red2pack {

bool weighted_rnt_hils::solve_mis(double mis_solve_time_limit, m2s_graph_access& reduced_graph) {
        RED2PACK_SCOPED_TIMER("Solve MWIS");
        // HILS algorithm from extern/hils/src/main.cpp
        // modified to fit our interface
        using opt::Graph;
        using opt::Solution;

        timer total_solve_mis;
        total_solve_mis.restart();

        // independent set reductions?
        if (mis_cfg.perform_reductions) {
                bool solved_optimally = false;

                graph_access transformed_graph;
                convert(reduced_graph, transformed_graph);

                NodeWeight weight_offset = 0;
                graph_access rG;
                std::unique_ptr<branch_and_reduce_algorithm> wmis_reducer;

                mis_cfg.time_limit = mis_solve_time_limit - total_solve_mis.elapsed();
                weight_offset = perform_reduction(wmis_reducer, transformed_graph, rG, mis_cfg);

                std::cout << "wmis_reduction_nodes " << rG.number_of_nodes() << "\n";
                std::cout << "wmis_reduction_offset " << weight_offset << std::endl;

                if (rG.number_of_nodes() != 0 && total_solve_mis.elapsed() < mis_solve_time_limit) {
                        // prepare graph
                        opt::generator.seed(hils_cfg.rand_seed);
                        // opt::Graph counts undirected edges
                        Graph graph_instance(rG.number_of_nodes(), rG.number_of_edges());
                        convert(rG, graph_instance);
                        graph_instance.sort();

                        // solve weighted MIS with HILS

                        // we replaced the bossa_timer.h with timer because we had troubles to compile it
                        mis_solve_time_limit -= total_solve_mis.elapsed();

                        // apply HILS
                        double time_until_hils = m2s_log::instance()->get_timer();
                        double best_time_hils = 0;
                        auto best_solution = perform_hils(graph_instance, hils_cfg, best_time_hils, mis_solve_time_limit);
                        auto sol = best_solution.solution();
                        mis_solution_size = best_solution.weight() + weight_offset;

                        m2s_log::instance()->set_best_size(get_solution_size(),
                                                           time_until_hils + best_time_hils);

                        for (NodeID v = 0; v < rG.number_of_nodes(); v++) {
                                rG.setPartitionIndex(v, sol[v]);
                        }



                } else {
                        mis_solution_size = weight_offset;
                        m2s_log::instance()->set_best_size(get_solution_size());
                        std::cout << "MIS_weight " << weight_offset << std::endl;
                        solved_optimally = true;
                }

                wmis_reducer->reverse_reduction(transformed_graph, rG, reverse_mapping);

                //ASSERT_TRUE(is_IS(transformed_graph));

                for (NodeID node = 0; node < transformed_graph.number_of_nodes(); node++) {
                        if (transformed_graph.getPartitionIndex(node) == 1 && !solution_status[former_node_id[node]]) {
                                solution_status[former_node_id[node]] = true;
                        } else if (transformed_graph.getPartitionIndex(node) == 0 &&
                                   solution_status[former_node_id[node]]) {
                                solution_status[former_node_id[node]] = false;
                        }
                }

                if(!solved_optimally) {
                        // maximality check
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
                }

                return solved_optimally;
        } else {
                // prepare graph
                opt::generator.seed(hils_cfg.rand_seed);
                // opt::Graph counts undirected edges
                Graph graph_instance(reduced_graph.number_of_nodes(),
                                     (reduced_graph.number_of_edges() + reduced_graph.number_of_links()) / 2);
                convert(reduced_graph, graph_instance);
                graph_instance.sort();

                mis_solve_time_limit -= total_solve_mis.elapsed();

                // apply HILS
                double time_until_hils = m2s_log::instance()->get_timer();
                double best_time_hils = 0;
                auto best_solution = perform_hils(graph_instance, hils_cfg, best_time_hils, mis_solve_time_limit);
                auto sol = best_solution.solution();
                mis_solution_size = best_solution.weight();
                for (NodeID v = 0; v < reduced_graph.number_of_nodes(); v++) {
                        solution_status[former_node_id[v]] = sol[v];
                }

                m2s_log::instance()->set_best_size(get_solution_size(),
                                                   time_until_hils + best_time_hils);

                return false;
        }
}

void weighted_rnt_hils::init_reducer() {
        reducer = std::make_unique<weighted_reduce_algorithm>(*graph, m2s_cfg);
}
bool weighted_rnt_hils::use_reducer() { return m2s_cfg.use_weighted_reductions(); }

}  // namespace red2pack
