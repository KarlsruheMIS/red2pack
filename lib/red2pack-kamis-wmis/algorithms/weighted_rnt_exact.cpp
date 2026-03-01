#include "red2pack-kamis-wmis/algorithms/weighted_rnt_exact.h"

#include <red2pack/algorithms/kernel/weighted_reduce_algorithm.h>
#include <red2pack/algorithms/convert_graph.h>
#include <red2pack/tools/scoped_timer.h>

#include <branch_and_reduce_algorithm.h>
#include <mis_log.h>
#include <graph_access.h>



namespace red2pack {

void weighted_rnt_exact::attach(std::unique_ptr<m2s_graph_access> G, red2pack::M2SConfig m2s_cfg, MISConfig mis_cfg) {
        reduce_and_transform::attach(std::move(G), std::move(m2s_cfg));
        this->mis_cfg = std::move(mis_cfg);
}

void weighted_rnt_exact::init_reducer() {
        reducer = std::move(std::make_unique<weighted_reduce_algorithm>(*graph, m2s_cfg));
}
bool weighted_rnt_exact::use_reducer() { return m2s_cfg.use_weighted_reductions(); }

bool weighted_rnt_exact::solve_mis(double mis_solve_time_limit, m2s_graph_access& reduced_graph) {
        RED2PACK_SCOPED_TIMER("Solve MWIS");
        timer total_solve_mis;
        total_solve_mis.restart();

        // measure convert
        auto start_t = std::chrono::system_clock::now();

        graph_access transformed_graph;
        convert(reduced_graph, transformed_graph);

        auto stop_t = std::chrono::system_clock::now();
        std::chrono::duration<double> duration = stop_t - start_t;
        double convert_time = duration.count();

        m2s_log::instance()->print_transformed_graph(transformed_graph.number_of_nodes(), transformed_graph.number_of_edges()/2, convert_time);

        mis_cfg.time_limit = mis_solve_time_limit - total_solve_mis.elapsed();
        branch_and_reduce_algorithm solver(transformed_graph, mis_cfg);
        // print elapased time since start of MW2PS algorithm to compute time found later on
        // TODO: can we capture time of best solution of KaMIS/wmis bnr instead of parsing log afterward?
        // std::cout << "time (s) until IS solver applied: " << m2s_log::instance()->get_timer() << std::endl;
	      //double new_best_time = m2s_log::instance()->get_timer();
        double time_until_bnr = m2s_log::instance()->get_timer();
        auto solved = solver.run_branch_reduce(true); // only use IS reductions if 2PS reduction were not used
        m2s_log::instance()->set_best_size(get_solution_size()+solver.get_is_weight(),
                                           time_until_bnr + solver.get_best_time());
        solver.apply_branch_reduce_solution(transformed_graph);

#ifdef NDEBUG
        for (NodeID node = 0; node < transformed_graph.number_of_nodes(); node++) {
                if(transformed_graph.getPartitionIndex(node) == 0) {
                        bool found = false;
                        forall_out_edges (transformed_graph, e, node) {
                               auto target = transformed_graph.getEdgeTarget(e);

                               if(transformed_graph.getPartitionIndex(target) == 1) {
                                       found = true;
                                       break;
                               }

                        }endfor
                        if (!found) {
                                std::cout << "solution for transformed graph not maximal" << std::endl;
                        }
                }
        }
#endif

        for (NodeID node = 0; node < transformed_graph.number_of_nodes(); node++) {
                if (transformed_graph.getPartitionIndex(node) == 1 && !solution_status[former_node_id[node]]) {
                        solution_status[former_node_id[node]] = true;
                        mis_solution_size+=transformed_graph.getNodeWeight(node);
                } else if (transformed_graph.getPartitionIndex(node) == 0 && solution_status[former_node_id[node]]) {
                        solution_status[former_node_id[node]] = false;
                        mis_solution_size-=transformed_graph.getNodeWeight(node);
                }
        }
        std::cout << "wmis sol: " << mis_solution_size << std::endl;

        return solved;
}

}  // namespace red2pack
