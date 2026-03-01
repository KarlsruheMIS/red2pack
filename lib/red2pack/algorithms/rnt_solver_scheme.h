#ifndef INC_2_PACKING_SET_RED2PACK_SOLVER_SCHEME_H
#define INC_2_PACKING_SET_RED2PACK_SOLVER_SCHEME_H

#include "red2pack/algorithms/kernel/reduce_and_transform.h"
#include "red2pack/data_structure/m2s_graph_access.h"
#include "red2pack/m2s_config.h"
#include "red2pack/tools/timer.h"

namespace red2pack {

/**
 * @class rnt_solver_scheme
 * @brief Abstract class for building maximum (weight) 2-packing-set solvers using our reduce-and-transform scheme.
 * To build your own 2-packing-set solver inherit this class and implement the missing methods.
 */
class rnt_solver_scheme : public reduce_and_transform {
       public:
        rnt_solver_scheme(std::unique_ptr<m2s_graph_access> G, M2SConfig m2s_cfg)
            : reduce_and_transform(std::move(G), std::move(m2s_cfg)),
              // 2 seconds
              eps_time(2.0) {}

        /**
         * Find a maximum two-packing set for the input graph.
         * @param t started timer for tracking the set time limit
         * @return true if solved without timeout or false otherwise
         */
        bool solve(timer t);

        /**
         * Get the size (weight in the weighted case) of the 2-packing set
         * @return
         */
        [[nodiscard]] NodeID get_solution_size() const { return get_solution_offset_size() + get_mis_solution_size(); }

        /**
         * Returns the solution size (or weight) of the M(W)IS problem for the reduced graph
         * @return Size (or Weight) of the (heuristic) M(W)IS found with @ref solve_mis.
         */
        [[nodiscard]] NodeID get_mis_solution_size() const { return mis_solution_size; }

       protected:
        /**
         * Use your favourite MIS solver to find a MIS for the reduced graph
         * Finally, update mis_solution_size and solution_status.
         *
         * @param mis_solve_time_limit time limit in seconds for finding a solution for the @p reduced_graph
         * @param reduced_graph reference to reduced graph
         * @return true is the problem was solved optimally.
         */
        virtual bool solve_mis(double mis_solve_time_limit, m2s_graph_access& reduced_graph) = 0;

        /// solution size (or weight in the weighted case) found with @ref solve_mis.
        NodeID mis_solution_size = 0;

        /// seconds used to update final global solution at the end
        double eps_time;
};

}  // namespace red2pack

#endif  // INC_2_PACKING_SET_RED2PACK_SOLVER_SCHEME_H
