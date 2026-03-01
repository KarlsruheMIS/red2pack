//
// Created by Jannick Borowitz on 16.10.23.
//

#ifndef RED2PACK_WEIGHTED_REDUCTIONS_H
#define RED2PACK_WEIGHTED_REDUCTIONS_H

#include "red2pack/red2pack_def.h"
#include "red2pack/data_structure/m2s_advanced_dynamic_graph.h"
#include "red2pack/data_structure/fast_set.h"
#include "red2pack/data_structure/sized_vector.h"

#include <memory>

namespace red2pack {

enum class mw2ps_reduction_type {
        neighborhood_removal,
        split_neighbor_removal,
        direct_neighbor_removal,
        two_neighbor_removal,
        single_fast_domination,
        split_intersection_removal,
        domination,
        fast_domination,
        weight_transfer,
        old_weight_transfer,
        heuristic,
        fast_neighborhood_removal,
        neighborhood_folding,
        fold2,
        fast_direct_neighbor_removal,
        fast_complete_degree_one_removal,
        fast_degree_two_removal,
};
constexpr size_t MW2PS_REDUCTION_NUM = 20;

class weighted_reduce_algorithm;

class weight_transfer_w2pack;
class fast_neighborhood_removal_w2pack;
class vertex_marker_w2pack {
        friend weight_transfer_w2pack;
        friend fast_neighborhood_removal_w2pack;

       private:
        sized_vector<NodeID> current;
        sized_vector<NodeID> next;
        std::vector<size_t> next_map;  // look up next node position
        fast_set added_vertices;

       public:
        explicit vertex_marker_w2pack(size_t size)
            : current(size), next(size), next_map(size, 0), added_vertices(size){};

        void add(NodeID vertex) {
                if (!added_vertices.get(vertex)) {
                        next.push_back(vertex);
                        added_vertices.add(vertex);
                }
        }

        void remove(NodeID vertex) {
                if (added_vertices.get(vertex)) {
                        using std::swap;
                        auto next_pos = next_map[vertex];
                        next[next_pos] = next.back();
                        next.pop_back();
                        next_map[next[next_pos]] = next_pos;
                        added_vertices.remove(vertex);
                }
        }

        bool is_added(NodeID vertex) { return added_vertices.get(vertex); }

        void get_next() {
                //if (!next.empty()) {
                        current.swap(next);
                        clear_next();
                //}
        }

        void clear_next() {
                next.clear();
                added_vertices.clear();
        }

        void clear_current() {
                current.clear();
        }

        void fill_current_ascending(size_t n) {
                current.clear();
                for (size_t i = 0; i < n; i++) {
                        current.push_back(i);
                }
        }

	void shuffle();

        NodeID current_vertex(size_t index) { return current[index]; }

        size_t current_size() { return current.size(); }
        size_t next_size() { return next.size(); }
};

struct general_reduction_w2pack;
using reduction_w2pack_ptr = std::unique_ptr<general_reduction_w2pack>;
struct general_reduction_w2pack {
        explicit general_reduction_w2pack(size_t n) : marker(n) {}
        virtual ~general_reduction_w2pack() = default;
        [[nodiscard]] virtual reduction_w2pack_ptr clone() const = 0;

        [[nodiscard]] virtual mw2ps_reduction_type get_reduction_type() const = 0;
        virtual bool reduce(weighted_reduce_algorithm* algo) = 0;
        virtual void restore(weighted_reduce_algorithm* algo, NodeID node) {}
        virtual void apply(weighted_reduce_algorithm* algo, NodeID node) {}
        virtual void reset() {
                has_run = false;
                marker.clear_next();
                marker.get_next();
        }
        virtual void init(weighted_reduce_algorithm* algo) {}

        // process marked vertices in batches
        // between the batches, we check the time limit
        static constexpr NodeID batch_size = 10000;

        bool has_run = false;
        vertex_marker_w2pack marker;
};

struct neighborhood_removal_w2pack : public general_reduction_w2pack {
        explicit neighborhood_removal_w2pack(size_t n) : general_reduction_w2pack(n) {}
        ~neighborhood_removal_w2pack() override = default;
        [[nodiscard]] reduction_w2pack_ptr clone() const final {
                return std::make_unique<neighborhood_removal_w2pack>(*this);
        }

