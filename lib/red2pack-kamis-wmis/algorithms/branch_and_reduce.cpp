#include "red2pack-kamis-wmis/algorithms/branch_and_reduce.h"

#include <red2pack/algorithms/convert_graph.h>
#include <red2pack/tools/m2s_log.h>

#include <branch_and_reduce_algorithm.h>

#include <utility>

bool red2pack::branch_and_reduce::solve_mis(double mis_solve_time_limit, m2s_graph_access& reduced_graph) {
        timer total_solve_mis;
        total_solve_mis.restart();

        auto start_t = std::chrono::system_clock::now();

        graph_access transformed_graph;
        convert(reduced_graph, transformed_graph);

        auto stop_t = std::chrono::system_clock::now();
        std::chrono::duration<double> duration = stop_t - start_t;
        double convert_time = duration.count();

        m2s_log::instance()->print_transformed_graph(transformed_graph.number_of_nodes(), transformed_graph.number_of_edges()/2, convert_time);

        std::cout << "time (s) until IS solver applied: " << m2s_log::instance()->get_timer() << std::endl;
        mis_cfg.time_limit = mis_solve_time_limit - total_solve_mis.elapsed();
        branch_and_reduce_algorithm solver(transformed_graph, mis_cfg);
        auto solved = solver.run_branch_reduce();


        solver.apply_branch_reduce_solution(transformed_graph);

        for (NodeID node = 0; node < transformed_graph.number_of_nodes(); node++) {
                if (transformed_graph.getPartitionIndex(node) == 1 && !solution_status[former_node_id[node]]) {
                        solution_status[former_node_id[node]] = true;
                        mis_solution_size+=transformed_graph.getNodeWeight(node);
                } else if (transformed_graph.getPartitionIndex(node) == 0 && solution_status[former_node_id[node]]) {
                        solution_status[former_node_id[node]] = false;
                        mis_solution_size-=transformed_graph.getNodeWeight(node);;
                }
        }

        return solved;
}
void red2pack::branch_and_reduce::attach(std::unique_ptr<m2s_graph_access> G, red2pack::M2SConfig m2s_cfg, MISConfig mis_cfg) {
        reduce_and_transform::attach(std::move(G), std::move(m2s_cfg));
        this->mis_cfg = std::move(mis_cfg);
}
