// this reduce algorithm is based on the branch-and-reduce solver from KaMIS/wmis
#ifndef INC_2_PACKING_SET_REDUCTIONS_H
#define INC_2_PACKING_SET_REDUCTIONS_H

#include <array>
#include <memory>
#include <vector>

#include "data_structure/m2s_dynamic_graph.h"
#include "definitions.h"
#include "fast_set.h"
#include "sized_vector.h"

namespace red2pack {

class reduce_algorithm;

// Update this when more reductions are implemented.
enum m2ps_reduction_type { clique, domination, deg_one, twin, deg_two, fast_domination };
constexpr size_t m2ps_REDUCTION_NUM = 6;  // this is the number of the reductions

class vertex_marker_2pack {
       private:
        sized_vector<NodeID> current;
        sized_vector<NodeID> next;
        fast_set added_vertices;

       public:
        explicit vertex_marker_2pack(size_t size) : current(size), next(size), added_vertices(size){};

        void add(NodeID vertex) {
                if (!added_vertices.get(vertex)) {
                        next.push_back(vertex);
                        added_vertices.add(vertex);
                }
        }

        void get_next() {
                if (!next.empty()) {
                        current.swap(next);
                        clear_next();
                }
        }

        void clear_next() {
                next.clear();
                added_vertices.clear();
        }

        void fill_current_ascending(size_t n) {
                current.clear();
                for (size_t i = 0; i < n; i++) {
                        current.push_back(i);
                }
        }

        NodeID current_vertex(size_t index) { return current[index]; }

        size_t current_size() { return current.size(); }
};

struct general_reduction_2pack;
using reduction_2pack_ptr = std::unique_ptr<general_reduction_2pack>;
struct general_reduction_2pack {
        explicit general_reduction_2pack(size_t n) : marker(n) {}
        virtual ~general_reduction_2pack() = default;
        // clone returns not real pointer anymore -> loosing covariant return
        // this is no real problems as long as general_reduction_2pack is the only interface for reductions
        [[nodiscard]] virtual reduction_2pack_ptr clone() const = 0;

        [[nodiscard]] virtual m2ps_reduction_type get_reduction_type() const = 0;
        virtual bool reduce(reduce_algorithm* algo) = 0;
        virtual void restore(reduce_algorithm* algo) {}
        virtual void apply(reduce_algorithm* algo) {}

        bool has_run = false;
        vertex_marker_2pack marker;
};


struct deg_one_2reduction : public general_reduction_2pack {
        explicit deg_one_2reduction(size_t n) : general_reduction_2pack(n) {}
        ~deg_one_2reduction() override = default;
        [[nodiscard]] reduction_2pack_ptr clone() const final { return std::make_unique<deg_one_2reduction>(*this); }

        [[nodiscard]] m2ps_reduction_type get_reduction_type() const final { return m2ps_reduction_type::deg_one; }
        bool reduce(reduce_algorithm* algo) final;
};

struct deg_two_2reduction : public general_reduction_2pack {
        explicit deg_two_2reduction(size_t n) : general_reduction_2pack(n) {}
        ~deg_two_2reduction() override = default;
        [[nodiscard]] reduction_2pack_ptr clone() const final { return std::make_unique<deg_two_2reduction>(*this); }
        [[nodiscard]] m2ps_reduction_type get_reduction_type() const final { return m2ps_reduction_type::deg_two; }
        bool reduce(reduce_algorithm* algo) final;
};

struct twin2_reduction : public general_reduction_2pack {
        explicit twin2_reduction(size_t n) : general_reduction_2pack(n) {}
        ~twin2_reduction() override = default;
        [[nodiscard]] reduction_2pack_ptr clone() const final { return std::make_unique<twin2_reduction>(*this); }
        [[nodiscard]] m2ps_reduction_type get_reduction_type() const final { return m2ps_reduction_type::twin; }
        bool reduce(reduce_algorithm* algo) final;
};

struct fast_domination2_reduction : public general_reduction_2pack {
        explicit fast_domination2_reduction(size_t n) : general_reduction_2pack(n) {}
        ~fast_domination2_reduction() override = default;
        [[nodiscard]] reduction_2pack_ptr clone() const final { return std::make_unique<fast_domination2_reduction>(*this); }
        [[nodiscard]] m2ps_reduction_type get_reduction_type() const final {
                return m2ps_reduction_type::fast_domination;
        }
        bool reduce(reduce_algorithm* algo) final;
};

struct domination2_reduction : public general_reduction_2pack {
        explicit domination2_reduction(size_t n) : general_reduction_2pack(n) {}
        ~domination2_reduction() override = default;
        [[nodiscard]] reduction_2pack_ptr clone() const final { return std::make_unique<domination2_reduction>(*this); }

        [[nodiscard]] m2ps_reduction_type get_reduction_type() const final { return m2ps_reduction_type::domination; }
        bool reduce(reduce_algorithm* algo) final;
};

struct clique2_reduction : public general_reduction_2pack {
        explicit clique2_reduction(size_t n) : general_reduction_2pack(n) {}
        ~clique2_reduction() override = default;
        [[nodiscard]] reduction_2pack_ptr clone() const final { return std::make_unique<clique2_reduction>(*this); }

        [[nodiscard]] m2ps_reduction_type get_reduction_type() const final { return m2ps_reduction_type::clique; }
        bool reduce(reduce_algorithm* algo) final;
};

template <class Last>
void make_2reduction_vector_helper(std::vector<reduction_2pack_ptr>& vec, size_t n) {
        vec.emplace_back(std::make_unique<Last>(n));
};

template <class First, class Second, class... Redus>
void make_2reduction_vector_helper(std::vector<reduction_2pack_ptr>& vec, size_t n) {
        vec.emplace_back(std::make_unique<First>(n));
        make_2reduction_vector_helper<Second, Redus...>(vec, n);
};

template <class... Redus>
std::vector<reduction_2pack_ptr> make_2reduction_vector(size_t n) {
        std::vector<reduction_2pack_ptr> vec;
        make_2reduction_vector_helper<Redus...>(vec, n);
        return vec;
}

}  // namespace red2pack

#endif  // INC_2_PACKING_SET_REDUCTIONS_H