        [[nodiscard]] mw2ps_reduction_type get_reduction_type() const final {
                return mw2ps_reduction_type::neighborhood_removal;
        }
        bool reduce(weighted_reduce_algorithm* algo) final;
};

struct split_neighbor_removal_w2pack : public general_reduction_w2pack {
        explicit split_neighbor_removal_w2pack(size_t n) : general_reduction_w2pack(n) {}
        ~split_neighbor_removal_w2pack() override = default;
        [[nodiscard]] reduction_w2pack_ptr clone() const override {
                return std::make_unique<split_neighbor_removal_w2pack>(*this);
        }

        [[nodiscard]] mw2ps_reduction_type get_reduction_type() const override {
                return mw2ps_reduction_type::split_neighbor_removal;
        }

        bool reduce_direct_neighbor(weighted_reduce_algorithm* algo, NodeID v, NodeID u,
                                    NodeWeight sum_weight_two_neigh_v);
        bool reduce_two_neighbor(weighted_reduce_algorithm* algo, NodeID v, NodeID u,
                                 NodeWeight sum_weight_two_neigh_v);

        bool reduce(weighted_reduce_algorithm* algo) override;
};

/**
 * This class implements the split intersection removal reduction.
 * This reduction improves the bounds of the intersection removal reduction by differentiating 1- and 2-edges.
 */
struct split_intersection_removal_w2pack : public general_reduction_w2pack {
        explicit split_intersection_removal_w2pack(size_t n) : general_reduction_w2pack(n) {}
        ~split_intersection_removal_w2pack() override = default;
        [[nodiscard]] reduction_w2pack_ptr clone() const final {
                return std::make_unique<split_intersection_removal_w2pack>(*this);
        }

        [[nodiscard]] mw2ps_reduction_type get_reduction_type() const final {
                return mw2ps_reduction_type::split_intersection_removal;
        }

        std::pair<bool, NodeWeight> reduce_intersection_of_direct_neighbors(weighted_reduce_algorithm* algo, NodeID v,
                                                                            NodeID u);
        std::pair<bool, NodeWeight> reduce_intersection_of_two_neighbors(weighted_reduce_algorithm* algo, NodeID v,
                                                                         NodeID u);

        bool reduce(weighted_reduce_algorithm* algo) final;
};

/**
 * This class implements two cases of the domination reduction. It is a special case of the intersection reduction.
 */
struct domination_w2pack : public general_reduction_w2pack {
        explicit domination_w2pack(size_t n) : general_reduction_w2pack(n) {}
        ~domination_w2pack() override = default;
        [[nodiscard]] reduction_w2pack_ptr clone() const final { return std::make_unique<domination_w2pack>(*this); }

        [[nodiscard]] mw2ps_reduction_type get_reduction_type() const final { return mw2ps_reduction_type::domination; }

        bool reduce(weighted_reduce_algorithm* algo) final;
};

/**
 * This class implements two cases of the domination reduction.
 * If you use this reduction, you do not need to apply the domination reductions afterwards.
 * It only initializes 2-neighborhoods if the necessary subset relation, N[v]\subseteq N[u], holds.
 */
struct fast_domination_w2pack : public general_reduction_w2pack {
        explicit fast_domination_w2pack(size_t n) : general_reduction_w2pack(n) {}
        ~fast_domination_w2pack() override = default;
        [[nodiscard]] reduction_w2pack_ptr clone() const final {
                return std::make_unique<fast_domination_w2pack>(*this);
        }

        [[nodiscard]] mw2ps_reduction_type get_reduction_type() const final {
                return mw2ps_reduction_type::fast_domination;
        }

