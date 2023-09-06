#ifndef INC_2_PACKING_SET_BRANCH_AND_REDUCE_H
#define INC_2_PACKING_SET_BRANCH_AND_REDUCE_H

#include "data_structure/m2s_dynamic_graph.h"
#include "data_structure/m2s_graph_access.h"
#include "graph_access.h"
#include "m2s_config.h"
#include "reduce_algorithm.h"

namespace red2pack {

/**
 * Reducer and graph transformer for 2-packing-set.
 * Takes a graph, applies 2-packing-set reductions and builds a condensed graph for the MIS problem
 */
class reduce_and_transform {
       public:
        /**
         * Constructor for 2-packing-set reduce-and-transform algorithm
         * @param graph
         * @param m2s_cfg
         */
        reduce_and_transform(std::unique_ptr<m2s_graph_access> G, M2SConfig m2s_cfg);

        /**
         * Applies 2-packing-set reductions and build the condensed graph for the MIS problem.
         * The condensed_graph is empty after the call if the reductions solve the graph to optimality.
         * @param condensed_graph (ref to not-yet constructed condensed graph)
         * @return true if solved, false if a non-empty condensed graph for the MIS problem left
         */
        bool run_reduce_and_transform(graph_access& condensed_graph);

        /**
         * @return the solution offset size achieved by the 2-packing-set reductions
         */
        [[nodiscard]] NodeWeight get_solution_offset_size() const { return solution_offset_size; }

        /**
         * @return const ref to a solution vector holding true values for the respective solution vertices.
         */
        const std::vector<bool>& get_solution() { return solution_status; }

        /**
         * Attach another graph and config to find a 2-packing-set for it.
         * Note that if a graph was attached before, it can not be obtained back using detach afterwards.
         * @param G
         * @param m2s_cfg
         * @return
         */
        virtual void attach(std::unique_ptr<m2s_graph_access> G, M2SConfig m2s_cfg);

        /**
         * Detach graph from solver
         * @return
         */
        std::unique_ptr<m2s_graph_access>& detach();

       protected:
        std::unique_ptr<m2s_graph_access> graph;
        M2SConfig m2s_cfg;

        // indicate solution vertices of the 2-packing-set for the input graph
        std::vector<bool> solution_status;

        // mapping graph node ids to condensed graph node ids
        std::vector<NodeID> reduced_node_id;
        // mapping condensed graph node ids to graph node ids
        std::vector<NodeID> former_node_id;

        // HELPERS to build the condensed graph

        // you can overwrite them if your MIS solver requires special properties for the condensed graphs
        /**
         * Build a condensed graph given a m2s_graph_access instance (input graph).
         * @param condensed_graph
         * @param graph
         * @param nodes of condensed_graph
         * @param edges of condensed_graph
         */
        virtual void build_condensed_graph_from_access(graph_access& condensed_graph, m2s_graph_access& graph,
                                                       NodeID nodes, EdgeID edges);

        /**
         * Build a condensed graph given a m2s_dynamic_graph instance and a node status with respect to the found
         * 2-packing-set so far.
         * @param condensed_graph
         * @param reduced_graph
         * @param reduced_node_status
         * @param nodes
         * @param edges
         */
        virtual void build_condensed_graph_from_status(
            graph_access& condensed_graph, m2s_dynamic_graph& reduced_graph,
            std::vector<reduce_algorithm::two_pack_status>& reduced_node_status, NodeID nodes, EdgeID edges);

       private:
        NodeWeight solution_offset_size = 0;

        /**
         * Transform the input graph to a condensed graph using red2pack reductions.
         * @param condensed_graph
         */
        void transform_with_reductions(graph_access& condensed_graph);

        /**
         * Transform the input graph to a condensed graph without using red2pack reduction.
         * Note that the condensed graph ist the square graph of the input graph.
         * @param condensed_graph
         */
        void transform_without_reductions(graph_access& condensed_graph);
};
}  // namespace red2pack

#endif  // INC_2_PACKING_SET_BRANCH_AND_REDUCE_H
