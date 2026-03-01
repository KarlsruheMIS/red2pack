//
// Created by Jannick Borowitz on 12.08.24.
//

#ifndef RED2PACK_HEURISTIC_REDUCTION_H
#define RED2PACK_HEURISTIC_REDUCTION_H

#include "red2pack/algorithms/kernel/weighted_reductions.h"
#include "red2pack/data_structure/advanced_max_node_heap.h"

namespace red2pack {

struct heuristic_w2pack : public general_reduction_w2pack {
        using NodeRating = std::pair<signed long int, NodeID>;

        explicit heuristic_w2pack(size_t n)
            : general_reduction_w2pack(n), top(n), top_value(n), top_set(n), rating_fnc(heuristic_rating_weight_diff) {}
        ~heuristic_w2pack() override = default;
        [[nodiscard]] reduction_w2pack_ptr clone() const final { return std::make_unique<heuristic_w2pack>(*this); }

        [[nodiscard]] mw2ps_reduction_type get_reduction_type() const final { return mw2ps_reduction_type::heuristic; }

        using RatingValue = signed long int;
        using RatingFunc = RatingValue (*)(m2s_advanced_dynamic_graph&, NodeID);

        // Heuristic rating strategies
        // Include the verties with the largest rating values
        // Exclude the vertices witht the smallest rating values
        static RatingValue heuristic_rating_weight_diff(m2s_advanced_dynamic_graph& graph, NodeID v) {
                NodeWeight sum_weight_two_neigh_v = 0;
                for (auto u : graph.neigh2(v)) {
                        sum_weight_two_neigh_v += graph.weight(u);
                }
                return static_cast<RatingValue>(graph.weight(v)) -
                       static_cast<RatingValue>(graph.get_sum_weight_of_neigh(v) + sum_weight_two_neigh_v);
        }
        static RatingValue heuristic_rating_weight(m2s_advanced_dynamic_graph& graph, NodeID v) {
                return static_cast<RatingValue>(graph.weight(v));
        }
        static RatingValue heuristic_rating_deg(m2s_advanced_dynamic_graph& graph, NodeID v) {
                return (-1) * static_cast<RatingValue>(graph.deg(v) + graph.deg2(v));
        }

       private:
        // data structured used by adaptive and non-apdative reduce-and-peel strategies to peel vertices
        sized_vector<NodeID> top;
        sized_vector<RatingValue> top_value;
        fast_set top_set;
        std::vector<NodeID> nodes;
        advanced_max_node_heap<RatingValue> candidates_q;
        // peeling heuristic
        RatingFunc rating_fnc;

       public:
        /**
         * @brief Reset local data structures so that the reduction can be used for a new status
         */
        void reset() final;

        /**
         * @brief Init heuristic reduction for a reduce algorithm (i.e. a specific config) before the graph is reduced.
         */
        void init(weighted_reduce_algorithm* algo) final;

        bool reduce(weighted_reduce_algorithm* algo) final;
        void restore(weighted_reduce_algorithm* algo, NodeID node) override;
        void apply(weighted_reduce_algorithm* algo, NodeID node) override;
};
}  // namespace red2pack

#endif  // RED2PACK_HEURISTIC_REDUCTION_H