        bool reduce(weighted_reduce_algorithm* algo) final;
};

/**
 * This class implements the single fast domination reduction.
 * It is one of the three cases of fast domination.
 * Since it needs a copy of the direct neighborhood (in contrast to the other cases of fast domination),
 * it is implemented as an extra reduction.
 */
struct single_fast_domination_w2pack : public general_reduction_w2pack {
        explicit single_fast_domination_w2pack(size_t n) : general_reduction_w2pack(n) {}
        ~single_fast_domination_w2pack() override = default;
        [[nodiscard]] reduction_w2pack_ptr clone() const final {
                return std::make_unique<single_fast_domination_w2pack>(*this);
        }

        [[nodiscard]] mw2ps_reduction_type get_reduction_type() const final {
                return mw2ps_reduction_type::single_fast_domination;
        }
        bool reduce(weighted_reduce_algorithm* algo) final;
};

/**
 * This class implements the weight transfer reduction.
 * Besides reducing the weight of non-2-simplicial vertices of 2-cliques,
 * it applies the weighted 2-clique reduction.
 */
struct weight_transfer_w2pack : public general_reduction_w2pack {
        explicit weight_transfer_w2pack(size_t n) : general_reduction_w2pack(n) {}
        ~weight_transfer_w2pack() override = default;
        [[nodiscard]] reduction_w2pack_ptr clone() const { return std::make_unique<weight_transfer_w2pack>(*this); }

        [[nodiscard]] mw2ps_reduction_type get_reduction_type() const { return mw2ps_reduction_type::weight_transfer; }

        void apply(weighted_reduce_algorithm* algo, NodeID node) final;
        void restore(weighted_reduce_algorithm* algo, NodeID node) final;
        bool reduce(weighted_reduce_algorithm* algo);

       protected:
        struct weighted_node {
                NodeID node;
                NodeWeight weight;
        };

       private:
        struct restore_data {
                weighted_node simplicial;
                std::vector<NodeID> non_simplicial;

                restore_data() = default;
                restore_data(const weighted_node& simplicial, std::vector<NodeID>&& non_simplicial)
                    : simplicial(simplicial), non_simplicial(std::move(non_simplicial)){};
        };
        std::vector<restore_data> restore_vec;

       protected:
        void fold(weighted_reduce_algorithm* algo, const weight_transfer_w2pack::weighted_node& simplicial,
                  std::vector<NodeID>&& non_simplicial_nodes);
};

struct fast_neighborhood_removal_w2pack : public general_reduction_w2pack {
        explicit fast_neighborhood_removal_w2pack(size_t n) : general_reduction_w2pack(n) {}
        ~fast_neighborhood_removal_w2pack() override = default;
        [[nodiscard]] reduction_w2pack_ptr clone() const final {
                return std::make_unique<fast_neighborhood_removal_w2pack>(*this);
        }

        [[nodiscard]] mw2ps_reduction_type get_reduction_type() const final {
                return mw2ps_reduction_type::fast_neighborhood_removal;
        }
        bool reduce(weighted_reduce_algorithm* algo) final;
};

struct fast_degree_two_removal_w2pack : public general_reduction_w2pack {
        explicit fast_degree_two_removal_w2pack(size_t n) : general_reduction_w2pack(n) {}
        ~fast_degree_two_removal_w2pack() override = default;
        [[nodiscard]] reduction_w2pack_ptr clone() const final {
                return std::make_unique<fast_degree_two_removal_w2pack>(*this);
        }

        [[nodiscard]] mw2ps_reduction_type get_reduction_type() const final {
                return mw2ps_reduction_type::fast_degree_two_removal;
        }
        bool reduce(weighted_reduce_algorithm* algo) final;
};

struct fast_complete_degree_one_removal_w2pack : public general_reduction_w2pack {
        explicit fast_complete_degree_one_removal_w2pack(size_t n) : general_reduction_w2pack(n) {}
        ~fast_complete_degree_one_removal_w2pack() override = default;
        [[nodiscard]] reduction_w2pack_ptr clone() const  {
                return std::make_unique<fast_complete_degree_one_removal_w2pack>(*this);
        }

        [[nodiscard]] mw2ps_reduction_type get_reduction_type() const {
                return mw2ps_reduction_type::fast_complete_degree_one_removal;
        }
        void apply(weighted_reduce_algorithm* algo, NodeID node) final;
        void restore(weighted_reduce_algorithm* algo, NodeID node) final;
        bool reduce(weighted_reduce_algorithm* algo);

