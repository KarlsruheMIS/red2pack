//
// Created by Jannick Borowitz on 19.10.23.
//

#ifndef RED2PACK_IREDUCE_ALGORITHM_H
#define RED2PACK_IREDUCE_ALGORITHM_H

#include <vector>

#include "red2pack/data_structure/m2s_graph_access.h"
#include "red2pack/red2pack_def.h"

namespace red2pack {

/**
 * @brief Abstract class for a (weight) 2-packing set reduce algorithm
 */
class Ireduce_algorithm {
       public:
        virtual ~Ireduce_algorithm() = default;

        /**
         * @brief Reconstruct a solution for the input graph. If the reduced graph is non-empty, it is assumed that
         * solution_vec sets a solution for the reduced graph.
         * @param solution_vec is expected to have size N with N is at least as large as the number of vertices in the
         * input graph. If the reduced graph is non-empty, it is assumed that vertex v of the reduced graph is in a
         * solution iff. solution_vec[v]=true
         */
        virtual void build_solution(std::vector<bool>& solution_vec) = 0;

        /**
         * @return Returns the current solution weight
         */
        [[nodiscard]] virtual NodeWeight get_solution_weight() const = 0;

        /**
         * @brief Obtain the exact kernel. If a heuristic reduction style is used, undo all reductions and peeling steps
         * up to and including the first peeling step.
         * @param reduced_graph the exact reduced graph with consecutive vertex identifiers
         * @param reduced_node_id maps vertices to the reduced graph
         * @param former_node_id reverse mapping
         */
        virtual void get_exact_kernel(m2s_graph_access& reduced_graph, std::vector<NodeID>& reduced_node_id,
                                      std::vector<NodeID>& former_node_id) = 0;

        /**
         * @brief Run reduce algorithm.
         */
        virtual void run_reductions() = 0;
};
}  // namespace red2pack

#endif  // RED2PACK_IREDUCE_ALGORITHM_H
