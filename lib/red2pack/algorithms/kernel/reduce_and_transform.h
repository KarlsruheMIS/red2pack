#ifndef INC_2_PACKING_SET_BRANCH_AND_REDUCE_H
#define INC_2_PACKING_SET_BRANCH_AND_REDUCE_H

#include <memory>
#include <utility>

#include "red2pack/algorithms/kernel/Ireduce_algorithm.h"
#include "red2pack/data_structure/m2s_graph_access.h"
#include "red2pack/m2s_config.h"
#include "red2pack/tools/m2s_graph_io.h"

namespace red2pack {

/**
 * Reducer and graph transformer for the (weighted) 2-packing-set.
 * Takes a graph, applies 2-packing-set reductions and builds a condensed graph for the MIS problem
 */
class reduce_and_transform {
       public:
        virtual ~reduce_and_transform() = default;
        /**
         * Constructor for 2-packing-set reduce-and-transform algorithm
         * @param graph
         * @param m2s_cfg
         */
        reduce_and_transform(std::unique_ptr<m2s_graph_access> G, M2SConfig m2s_cfg)
            : m2s_cfg(std::move(m2s_cfg)),
              solution_status(G->number_of_nodes(), false),
              reduced_node_id(G->number_of_nodes(), G->number_of_nodes()),
              former_node_id(G->number_of_nodes()) {
                graph = std::move(G);
        }

        /**
         * @brief Applies 2-packing-set reductions and builds the condensed graph for the MIS problem. The
         * condensed_graph is empty after the call if the reductions solve the graph optimally. If all reductions
         * are disabled, the 2-neighborhood is constructed (the input graph is changed), and a reference to the input
         * graph is returned.
         * @return pair of a boolean and a reference to the reduced graph; true if solved, false if a non-empty
         * condensed graph for the MIS problem is left
         */
        std::pair<bool, m2s_graph_access&> run_reduce_and_transform();

        /**
         * @return the solution offset size achieved by the 2-packing-set reductions
         */
        [[nodiscard]] NodeWeight get_solution_offset_size() const { return solution_offset_weight; }

        /**
         * @return const ref to a solution vector holding true values for the respective solution vertices.
         */
        const std::vector<bool>& get_solution() { return solution_status; }

        /**
         * @brief Attach another graph and config to find a 2-packing-set for it. Note that if a graph was attached
         * before, it can not be obtained back using detach afterwards.
         * @param G
         * @param m2s_cfg
         * @return
         */
        void attach(std::unique_ptr<m2s_graph_access> G, M2SConfig m2s_cfg);

        /**
         * Detach graph from solver
         * @return
         */
        std::unique_ptr<m2s_graph_access>& detach();

       protected:
        std::unique_ptr<m2s_graph_access> graph;
        std::unique_ptr<Ireduce_algorithm> reducer;
        M2SConfig m2s_cfg;

        // indicate solution vertices of the 2-packing-set for the input graph
        std::vector<bool> solution_status;
        // solution weight
        NodeWeight solution_offset_weight = 0;

        // mapping graph node ids to condensed graph node ids
        std::vector<NodeID> reduced_node_id;
        // mapping condensed graph node ids to graph node ids
        std::vector<NodeID> former_node_id;
        // reduce and compute new graph access
        m2s_graph_access reduced_graph;

        virtual void init_reducer();

        virtual bool use_reducer();

       private:
        /**
         * Transform the input graph to a condensed graph using red2pack reductions.
         */
        void transform_with_reductions();

        /**
         * Transform the input graph to a condensed graph without using red2pack reduction.
         * Note that the condensed graph ist the square graph of the input graph.
         */
        void transform_without_reductions();
};
}  // namespace red2pack

#endif  // INC_2_PACKING_SET_BRANCH_AND_REDUCE_H
