#include "branch_and_reduce.h"

#include "branch_and_reduce_algorithm.h"

bool red2pack::branch_and_reduce::solve_mis(graph_access& condensed_graph) {

        mis_cfg.time_limit = get_mis_solve_time();
        branch_and_reduce_algorithm solver(condensed_graph, mis_cfg);

        auto solved = solver.run_branch_reduce();
        solver.apply_branch_reduce_solution(condensed_graph);

        return solved;
}
