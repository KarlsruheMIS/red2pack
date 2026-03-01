//
// Created by Jannick Borowitz on 16.10.23.
//

#ifndef RED2PACK_WEIGHTED_REDUCE_ALGORITHM_H
#define RED2PACK_WEIGHTED_REDUCE_ALGORITHM_H

#include "red2pack/algorithms/kernel/Ireduce_algorithm.h"
#include "red2pack/algorithms/kernel/weighted_reductions.h"
#include "red2pack/algorithms/kernel/heuristic_reduction.h"
#include "red2pack/data_structure/m2s_advanced_dynamic_graph.h"
#include "red2pack/m2s_config.h"
#include "red2pack/red2pack_def.h"
#include "red2pack/tools/timer.h"

namespace red2pack {

class weighted_reduce_algorithm : public Ireduce_algorithm {
       public:
        struct graph_status {
                // original number of nodes
                size_t n = 0;
                // remaining nodes
                size_t remaining_nodes = 0;
                // current 2-packing-set weight (size if unweighted)
                NodeWeight sol_weight = 0;
                // weight offset to input graph
                NodeWeight reduction_offset = 0;
                // heuristic steps
                bool peeled = false;
                NodeID first_peeled_node = 0;
                // dynamic graph
                m2s_advanced_dynamic_graph graph;
                // solution status
                std::vector<two_pack_status> node_status;
                // 2-packing-set reductions that are applied in the given order
                std::vector<reduction_w2pack_ptr> reductions;
                std::size_t fast_reductions_offset = 0;
                // folds of nodes
                sized_vector<mw2ps_reduction_type> folded_queue;
                // modified vertices
                sized_vector<NodeID> modified_queue;

                /**
                 * Initialize new graph status
                 * @param G
                 * @param two_neighborhood_initialized true if the two neighborhood was constructed for G
                 */
                graph_status(m2s_graph_access& G, bool two_neighborhood_initialized, bool maintain_two_neighborhood)
                    : n(G.number_of_nodes()),
                      remaining_nodes(n),
                      graph(G, two_neighborhood_initialized, true, maintain_two_neighborhood),
                      node_status(n, two_pack_status::not_set),
                      folded_queue(n),
                      modified_queue(n)
                {
                }

                graph_status(const graph_status& other)
                    : n(other.n),
                      remaining_nodes(other.remaining_nodes),
                      sol_weight(other.sol_weight),
                      reduction_offset(other.reduction_offset),
                      peeled(other.peeled),
                      first_peeled_node(other.first_peeled_node),
                      graph(other.graph),
                      node_status(other.node_status),
                      fast_reductions_offset(other.fast_reductions_offset),
                      folded_queue(other.folded_queue),
                      modified_queue(other.modified_queue)
                {
                        reductions.reserve(other.reductions.size());
                        for (auto& reduction : other.reductions) {
                                reductions.push_back(reduction->clone());
                        }
                }

                graph_status& operator=(const graph_status& other) {
                        if (&other == this) {
                                return *this;
                        }
                        n = other.n;
                        remaining_nodes = other.remaining_nodes;
                        sol_weight = other.sol_weight;
                        reduction_offset = other.reduction_offset;
                        peeled = other.peeled;
                        first_peeled_node = other.first_peeled_node;
                        graph = other.graph;
                        node_status = other.node_status;
                        fast_reductions_offset = other.fast_reductions_offset;
                        folded_queue = other.folded_queue;
                        modified_queue = other.modified_queue;
                        reductions.clear();
                        reductions.reserve(other.reductions.size());
                        for (auto& reduction : other.reductions) {
                                reductions.push_back(reduction->clone());
                        }

                        return *this;
                }

                graph_status() = default;


                NodeWeight get_weight_two_neigh(NodeID v) {
                        NodeWeight weight_two_neigh = 0;
                        for (auto& neighbor : graph.neigh2(v)) {
                                weight_two_neigh += graph.weight(neighbor);
                        }
                        return weight_two_neigh;
                }
        };

       protected:

        // friend weighted reductions
        friend fast_neighborhood_removal_w2pack;
        friend fast_complete_degree_one_removal_w2pack;
        friend fast_track_complete_degree_one_removal_w2pack;
        friend fast_degree_two_removal_w2pack;
        friend neighborhood_folding_w2pack;
        friend fold2_w2pack;
        friend neighborhood_removal_w2pack;
        friend split_neighbor_removal_w2pack;
        friend direct_neighbor_removal_w2pack;
        friend two_neighbor_removal_w2pack;
        friend single_fast_domination_w2pack;
        friend split_intersection_removal_w2pack;
        friend domination_w2pack;
        friend fast_domination_w2pack;
        friend weight_transfer_w2pack;
        friend fast_direct_neighbor_removal_w2pack;
        friend heuristic_w2pack;

        /////////////////////////////////////////////////////DATA/////////////////////////////////////////////////////////////////
        size_t active_reduction_index;

        std::vector<size_t> reduction_map;


        timer t;
        fast_set set_1;
        fast_set set_2;
        fast_set set_3;
        sized_vector<NodeID> weights_neigh;
        sized_vector<sized_vector<NodeID>> buffers;
        sized_vector<signed long int> signed_buffer;
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        /**
         * Sets 2-pack status for node.
         * @param node
         * @param new_two_pack_status
         */
        void set(NodeID node, two_pack_status new_two_pack_status, bool push_modified = true,
                 bool hide_two_edges = true, bool mark_neigh = true);
        void set_included_fast(NodeID node);
        void set_two_simplicial_included(NodeID included_node, sized_vector<NodeID>& two_clique,
                                         fast_set& two_clique_set);
        void bulk_exclude(sized_vector<NodeID>& two_clique, fast_set& two_clique_set);

        void unset(NodeID node, bool restore = true);

        // helpers for reduction setup
        void init_reduction_step();

        void reduce_graph_internal();

        void add_next_level_node(NodeID node);
        void add_next_level_node_no_fast(NodeID node);
        void add_next_level_nodes(const std::vector<NodeID>& nodes);
        void add_next_level_nodes(const sized_vector<NodeID>& nodes);
        void add_next_level_neighborhood(NodeID node);
        void add_next_level_two_neighborhood(NodeID node);
        void add_next_level_neighborhood(const std::vector<NodeID>& nodes);

        void remove_next_level_node(NodeID node, mw2ps_reduction_type reduction_type);

        bool time_limit_reached() { return config.time_limit < t.elapsed(); }

        void add_remaining_nodes_greedily();

        // heuristic mode
        void restore_exact_kernel();
        void apply_heuristic_solution(m2s_graph_access& exact_reduced_graph, const std::vector<NodeID>& former_node_id);

       public:
        weighted_reduce_algorithm(m2s_graph_access& G, M2SConfig m2s_config);

        // set solution for kernel, restore graph (set solution for reduced nodes)
        void build_solution(std::vector<bool>& solution_vec) override;

        [[nodiscard]] NodeWeight get_solution_weight() const override;

        void get_exact_kernel(m2s_graph_access& reduced_graph, std::vector<NodeID>& reduced_node_id,
                              std::vector<NodeID>& former_node_id) override;

        void run_reductions() override;
        void restore();
        void reset();

        graph_status global_status;
        M2SConfig config;
};

}  // namespace red2pack

#endif  // RED2PACK_WEIGHTED_REDUCE_ALGORITHM_H
