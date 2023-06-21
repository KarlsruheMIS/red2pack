//
// Created by Jannick Borowitz on 21.06.23.
//

#include "branch_and_reduce.h"

#include <chrono>
#include <utility>

#include "algorithms/kernel/reduce_algorithm.h"
#include "branch_and_reduce_algorithm.h"
#include "tools/m2s_log.h"

namespace two_packing_set {
branch_and_reduce::branch_and_reduce(two_packing_set::m2s_graph_access& G, M2SConfig m2s_cfg, MISConfig mis_cfg)
    : m2s_cfg(std::move(m2s_cfg)),
      mis_cfg(std::move(mis_cfg)),
      solution_status(G.number_of_nodes(), false),
      reduced_node_id(G.number_of_nodes(), G.number_of_nodes()),
      former_node_id(G.number_of_nodes()) {
        G.copy(this->graph);
}

bool branch_and_reduce::run() {
        auto start_t = std::chrono::system_clock::now();
        // kernel obtained with 2-packing set reductions
        auto kernel = perform_initial_reductions();

        auto stop_preprocessing = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_time_preprocessing = stop_preprocessing - start_t;
        mis_cfg.time_limit -= elapsed_time_preprocessing.count();


        if (kernel.number_of_nodes() == 0) {
                return true;
        }

        if (mis_cfg.time_limit > 0) {
                // find maximum independent set for kernel
                cout_handler::disable_cout();
                branch_and_reduce_algorithm mis_reducer(kernel, mis_cfg);
                mis_reducer.run_branch_reduce();
                cout_handler::enable_cout();
                mis_reducer.apply_branch_reduce_solution(kernel);

                // apply results to solution status
                for(std::size_t node = 0; node < graph.number_of_nodes(); node++) {
                        auto red_node = reduced_node_id[node];
                        if (red_node == -1) continue; // TODO
                        if (kernel.getPartitionIndex(red_node) == 1) {
                                solution_status[node] = true;
                                solution_size++;
                        }
                }

                return true;
        }

        return false;
}

graph_access branch_and_reduce::perform_initial_reductions() {
        auto start_t = std::chrono::system_clock::now();
        // transformed graph
        graph.construct_2neighborhood();
        // run first reductions
        reduce_algorithm reducer(graph, m2s_cfg);
        reducer.run_reductions();
        auto stop_t = std::chrono::system_clock::now();
        std::chrono::duration<double> time = stop_t - start_t;
        double reduction_time = time.count();
        // TODO print reduction time

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
        graph_access kernel;
        if (kernel_nodes == 0) {
                kernel.start_construction(0,0);
                kernel.finish_construction();
        }else {
                auto start_construction = std::chrono::system_clock::now();
                kernel.start_construction(kernel_nodes, std::min(kernel_nodes*kernel_nodes, graph.number_of_edges()));

                for (size_t i = 0; i < status.graph.size(); i++) {
                        if (status.node_status[i] == reduce_algorithm::pack_status::not_set) {
                                kernel.new_node();
                                for (size_t j = 0; j < status.graph[i].size(); j++) {
                                        if (status.node_status[status.graph[i][j]] == reduce_algorithm::pack_status::not_set) {
                                                kernel.new_edge(reduced_node_id[i], reduced_node_id[status.graph[i][j]]);
                                        }
                                }
                                for (size_t j = 0; j < status.graph.get2neighbor_list(i).size(); j++) {
                                        if (status.node_status[status.graph.get2neighbor_list(i)[j]] ==
                                            reduce_algorithm::pack_status::not_set) {
                                                kernel.new_edge(reduced_node_id[i], reduced_node_id[status.graph.get2neighbor_list(i)[j]]);
                                        }
                                }

                        }
                }
                kernel.finish_construction();

                auto stop_construction = std::chrono::system_clock::now();
                std::chrono::duration<double> elapsed_time_construction = stop_construction - start_construction;
                m2s_log::instance()->print_condensed_graph(kernel.number_of_nodes(), kernel.number_of_edges()/2, elapsed_time_construction.count());
        }

        return kernel;
}

}  // namespace two_packing_set