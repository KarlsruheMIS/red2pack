//
// Created by Jannick Borowitz on 19.08.23.
//

#include "solver_scheme.h"

#include <utility>

namespace red2pack {

bool solver_scheme::solve() {
        auto start_t = std::chrono::system_clock::now();
        // kernel obtained with 2-packing set reductions
        graph_access condensed_graph;

        // run reductions and build two-neighborhood
        run_reduce_and_transform(condensed_graph);

        auto stop_preprocessing = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_time_preprocessing = stop_preprocessing - start_t;
        mis_solve_time_limit -= elapsed_time_preprocessing.count();

        if (condensed_graph.number_of_nodes() == 0) {
                return true;
        }

        if (mis_solve_time_limit > 0) {
                // find maximum independent set for condensed graph
                // cout_handler::disable_cout();
                // cout_handler::enable_cout();
                auto solved = solve_mis(condensed_graph);
                apply_solution(condensed_graph);

                return solved;
        }

        return false;
}

void solver_scheme::apply_solution(graph_access& condensed_graph) {
        for (NodeID node = 0; node < condensed_graph.number_of_nodes(); node++) {
                if (condensed_graph.getPartitionIndex(node) == 1 && !solution_status[former_node_id[node]]) {
                        solution_status[former_node_id[node]] = true;
                        mis_solution_size++;
                }else if (condensed_graph.getPartitionIndex(node) == 0 && solution_status[former_node_id[node]]){
                        solution_status[former_node_id[node]] = false;
                        mis_solution_size--;
                }
        }
}

}  // namespace two_packing_set