#include "heuristic.h"

#include <chrono>
#include <utility>

#include "algorithms/kernel/reduce_algorithm.h"
#include "ils.h"
#include "local_search.h"
#include "online_ils.h"
#include "tools/m2s_log.h"

namespace two_packing_set {
heuristic::heuristic(two_packing_set::m2s_graph_access& G, M2SConfig m2s_cfg, MISConfig mis_cfg)
    : m2s_cfg(std::move(m2s_cfg)),
      mis_cfg(std::move(mis_cfg)),
      solution_status(G.number_of_nodes(), false),
      reduced_node_id(G.number_of_nodes(), G.number_of_nodes()),
      former_node_id(G.number_of_nodes()) {
        G.copy(this->graph);
}

bool heuristic::run() {
        auto start_t = std::chrono::system_clock::now();
        // kernel obtained with 2-packing set reductions
        graph_access condensed_graph;
        construct_condensed_graph(condensed_graph);

        auto stop_preprocessing = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_time_preprocessing = stop_preprocessing - start_t;
        mis_cfg.time_limit -= elapsed_time_preprocessing.count();


        if (condensed_graph.number_of_nodes() == 0) {
                return true;
        }

        if (mis_cfg.time_limit > 0) {
                // find maximum independent set for kernel
                online_ils solver;
                solver.perform_ils(mis_cfg, condensed_graph, mis_cfg.ils_iterations);

                // apply results to solution status
                for(std::size_t node = 0; node < condensed_graph.number_of_nodes(); node++) {
                        if (condensed_graph.getPartitionIndex(node) == 1) {
                                solution_status[former_node_id[node]] = true;
                                solution_size++;
                        }
                }
        }

        return false;
}

void heuristic::construct_condensed_graph(graph_access& condensed_graph) {
        if(m2s_cfg.disable_fast_domination && m2s_cfg.disable_domination && m2s_cfg.disable_deg_two && m2s_cfg.disable_deg_one && m2s_cfg.disable_twin && m2s_cfg.disable_clique) {
                auto start_t = std::chrono::system_clock::now();
                graph.construct_2neighborhood();
                auto stop_t = std::chrono::system_clock::now();
                std::chrono::duration<double> time = stop_t - start_t;
                double reduction_time = time.count();
                m2s_log::instance()->print_reduction(solution_size, graph.number_of_nodes(), 0,
                                                     reduction_time);

                for (size_t i = 0; i < graph.number_of_nodes(); i++) {
                        reduced_node_id[i] = i;
                        former_node_id[i] = i;
                }
                auto start_construction = std::chrono::system_clock::now();
                EdgeID count_edges = 0;
                for (size_t i = 0; i < graph.number_of_nodes(); i++) {
                        count_edges += graph.getNodeDegree(i) + graph.get2Degree(i);
                }
                condensed_graph.start_construction(graph.number_of_nodes(), count_edges);
                for (size_t i = 0; i < graph.number_of_nodes(); i++) {
                        condensed_graph.new_node();
                        condensed_graph.setNodeWeight(reduced_node_id[i], 1);
                        for (size_t j = graph.get_first_edge(i); j < graph.get_first_invalid_edge(i); j++) {
                                condensed_graph.new_edge(reduced_node_id[i], reduced_node_id[graph.getEdgeTarget(j)]);
                        }
                        for (size_t j = graph.get_first_edge2(i); j < graph.get_first_invalid_edge2(i); j++) {
                                condensed_graph.new_edge(reduced_node_id[i], reduced_node_id[graph.getEdgeTarget2(j)]);
                        }
                }
                condensed_graph.finish_construction();

                auto stop_construction = std::chrono::system_clock::now();
                std::chrono::duration<double> elapsed_time_construction = stop_construction - start_construction;
                m2s_log::instance()->print_condensed_graph(condensed_graph.number_of_nodes(), condensed_graph.number_of_edges()/2, elapsed_time_construction.count());

        }else {
                perform_initial_reductions(condensed_graph);
        }
}

void heuristic::perform_initial_reductions(graph_access& condensed_graph) {
        auto start_t = std::chrono::system_clock::now();
        // construct 2neighborhood
        if (!m2s_cfg.on_demand_two_neighborhood) {
                graph.construct_2neighborhood();
        }
        // run first reductions
        reduce_algorithm reducer(graph, m2s_cfg);
        reducer.run_reductions();
        auto stop_t = std::chrono::system_clock::now();
        std::chrono::duration<double> time = stop_t - start_t;
        double reduction_time = time.count();

        // set solution status
        reducer.get_solution(solution_status);
        solution_size = 0;
        for (auto && status_of_node : solution_status) {
                if (status_of_node) solution_size++;
        }

        // compute new graph access
        auto &status = reducer.global_status;
        auto n = status.graph.size();
        reduced_node_id.resize(n);

        NodeID kernel_nodes = 0;

        // Implemented optimization: we don't need to sort adjacency arrays to run KaMIS/wmis

        // compute node id mapping for graph and kernel
        for (size_t i = 0; i < status.graph.size(); i++) {
                if (status.node_status[i] == reduce_algorithm::pack_status::not_set) {
                        reduced_node_id[i] = kernel_nodes;
                        former_node_id[kernel_nodes] = i;
                        kernel_nodes++;
                }
        }

        m2s_log::instance()->print_reduction(solution_size, kernel_nodes, 0,
                                             reduction_time);

        // compute adjacency arrays
        if (kernel_nodes == 0) {
                condensed_graph.start_construction(0,0);
                condensed_graph.finish_construction();
        }else {
                auto start_construction = std::chrono::system_clock::now();
                EdgeID count_edges = 0;
                for (size_t i = 0; i < status.graph.size(); i++) {
                        if (status.node_status[i] == reduce_algorithm::pack_status::not_set) {
                                for (size_t j = 0; j < status.graph[i].size(); j++) {
                                        if (status.node_status[status.graph[i][j]] == reduce_algorithm::pack_status::not_set) {
                                                count_edges++;
                                        }
                                }
                                for (size_t j = 0; j < status.graph.get2neighbor_list(i).size(); j++) {
                                        if (status.node_status[status.graph.get2neighbor_list(i)[j]] ==
                                            reduce_algorithm::pack_status::not_set) {
                                                count_edges++;
                                        }
                                }

                        }
                }
                condensed_graph.start_construction(kernel_nodes, count_edges);
                for (size_t i = 0; i < status.graph.size(); i++) {
                        if (status.node_status[i] == reduce_algorithm::pack_status::not_set) {
                                condensed_graph.new_node();
                                condensed_graph.setNodeWeight(reduced_node_id[i], 1);
                                for (size_t j = 0; j < status.graph[i].size(); j++) {
                                        if (status.node_status[status.graph[i][j]] == reduce_algorithm::pack_status::not_set) {
                                                condensed_graph.new_edge(reduced_node_id[i], reduced_node_id[status.graph[i][j]]);
                                        }
                                }
                                for (size_t j = 0; j < status.graph.get2neighbor_list(i).size(); j++) {
                                        if (status.node_status[status.graph.get2neighbor_list(i)[j]] ==
                                            reduce_algorithm::pack_status::not_set) {
                                                condensed_graph.new_edge(reduced_node_id[i], reduced_node_id[status.graph.get2neighbor_list(i)[j]]);
                                        }
                                }

                        }
                }
                condensed_graph.finish_construction();

                auto stop_construction = std::chrono::system_clock::now();
                std::chrono::duration<double> elapsed_time_construction = stop_construction - start_construction;
                m2s_log::instance()->print_condensed_graph(condensed_graph.number_of_nodes(), condensed_graph.number_of_edges()/2, elapsed_time_construction.count());
        }
}

}  // namespace two_packing_set
