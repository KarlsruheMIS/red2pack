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
#include "data_structures/m2s_dynamic_graph.h"
#include "data_structures/m2s_graph_access.h"
#include "definitions.h"
#include "fast_set.h"
#include "graph_access.h"
#include "m2s_config.h"
#include "data_structures/sized_vector.h"
#include "timer.h"

namespace two_packing_set {

class evo_graph;

class reduce_algorithm {
       public:
        enum pack_status { not_set, included, excluded, folded, unsafe };

       private:
        friend general_reduction_2pack;
        friend deg_one_2reduction_e;
        friend deg_two_2reduction_e;
        friend twin2_reduction_e;
        friend fast_domination2_reduction_e;
        friend neighborhood2_reduction_e;
        friend domination2_reduction_e;
        friend clique2_reduction_e;

        friend evo_graph;

        struct node_pos {
                NodeID node;
                size_t pos;

                // Konstruktor...
                node_pos(NodeID node = 0, size_t pos = 0) : node(node), pos(pos) {}
        };

        struct graph_status {
                friend evo_graph;

                size_t n = 0;
                size_t remaining_nodes = 0;
                NodeWeight pack_weight = 0;
                NodeWeight reduction_offset = 0;
                m2s_dynamic_graph graph;
                std::vector<NodeWeight> weights;
                std::vector<pack_status> node_status;
                std::vector<reduction2_ptr> reductions2;
                std::vector<m2ps_reduction_type> folded_queue;
                std::vector<NodeID> modified_queue;

                graph_status(m2s_graph_access& G, bool two_neighborhood_initialized)
                    : n(G.number_of_nodes()),
                      remaining_nodes(n),
                      graph(G, two_neighborhood_initialized),
                      weights(n, 0),
                      node_status(n, pack_status::not_set),
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

        // graph_status global_status;
        std::vector<size_t> reduction_map;

        M2SConfig config;
        timer t;
        // Lets see whether we need all of them or not...
        fast_set set_1;
        fast_set set_2;
        fast_set double_set;
        sized_vector<sized_vector<NodeID>> buffers;
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        //void set_imprecise(NodeID node, pack_status mpack_status);
        void set(NodeID node, pack_status mpack_status);
        size_t deg(NodeID node) const;
        size_t two_deg(NodeID node);

        bool is_adj(NodeID first, NodeID second);
        bool is_two_adj(NodeID first, NodeID second);

        bool reduce_deg_leq_one(NodeID node);


        void init_reduction_step();
        void reduce_graph_internal();

       public:
        reduce_algorithm(m2s_graph_access& G, const M2SConfig &m2s_config);
        void get_solution(std::vector<bool>& solution_vec);
        void run_reductions();
        /* 	std::vector<NodeID> get_status();  */
        graph_status global_status;
};

}  // namespace two_packing_set

#endif  // INC_2_PACKING_SET_REDUCE_ALGORITHM_H
