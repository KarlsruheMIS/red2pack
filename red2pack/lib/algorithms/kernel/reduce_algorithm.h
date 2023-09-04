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
        enum two_pack_status { not_set, included, excluded, folded, unsafe };

       private:
        friend general_reduction_2pack;
        friend deg_one_2reduction;
        friend deg_two_2reduction;
        friend twin2_reduction;
        friend fast_domination2_reduction;
        friend domination2_reduction;
        friend clique2_reduction;

        struct graph_status {
                size_t n = 0;
                size_t remaining_nodes = 0;
                NodeWeight pack_weight = 0;
                NodeWeight reduction_offset = 0;
                m2s_dynamic_graph graph;
                std::vector<NodeWeight> weights;
                std::vector<two_pack_status> node_status;
                std::vector<reduction2_ptr> reductions2;
                std::vector<m2ps_reduction_type> folded_queue;
                std::vector<NodeID> modified_queue;

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

        struct node_pos {
                NodeID node;
                size_t pos;

                // Konstruktor...
                node_pos(NodeID node = 0, size_t pos = 0) : node(node), pos(pos) {}
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
        size_t deg(NodeID node) const;
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

}  // namespace two_packing_set

#endif  // INC_2_PACKING_SET_REDUCE_ALGORITHM_H