       protected:
        struct weighted_node {
                NodeID node;
                NodeWeight weight;
        };

       private:
        struct restore_data {
                weighted_node simplicial;
                std::vector<NodeID> non_simplicial;

                restore_data() = default;
                restore_data(const weighted_node& simplicial, std::vector<NodeID>&& non_simplicial)
                    : simplicial(simplicial), non_simplicial(std::move(non_simplicial)){};
        };

        std::vector<restore_data> restore_vec;

       protected:
        void fold(weighted_reduce_algorithm* algo,
                  const fast_complete_degree_one_removal_w2pack::weighted_node& simplicial,
                  std::vector<NodeID>&& weight_shifted);

};

struct fast_track_complete_degree_one_removal_w2pack : public fast_complete_degree_one_removal_w2pack {
        explicit fast_track_complete_degree_one_removal_w2pack(size_t n) : fast_complete_degree_one_removal_w2pack(n) {}
        ~fast_track_complete_degree_one_removal_w2pack() override = default;
        [[nodiscard]] reduction_w2pack_ptr clone() const final {
                return std::make_unique<fast_track_complete_degree_one_removal_w2pack>(*this);
        }

        [[nodiscard]] mw2ps_reduction_type get_reduction_type() const final {
                return mw2ps_reduction_type::fast_complete_degree_one_removal;
        }

        bool reduce(weighted_reduce_algorithm* algo) final;
};

/**
 * This class implements the neighborhood folding reduction.
 */
struct neighborhood_folding_w2pack : public general_reduction_w2pack {
        explicit neighborhood_folding_w2pack(size_t n) : general_reduction_w2pack(n) {}
        ~neighborhood_folding_w2pack() override = default;
        [[nodiscard]] reduction_w2pack_ptr clone() const final {
                return std::make_unique<neighborhood_folding_w2pack>(*this);
        }

        [[nodiscard]] mw2ps_reduction_type get_reduction_type() const final {
                return mw2ps_reduction_type::neighborhood_folding;
        }

        void apply(weighted_reduce_algorithm* algo, NodeID node) final;
        void restore(weighted_reduce_algorithm* algo, NodeID node) final;
        bool reduce(weighted_reduce_algorithm* algo) final;

       private:
        struct folded_nodes {
                NodeID main;
                std::vector<NodeID> MW2S;
        };

        struct restore_data {
                folded_nodes nodes;
                NodeWeight main_weight;
                NodeWeight MW2S_weight;
                m2s_advanced_dynamic_graph::neighbor_list main_two_neighbor_list;
                std::vector<std::vector<NodeID>> MW2S_one_neigh;
                std::vector<std::vector<NodeID>> MW2S_two_neigh;
        };

        void fold(weighted_reduce_algorithm* algo, NodeID main_node, fast_set& MW2S_set, NodeWeight MW2S_weight);

        std::vector<restore_data> restore_vec;
};

/**
 * This class implements the fold2 reduction.
 * It is a special case of the neighborhood folding reduction with for verices with a 2-degree of at most 2.
 */
struct fold2_w2pack : public general_reduction_w2pack {
        explicit fold2_w2pack(size_t n) : general_reduction_w2pack(n) {}
        ~fold2_w2pack() override = default;
        [[nodiscard]] reduction_w2pack_ptr clone() const final { return std::make_unique<fold2_w2pack>(*this); }

        [[nodiscard]] mw2ps_reduction_type get_reduction_type() const final { return mw2ps_reduction_type::fold2; }

        void apply(weighted_reduce_algorithm* algo, NodeID node) final;
        void restore(weighted_reduce_algorithm* algo, NodeID node) final;
        bool reduce(weighted_reduce_algorithm* algo) final;

       private:
        struct folded_nodes {
                NodeID main;
                std::array<NodeID, 2> neighbors;
        };

