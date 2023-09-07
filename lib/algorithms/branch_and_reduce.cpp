#include "branch_and_reduce.h"

#include <utility>

#include "branch_and_reduce_algorithm.h"

bool red2pack::branch_and_reduce::solve_mis(graph_access& condensed_graph) {

        mis_cfg.time_limit = get_mis_solve_time();
        branch_and_reduce_algorithm solver(condensed_graph, mis_cfg);

        auto solved = solver.run_branch_reduce();
        solver.apply_branch_reduce_solution(condensed_graph);

        return solved;
}
void red2pack::branch_and_reduce::attach(std::unique_ptr<m2s_graph_access> G, red2pack::M2SConfig m2s_cfg, MISConfig mis_cfg) {
        reduce_and_transform::attach(std::move(G), std::move(m2s_cfg));
        this->mis_cfg = std::move(mis_cfg);
}
