#include <red2pack-onlinemis/algorithms/heuristic.h>

#include <red2pack/algorithms/convert_graph.h>
#include <red2pack/tools/m2s_log.h>

#include <onlinemis/ils/online_ils.h>

bool red2pack::heuristic::solve_mis(double mis_solve_time_limit, m2s_graph_access& reduced_graph) {
        using namespace onlinemis;

        timer total_solve_mis;
        total_solve_mis.restart();

        auto start_t = std::chrono::system_clock::now();

        graph_access transformed_graph;
        convert_sorted(reduced_graph, transformed_graph);

        auto stop_t = std::chrono::system_clock::now();
        std::chrono::duration<double> duration = stop_t - start_t;
        double convert_time = duration.count();

        m2s_log::instance()->print_transformed_graph(transformed_graph.number_of_nodes(), transformed_graph.number_of_edges()/2, convert_time);



        online_ils solver;
        mis_cfg.time_limit = mis_solve_time_limit - total_solve_mis.elapsed();
        std::cout << "time (s) until IS solver applied: " << m2s_log::instance()->get_timer() << std::endl;
        solver.perform_ils(mis_cfg, transformed_graph, mis_cfg.ils_iterations);

        for (NodeID node = 0; node < transformed_graph.number_of_nodes(); node++) {
                if (transformed_graph.getPartitionIndex(node) == 1 && !solution_status[former_node_id[node]]) {
                        solution_status[former_node_id[node]] = true;
                        mis_solution_size+=transformed_graph.getNodeWeight(node);
                } else if (transformed_graph.getPartitionIndex(node) == 0 && solution_status[former_node_id[node]]) {
                        solution_status[former_node_id[node]] = false;
                        mis_solution_size-=transformed_graph.getNodeWeight(node);;
                }
        }

        return false;
}

void red2pack::heuristic::attach(std::unique_ptr<m2s_graph_access> G, red2pack::M2SConfig m2s_cfg, onlinemis::MISConfig mis_cfg) {
        reduce_and_transform::attach(std::move(G), std::move(m2s_cfg));
        this->mis_cfg = std::move(mis_cfg);
}