        struct restore_data {
                folded_nodes nodes;
                NodeWeight main_weight;
                m2s_advanced_dynamic_graph::neighbor_list main_two_neighbor_list;
                std::array<std::vector<NodeID>, 2> MW2S_one_neigh;
                std::array<std::vector<NodeID>, 2> MW2S_two_neigh;
        };

        void fold(weighted_reduce_algorithm* algo, const folded_nodes& nodes);

        std::vector<restore_data> restore_vec;
};

/**
 * This class implements the direct neighbor removal reduction.
 * It is the first case of the split neighbor removal reduction.
 */
struct direct_neighbor_removal_w2pack : public split_neighbor_removal_w2pack {
        explicit direct_neighbor_removal_w2pack(size_t n) : split_neighbor_removal_w2pack(n) {}
        ~direct_neighbor_removal_w2pack() override = default;
        [[nodiscard]] reduction_w2pack_ptr clone() const final {
                return std::make_unique<direct_neighbor_removal_w2pack>(*this);
        }

        [[nodiscard]] mw2ps_reduction_type get_reduction_type() const final {
                return mw2ps_reduction_type::direct_neighbor_removal;
        }
        bool reduce(weighted_reduce_algorithm* algo) final;
};

struct fast_direct_neighbor_removal_w2pack : public split_neighbor_removal_w2pack {
        explicit fast_direct_neighbor_removal_w2pack(size_t n) : split_neighbor_removal_w2pack(n) {}
        ~fast_direct_neighbor_removal_w2pack() override = default;
        [[nodiscard]] reduction_w2pack_ptr clone() const final {
                return std::make_unique<fast_direct_neighbor_removal_w2pack>(*this);
        }

        [[nodiscard]] mw2ps_reduction_type get_reduction_type() const final {
                return mw2ps_reduction_type::fast_direct_neighbor_removal;
        }
        bool reduce(weighted_reduce_algorithm* algo) final;
        bool fast_reduce_direct_neighbor(weighted_reduce_algorithm* algo, NodeID v, NodeID u,
                                         NodeWeight sum_weight_two_neigh_v);
};

/**
 * This class implements the two neighbor removal reduction.
 * It is the second case of the split neighbor removal reduction.
 */
struct two_neighbor_removal_w2pack : public split_neighbor_removal_w2pack {
        explicit two_neighbor_removal_w2pack(size_t n) : split_neighbor_removal_w2pack(n) {}
        ~two_neighbor_removal_w2pack() override = default;
        [[nodiscard]] reduction_w2pack_ptr clone() const final {
                return std::make_unique<two_neighbor_removal_w2pack>(*this);
        }

        [[nodiscard]] mw2ps_reduction_type get_reduction_type() const final {
                return mw2ps_reduction_type::two_neighbor_removal;
        }
        bool reduce(weighted_reduce_algorithm* algo) final;
};

template <class Last>
void make_w2reduction_vector_helper(std::vector<reduction_w2pack_ptr>& vec, size_t n) {
        vec.emplace_back(std::make_unique<Last>(n));
};

template <class First, class Second, class... Redus>
void make_w2reduction_vector_helper(std::vector<reduction_w2pack_ptr>& vec, size_t n) {
        vec.emplace_back(std::make_unique<First>(n));
        make_w2reduction_vector_helper<Second, Redus...>(vec, n);
};

template <class... Redus>
std::vector<reduction_w2pack_ptr> make_w2reduction_vector(size_t n) {
        std::vector<reduction_w2pack_ptr> vec;
        make_w2reduction_vector_helper<Redus...>(vec, n);
        return vec;
}

template <class... FastRedus>
std::vector<reduction_w2pack_ptr> make_core_w2reduction_vector(size_t n) {
        return make_w2reduction_vector<FastRedus..., neighborhood_removal_w2pack, fold2_w2pack, weight_transfer_w2pack,
                                       single_fast_domination_w2pack, direct_neighbor_removal_w2pack, domination_w2pack,
                                       two_neighbor_removal_w2pack, split_intersection_removal_w2pack,
                                       neighborhood_folding_w2pack>(n);
}

}  // namespace red2pack
#endif  // RED2PACK_WEIGHTED_REDUCTIONS_H
