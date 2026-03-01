#include "red2pack/algorithms/rnt_solver_scheme.h"
#include "red2pack/tools/scoped_timer.h"

namespace red2pack {

bool rnt_solver_scheme::solve(timer t) {
        RED2PACK_SCOPED_TIMER("RnT Solver");
        if (graph == nullptr) {
                std::cerr << "No graph is attached to the 2-packing-set solver";
                exit(1);
        }

        // run reductions and build two-neighborhood
        auto [solved, reduced_graph] = run_reduce_and_transform();

        // finished or timelimit almost reached
        if (!solved && t.elapsed() < m2s_cfg.time_limit - eps_time) {
                // adjust timelimit adjusted by eps to have enough time to set a solution later on
                double mis_solve_time_limit = m2s_cfg.time_limit - eps_time - t.elapsed();

                // find maximum independent set for condensed graph and set solution
                solved = solve_mis(mis_solve_time_limit, reduced_graph);
        }

        // set global solution
        if(reducer != nullptr) {
                // reads solution status for nodes in reduced graph and builds global solution
                reducer->build_solution(solution_status);
                solution_offset_weight = reducer->get_solution_weight();
                m2s_log::instance()->set_best_size(get_solution_size());
        }

        return solved && t.elapsed() <= m2s_cfg.time_limit;
}

}  // namespace red2pack