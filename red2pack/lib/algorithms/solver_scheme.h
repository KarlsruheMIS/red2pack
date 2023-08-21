//
// Created by Jannick Borowitz on 19.08.23.
//

#ifndef INC_2_PACKING_SET_RED2PACK_SOLVER_SCHEME_H
#define INC_2_PACKING_SET_RED2PACK_SOLVER_SCHEME_H

#include "algorithms/kernel/reduce_and_transform.h"
#include "data_structure/m2s_graph_access.h"
#include "m2s_config.h"
#include "mis_config.h"
namespace red2pack {

class solver_scheme : public reduce_and_transform {
       public:
        solver_scheme(m2s_graph_access& G, M2SConfig m2s_cfg)
            : reduce_and_transform(G, std::move(m2s_cfg)), mis_solve_time_limit(m2s_cfg.time_limit) {}

        /**
         * Find a maximum two-packing set for the input graph.
         * @return true if solved without timeout or false otherwise
         */
        bool solve();

        /**
         * Use your favourite MIS solver to find a MIS for the condensed graph
         * @param condensed_graph
         * @return
         */
        virtual bool solve_mis(graph_access& condensed_graph) = 0;

        /**
         * Get the size of the two-packing-set
         * @return
         */
        NodeID get_solution_size() { return get_solution_offset_size() + get_mis_solution_size(); }

        /**
         * get the solution size of the MIS problem for the condensed graph
         * @return
         */
        [[nodiscard]] NodeID get_mis_solution_size() const {
                return mis_solution_size;
        }

       protected:
        /**
         * @return time-limit that must be assigned to the MIS solver
         */
        [[nodiscard]] double get_mis_solve_time() const { return mis_solve_time_limit; }

        /**
         * Updates the solution status of the related vertices of the input graphs
         * @param condensed_graph
         */
        void apply_solution(graph_access& condensed_graph);

       private:
        double mis_solve_time_limit = 0;
        NodeID mis_solution_size = 0;
};

}  // namespace red2pack

#endif  // INC_2_PACKING_SET_RED2PACK_SOLVER_SCHEME_H
