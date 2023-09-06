#ifndef INC_2_PACKING_SET_REDUCE_ALGORITHM_H
#define INC_2_PACKING_SET_REDUCE_ALGORITHM_H

#include <array>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "algorithms/kernel/reductions.h"
#include "data_structure/m2s_dynamic_graph.h"
#include "data_structure/m2s_graph_access.h"
#include "definitions.h"
#include "fast_set.h"
#include "graph_access.h"
#include "m2s_config.h"
#include "sized_vector.h"
#include "timer.h"

namespace red2pack {

class reduce_algorithm {
       public:
        enum two_pack_status { not_set, included, excluded };

       protected:
        friend general_reduction_2pack;
        friend deg_one_2reduction;
        friend deg_two_2reduction;
        friend twin2_reduction;
        friend fast_domination2_reduction;
        friend domination2_reduction;
        friend clique2_reduction;

        // data structure that keeps track of all modifications by reductions to a given graph
        struct graph_status {

                // original number of nodes
                size_t n = 0;
                // remaining nodes
                size_t remaining_nodes = 0;
                // current 2-packing-set weight (size if unweighted)
                NodeWeight sol_weight = 0;
                // dynamic graph
                m2s_dynamic_graph graph;
                // weights of vertices
                std::vector<NodeWeight> weights;
                // solution status
                std::vector<two_pack_status> node_status;
                // 2-packing-set reductions that are applied in the given order
                std::vector<reduction2_ptr> reductions;
                // folds of nodes
                std::vector<m2ps_reduction_type> folded_queue;
                // modified vertices
                std::vector<NodeID> modified_queue;

                /**
                 * Initialize new graph status
                 * @param G
                 * @param two_neighborhood_initialized true if the two neighborhood was constructed for G
                 */
                graph_status(m2s_graph_access& G, bool two_neighborhood_initialized)
                    : n(G.number_of_nodes()),
                      remaining_nodes(n),
                      graph(G, two_neighborhood_initialized),
                      weights(n, 0),
                      node_status(n, two_pack_status::not_set),
                      folded_queue(n) {
                        forall_nodes (G, node) {
                                weights[node] = G.getNodeWeight(node);
                        }
                        endfor
                }

                graph_status() = default;
        };

        /////////////////////////////////////////////////////DATA/////////////////////////////////////////////////////////////////
        size_t active_reduction_index;

        std::vector<size_t> reduction_map;

        M2SConfig config;
        timer t;
        fast_set set_1;
        sized_vector<sized_vector<NodeID>> buffers;
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        /**
         * Sets 2-pack status for node.
         * @param node
         * @param new_two_pack_status
         */
        void set(NodeID node, two_pack_status new_two_pack_status);

        // get node degrees of current reduced graph
        [[nodiscard]] size_t deg(NodeID node) const;
        size_t two_deg(NodeID node);

        // adj queries for current reduced graph
        bool is_adj(NodeID first, NodeID second);
        bool is_two_adj(NodeID first, NodeID second);

        // helper function to reduce node if degree 0 or 1 reduction applies
        bool reduce_deg_leq_one(NodeID node);

        // helpers for reduction setup
        void init_reduction_step();
        void reduce_graph_internal();

       public:
        reduce_algorithm(m2s_graph_access& G, const M2SConfig &m2s_config);
        void get_solution(std::vector<bool>& solution_vec);
        void run_reductions();

        graph_status global_status;
};

}  // namespace red2pack

#endif  // INC_2_PACKING_SET_REDUCE_ALGORITHM_H
