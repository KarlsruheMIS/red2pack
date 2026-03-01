// this dynamic graphs is based on the dynamic graph from KaMIS
#ifndef INC_2_PACKING_SET_M2S_ADVANCED_DYNAMIC_GRAPH_H
#define INC_2_PACKING_SET_M2S_ADVANCED_DYNAMIC_GRAPH_H

#include <algorithm>
#include <functional>
#include <queue>
#include <vector>

#include "red2pack/data_structure/fast_set.h"
#include "red2pack/data_structure/m2s_graph_access.h"
#include "red2pack/data_structure/sized_vector.h"

namespace red2pack {

class m2s_advanced_dynamic_graph {
       public:
        class neighbor_list {
                friend m2s_advanced_dynamic_graph;

               public:
                using iterator = std::vector<NodeID>::iterator;
                using const_iterator = std::vector<NodeID>::const_iterator;

                neighbor_list() = default;
                explicit neighbor_list(size_t size) : neighbors(size) {};
                explicit neighbor_list(const std::vector<NodeID> &neighbors)
                    : neighbors(neighbors), counter(neighbors.size()) {}
                explicit neighbor_list(std::vector<NodeID> &&neighbors)
                    : neighbors(std::move(neighbors)), counter(this->neighbors.size()) {}

                iterator begin() { return neighbors.begin(); }
                iterator end() { return neighbors.begin() + counter; }
                [[nodiscard]] const_iterator begin() const { return neighbors.begin(); }
                [[nodiscard]] const_iterator end() const { return neighbors.begin() + counter; }
                [[nodiscard]] const_iterator cbegin() const { return neighbors.cbegin(); }
                [[nodiscard]] const_iterator cend() const { return neighbors.cbegin() + counter; }

                [[nodiscard]] size_t size() const noexcept { return counter; }
                void resize(size_t size) { neighbors.resize(size); }

                NodeID &operator[](size_t index) { return neighbors[index]; }
                const NodeID &operator[](size_t index) const { return neighbors[index]; }

                bool has_hidden_neigh() { return neighbors.size() - counter > 0; }
                NodeID capacity() { return neighbors.size(); }

               private:
                std::vector<NodeID> neighbors;
                size_t counter = 0;
        };

        explicit m2s_advanced_dynamic_graph(size_t nodes = 0)
            : init_link_neighbors(nodes, false),
              visible_nodes(nodes),
              hided_nodes(nodes, false),
              maintain_weight_stat(false),
              one_neigh(nodes),
              two_neigh(nodes),
              set(nodes),
              one_border_bulk_hide_set(nodes),
              two_border_bulk_hide_set(nodes),
              one_border_bulk_hide(nodes),
              two_border_bulk_hide(nodes) {
                one_neigh.reserve(nodes);
                two_neigh.reserve(nodes);
        }

        m2s_advanced_dynamic_graph(m2s_graph_access &G, bool two_neighbors_initialized, bool maintain_weight_stat,
                                   bool maintain_two_neighborhood)
            : init_link_neighbors(G.number_of_nodes(), two_neighbors_initialized),
              reset_link_neigh(G.number_of_nodes(), static_cast<int>(maintain_two_neighborhood)),
              maintain_two_neigh(maintain_two_neighborhood),
              two_neighbors_pre_initialized(two_neighbors_initialized),
              visible_nodes(G.number_of_nodes()),
              // do not reset two-neigh if all two neigh are init
              hided_nodes(G.number_of_nodes(), false),
              weights(G.number_of_nodes(), 0),
              maintain_weight_stat(maintain_weight_stat),
              weight_stats(G.number_of_nodes(), {0, 0, 0, true}),
              one_neigh(G.number_of_nodes()),
              two_neigh(G.number_of_nodes()),
              set(G.number_of_nodes()),
              one_border_bulk_hide_set(G.number_of_nodes()),
              two_border_bulk_hide_set(G.number_of_nodes()),
              one_border_bulk_hide(G.number_of_nodes()),
              two_border_bulk_hide(G.number_of_nodes()) {
                // set initial direct neighbors and weight
                neighbor_list *slotA;
                forall_nodes (G, node) {
                        slotA = &one_neigh[node];
                        slotA->resize(G.getNodeDegree(node));

                        forall_out_edges (G, edge, node) {
                                slotA->neighbors[slotA->counter++] = G.getEdgeTarget(edge);
                                ;
                        }
                        endfor

                        weights[node] = G.getNodeWeight(node);
                }
                endfor

                // weight statistics
                if (maintain_weight_stat) {
                        forall_nodes (G, node) {
                                NodeWeight weight = weights[node];

                                forall_out_edges (G, edge, node) {
                                        const auto target = G.getEdgeTarget(edge);

                                        weight_stats[target].sum_weight += weight;
                                        if (weight_stats[target].max_weight < weight) {
                                                weight_stats[target].max_weight = weight;
                                                weight_stats[target].count_max_weight_nodes = 1;
                                        } else if (weight_stats[target].max_weight == weight) {
                                                weight_stats[target].count_max_weight_nodes++;
                                        }
                                }
                                endfor
                        }
                        endfor
                }

                // set initial two neighborhoods if it is given
                if (two_neighbors_initialized) {
                        neighbor_list *slotB;
                        forall_nodes (G, node) {
                                slotB = &two_neigh[node];
                                slotB->resize(G.getLinkDegree(node));

                                forall_out_links(G, e, node) {
                                        slotB->neighbors[slotB->counter++] = G.getLinkTarget(e);
                                }
                                endfor
                        }
                        endfor
                }
        }

       private:
        void impl_hide_two_neigh_with_preinitialized_two_neigh(NodeID node) {
                RED2PACK_ASSERT_TRUE(two_neighbors_pre_initialized);
                RED2PACK_ASSERT_TRUE(init_link_neighbors[node]);
                // two neighborhood is known
                for (auto two_neighbor : two_neigh[node]) {
                        if (visible(two_neighbor)) {
                                hide_link(two_neighbor, node);
                        }
                }
        }

        void impl_hide_two_neigh_with_stored_two_neigh(NodeID node) {
                RED2PACK_ASSERT_TRUE(!two_neighbors_pre_initialized);
                RED2PACK_ASSERT_TRUE(init_link_neighbors[node]);
                // two neighborhood is known but maybe not for neighbors
                // hide 2-edges pointing to node
                for (auto two_neighbor : two_neigh[node]) {
                        if (init_link_neighbors[two_neighbor]) {
                                RED2PACK_ASSERT_TRUE(visible(two_neighbor));
                                if (reset_link_neigh_allowed(two_neighbor)) {
                                        init_link_neighbors[two_neighbor] = false;
                                } else {
                                        hide_link(two_neighbor, node);
                                }
                        }
                }
        }

        void impl_hide_two_neigh_without_stored_two_neigh(NodeID node) {
                RED2PACK_ASSERT_TRUE(!two_neighbors_pre_initialized);
                RED2PACK_ASSERT_TRUE(!init_link_neighbors[node]);
                // iterate over 2-neigh of node to hide all 2-edges pointing to node
                set.clear();
                for (size_t k = 0; k < one_neigh[node].neighbors.size(); k++) {
                        set.add(one_neigh[node][k]);
                }
                set.add(node);
                for (size_t k = 0; k < one_neigh[node].neighbors.size(); k++) {
                        auto target1 = one_neigh[node][k];
                        for (size_t l = 0; l < one_neigh[target1].counter; l++) {
                                auto target2 = one_neigh[target1][l];
                                if (!set.get(target2) && visible(target2) && init_link_neighbors[target2]) {
                                        if (reset_link_neigh_allowed(target2)) {
                                                init_link_neighbors[target2] = false;
                                        } else {
                                                hide_link(target2, node);
                                        }

                                        set.add(target2);
                                }
                        }
                }
        }

       public:
        /**
         * Hides a node by hiding all 1-edges and 2-edges pointing to node.
         * @param node
         */
        void hide_node(NodeID node, bool hide_two_edges = true) {
                RED2PACK_ASSERT_TRUE(!hided_nodes[node]);
                set_hidden(node);

                // hide 1-edges pointing to node
                for (auto neighbor : one_neigh[node]) {
                        RED2PACK_ASSERT_TRUE(visible(neighbor));
                        hide_edge(neighbor, node);
                }

                // dispatch implementation to hide 2-edges pointing to node
                if (hide_two_edges) {
                        if (two_neighbors_pre_initialized) {
                                impl_hide_two_neigh_with_preinitialized_two_neigh(node);
                        } else if (!init_link_neighbors[node]) {
                                impl_hide_two_neigh_without_stored_two_neigh(node);
                        } else {
                                impl_hide_two_neigh_with_stored_two_neigh(node);
                        }
                }
        }

        /**
         * Hide a set of nodes together sorted by their node id. This bulk operation is useful if multiple edges are
         * removed the dyanmic neighborhoods, e.g. removing a clique. This hide-operation can only be correctly be
         * undone by restoring *all* these nodes in reversed order.
         * @param hide_nodes nodes to hide
         * @param hide_node_set contains exactly the nodes from @hide_nodes
         * @param input_is_sorted must be set to false if the input is not sorted ascending by the node id (default:
         * true)
         */
        void bulk_hide(sized_vector<NodeID> &hide_nodes, fast_set &hide_nodes_set, bool input_is_sorted = false) {
                if (!input_is_sorted) {
                        std::sort(hide_nodes.begin(), hide_nodes.end());
                }

                one_border_bulk_hide_set.clear();
                two_border_bulk_hide_set.clear();
                one_border_bulk_hide.clear();
                two_border_bulk_hide.clear();

                for (auto node : hide_nodes) {
                        set_hidden(node);
                        for (std::size_t pos = 0; pos < one_neigh[node].size(); ++pos) {
                                auto &target = one_neigh[node][pos];
                                if (hide_nodes_set.get(static_cast<int>(target))) {
                                        if (target < node) {
                                                one_neigh[node].counter--;
                                                std::swap(target, one_neigh[node][one_neigh[node].size()]);
                                                --pos;
                                        }
                                } else {
                                        // hide_edge(target, node);
                                        if (one_border_bulk_hide_set.add(static_cast<int>(target))) {
                                                one_border_bulk_hide.push_back(target);
                                        }
                                }
                        }
                        for (std::size_t pos = 0; pos < neigh2(node).size(); ++pos) {
                                auto &target = neigh2(node)[pos];
                                if (hide_nodes_set.get(static_cast<int>(target))) {
                                        if (target < node) {
                                                neigh2(node).counter--;
                                                std::swap(target, neigh2(node)[neigh2(node).size()]);
                                                --pos;
                                        }
                                } else {
                                        // hide_two_edge(target, node);
                                        if (two_border_bulk_hide_set.add(static_cast<int>(target))) {
                                                two_border_bulk_hide.push_back(target);
                                        }
                                }
                        }
                }

                for (auto target : one_border_bulk_hide) {
                        bulk_hide_edge(target, hide_nodes_set);
                }
                for (auto target : two_border_bulk_hide) {
                        bulk_hide_link(target, hide_nodes_set);
                }
        }

       private:
        void impl_restore_two_neigh_with_preinitialized_two_neigh(NodeID node) {
                RED2PACK_ASSERT_TRUE(two_neighbors_pre_initialized);
                RED2PACK_ASSERT_TRUE(init_link_neighbors[node]);
                for (auto two_neighbor : two_neigh[node]) {
                        if (visible(two_neighbor)) {
                                RED2PACK_ASSERT_TRUE(init_link_neighbors[two_neighbor]);
                                two_neigh[two_neighbor].counter++;
                        }
                }
        }
        void impl_restore_two_neigh(NodeID node) {
                for (auto two_neighbor : two_neigh[node]) {
                        if (visible(two_neighbor) && init_link_neighbors[two_neighbor]) {
                                if (reset_link_neigh_allowed(two_neighbor)) {
                                        // 2-neighborhood of two neighbor might not capture node
                                        init_link_neighbors[two_neighbor] = false;
                                } else {
                                        // 2-neighborhood of two_neighbor is still not allowed to be reset hence
                                        // it must capture node
                                        if (two_neigh[two_neighbor].counter ==
                                            two_neigh[two_neighbor].neighbors.size()) {
                                                // two-neighborhood of two_neighbor was initialized after hiding
                                                // of node; by restoring node, it is not longer valid (not
                                                // pointer to node)
                                                init_link_neighbors[two_neighbor] = false;
                                        } else {
                                                two_neigh[two_neighbor].counter++;
                                        }
                                        RED2PACK_ASSERT_TRUE(two_neigh[two_neighbor].counter <=
                                                             two_neigh[two_neighbor].neighbors.size());
                                }
                        }
                }
        }

       public:
        /**
         * Restore a hidden node. Note that if @node was part of a bulk hide operation, edges pointing to @node
         * will be restored when all nodes from the bulk hide operation have been restored.
         * @param node the node to restore
         */
        void restore_node(NodeID node) {
                RED2PACK_ASSERT_TRUE(!visible(node));
                set_visible(node);
                weight_stats[node].init = false;

                // restore one-edges pointing to node
                for (auto neighbor : one_neigh[node]) {
                        RED2PACK_ASSERT_TRUE(visible(neighbor));
                        restore_edge(neighbor, node);
                }

                // dispatch restore two-edges implementation
                if (two_neighbors_pre_initialized) {
                        impl_restore_two_neigh_with_preinitialized_two_neigh(node);
                } else {
                        impl_restore_two_neigh(node);
                }
        }

        void restore_two_edge_and_replace(NodeID node, NodeID replacement,
                                          bool backup_in_last_hidden_one_edge = false) {
                if (!init_link_neighbors[node]) {
                        set.clear();
                        RED2PACK_ASSERT_TRUE(queue.empty());
                        for (size_t k = 0; k < one_neigh[node].neighbors.size(); k++) {
                                set.add(static_cast<int>(one_neigh[node][k]));
                        }
                        set.add(static_cast<int>(node));
                        for (size_t k = 0; k < one_neigh[node].neighbors.size(); k++) {
                                auto target1 = one_neigh[node][k];
                                for (size_t l = 0; l < one_neigh[target1].counter; l++) {
                                        auto target2 = one_neigh[target1][l];
                                        if (!set.get(static_cast<int>(target2)) && visible(target2)) {
                                                queue.push(static_cast<int>(target2));
                                                set.add(static_cast<int>(target2));
                                        }
                                }
                        }
                        two_neigh[node].resize(queue.size() + 1);  // we need one more slot
                        two_neigh[node].counter = 0;
                        while (!queue.empty()) {
                                two_neigh[node][two_neigh[node].counter++] = queue.front();
                                queue.pop();
                        }
                        init_link_neighbors[node] = true;
                        // on_demand_two_neighborhood_stack.push_back(node);
                        RED2PACK_ASSERT_TRUE(two_neigh[node].counter + 1 == two_neigh[node].neighbors.size());
                }
                // we might need to adjust space
                // if noval 2-edges are introduced (see folding)
                auto &slot = two_neigh[node];
                RED2PACK_ASSERT_TRUE(slot.counter <= slot.neighbors.size());
                if (slot.counter == slot.neighbors.size()) {
                        slot.resize(slot.size() + 1);
                } else if (backup_in_last_hidden_one_edge) {
                        // backup latest hidden edge in the one neighborhood of end
                        // there is at least one hidden edge in the one neighborhood
                        // we assume that we can remember this hidden one neighbor
                        RED2PACK_ASSERT_TRUE(one_neigh[node].size() < one_neigh[node].neighbors.size());
                        *(one_neigh[node].end()) = slot.neighbors[slot.counter];
                }
                slot.neighbors[slot.counter++] = replacement;
                RED2PACK_ASSERT_TRUE(slot.neighbors[slot.counter - 1] == replacement);
                RED2PACK_ASSERT_TRUE(slot.counter <= slot.neighbors.size());
        }

        void replace_last_hidden_two_edge(NodeID node, NodeID replacement) {
                RED2PACK_ASSERT_TRUE(init_link_neighbors[node]);
                *(two_neigh[node].end()) = replacement;
        }

        void replace_last_restored_two_edge(NodeID node, NodeID replacement) {
                RED2PACK_ASSERT_TRUE(init_link_neighbors[node]);
                *(two_neigh[node].end() - 1) = replacement;
        }

        void replace_last_restored_one_edge(NodeID node, NodeID replacement) {
                *(one_neigh[node].end() - 1) = replacement;
        }

        neighbor_list &operator[](NodeID node) { return one_neigh[node]; }
        const neighbor_list &operator[](NodeID node) const { return one_neigh[node]; }

        neighbor_list &neigh2(NodeID node) {
                if (!init_link_neighbors[node]) {
                        set.clear();
                        RED2PACK_ASSERT_TRUE(queue.empty());
                        for (size_t k = 0; k < one_neigh[node].neighbors.size(); k++) {
                                set.add(one_neigh[node][k]);
                        }
                        set.add(node);
                        for (size_t k = 0; k < one_neigh[node].neighbors.size(); k++) {
                                auto target1 = one_neigh[node][k];
                                auto &slotA = one_neigh[target1];
                                for (size_t l = 0; l < slotA.counter; l++) {
                                        auto target2 = slotA[l];

                                        if (!set.get(target2) && visible(target2)) {
                                                queue.push(target2);
                                                set.add(target2);
                                        }
                                }
                        }
                        two_neigh[node].resize(queue.size());
                        two_neigh[node].counter = 0;
                        while (!queue.empty()) {
                                two_neigh[node][two_neigh[node].counter++] = queue.front();
                                queue.pop();
                        }
                        init_link_neighbors[node] = true;
                        // on_demand_two_neighborhood_stack.push_back(node);
                        RED2PACK_ASSERT_TRUE(two_neigh[node].counter == two_neigh[node].neighbors.size());
                }

                return two_neigh[node];
        }

        [[nodiscard]] NodeID visible(NodeID node) const { return !hided_nodes[node]; }
        [[nodiscard]] NodeID num_visible_nodes() const { return visible_nodes; }
        [[nodiscard]] size_t size() const noexcept { return one_neigh.size(); }

        [[nodiscard]] NodeID deg(NodeID node) const noexcept { return one_neigh[node].size(); }

        NodeID deg2(NodeID node) noexcept { return neigh2(node).size(); }

        NodeWeight get_max_weight_of_neigh(NodeID node) {
                if (maintain_weight_stat) {
                        if (!weight_stats[node].init) {
                                init_weight_stat(node);
                        }
                        return weight_stats[node].max_weight;
                } else {
                        NodeWeight max_weight = 0;
                        std::for_each(one_neigh[node].begin(), one_neigh[node].end(),
                                      [&w = this->weights, &max_weight](NodeID neighbor) {
                                              if (max_weight < w[neighbor]) {
                                                      max_weight = w[neighbor];
                                              }
                                      });
                        return max_weight;
                }
        }

        NodeWeight get_sum_weight_of_neigh(NodeID node) {
                if (maintain_weight_stat) {
                        if (!weight_stats[node].init) {
                                init_weight_stat(node);
                        }

                        return weight_stats[node].sum_weight;
                } else {
                        NodeWeight sum_weight = 0;
                        std::for_each(
                            one_neigh[node].begin(), one_neigh[node].end(),
                            [&w = this->weights, &sum_weight](NodeID neighbor) { sum_weight += w[neighbor]; });
                        return sum_weight;
                }
        }

        bool is_weight_stat_init(NodeID node) { return weight_stats[node].init; }

        [[nodiscard]] NodeWeight weight(NodeID node) const { return weights[node]; }

        void set_weight(NodeID node, NodeWeight weight) {
                if (maintain_weight_stat && weight != weights[node]) {
                        for (NodeID &target : one_neigh[node]) {
                                auto &stat = weight_stats[target];
                                if (stat.init) {
                                        stat.sum_weight -= weights[node];
                                        stat.sum_weight += weight;
                                        if (stat.max_weight == weights[node]) {
                                                if (weights[node] < weight) {
                                                        stat.max_weight = weight;
                                                        stat.count_max_weight_nodes = 1;
                                                } else {
                                                        stat.count_max_weight_nodes--;
                                                        if (stat.count_max_weight_nodes == 0) {
                                                                stat.init = false;
                                                        }
                                                }
                                        }
                                }
                        }
                }
                weights[node] = weight;
        }

        void set_visible(NodeID node) {
                visible_nodes += static_cast<NodeID>(hided_nodes[node]);
                hided_nodes[node] = false;
        }
        void set_hidden(NodeID node) {
                visible_nodes -= static_cast<NodeID>(!hided_nodes[node]);
                hided_nodes[node] = true;
        }

        [[nodiscard]] const sized_vector<NodeID> &get_last_one_border_bulk_hide() const { return one_border_bulk_hide; }

        [[nodiscard]] const sized_vector<NodeID> &get_last_two_border_bulk_hide() const { return two_border_bulk_hide; }

        std::vector<bool> init_link_neighbors;  // for which vertices do we store the 2-edges
        std::vector<int> reset_link_neigh;      // for which vertices do we store the 2-edges

        void disable_reset_link_neigh(NodeID v) { reset_link_neigh[v]++; }

        void enable_reset_link_neigh(NodeID v) { reset_link_neigh[v]--; }

        bool reset_link_neigh_allowed(NodeID v) { return reset_link_neigh[v] <= 0; }

        bool maintain_two_neigh = false;
        bool two_neighbors_pre_initialized = false;

       private:
        NodeID visible_nodes;
        std::vector<bool> hided_nodes;  // which vertices are hidden
        std::vector<NodeWeight> weights;

        // maintain weights;
        bool maintain_weight_stat;
        struct weight_stat {
                // stats about the one neighborhood
                NodeWeight max_weight;
                NodeID count_max_weight_nodes;
                NodeWeight sum_weight;
                bool init;
        };
        std::vector<weight_stat> weight_stats;

        std::vector<neighbor_list> one_neigh;  // stores 1-edges for each vertex
        std::vector<neighbor_list> two_neigh;  // stored 2-edges for each vertex
        fast_set set;
        fast_set one_border_bulk_hide_set;
        fast_set two_border_bulk_hide_set;
        sized_vector<NodeID> one_border_bulk_hide;
        sized_vector<NodeID> two_border_bulk_hide;
        std::queue<NodeID> queue;

        void init_weight_stat(NodeID node) {
                auto &stat = weight_stats[node];
                stat.max_weight = 0;
                stat.sum_weight = 0;
                stat.count_max_weight_nodes = 0;
                std::for_each(one_neigh[node].begin(), one_neigh[node].end(),
                              [&w = this->weights, &stat](NodeID neighbor) {
                                      if (stat.max_weight < w[neighbor]) {
                                              stat.max_weight = w[neighbor];
                                              stat.count_max_weight_nodes = 1;
                                      } else if (stat.max_weight == w[neighbor]) {
                                              stat.count_max_weight_nodes += 1;
                                      }
                                      stat.sum_weight += w[neighbor];
                              });
                stat.init = true;
        }

        void update_weight_stat_on_hide_edge(NodeID source, NodeID target) {
                RED2PACK_ASSERT_TRUE(maintain_weight_stat);
                if (weight_stats[source].init) {
                        weight_stats[source].sum_weight -= weights[target];
                        if (weights[target] == weight_stats[source].max_weight) {
                                weight_stats[source].count_max_weight_nodes--;
                                if (weight_stats[source].count_max_weight_nodes == 0) {
                                        weight_stats[source].init = false;
                                }
                        }
                }
        }

        void hide_edge(NodeID source, NodeID target) {
                auto &slot = one_neigh[source];
                for (size_t pos = 0; pos < slot.counter; pos++) {
                        if (slot.neighbors[pos] == target) {
                                std::swap(slot.neighbors[pos], slot.neighbors[--slot.counter]);
                                break;
                        }
                }
                // update weight stat
                if (maintain_weight_stat) {
                        update_weight_stat_on_hide_edge(source, target);
                }
        }

        void bulk_hide_edge(NodeID source, fast_set &target_set) {
                auto &slot = one_neigh[source];

                for (size_t pos = 0; pos < slot.counter; pos++) {
                        auto target = slot.neighbors[pos];
                        if (target_set.get(static_cast<int>(target))) {
                                std::swap(slot.neighbors[pos], slot.neighbors[--slot.counter]);

                                // update weight stat
                                if (maintain_weight_stat) {
                                        update_weight_stat_on_hide_edge(source, target);
                                }

                                --pos;
                        }
                }
        }

        // hide the link (source, target)
        void hide_link(NodeID source, NodeID target) {
                auto &slot = two_neigh[source];
                for (size_t pos = 0; pos < slot.counter; pos++) {
                        if (slot.neighbors[pos] == target) {
                                std::swap(slot.neighbors[pos], slot.neighbors[--slot.counter]);
                                return;
                        }
                }
        }

        void bulk_hide_link(NodeID source, fast_set &target_set) {
                auto &slot = two_neigh[source];
                for (size_t pos = 0; pos < slot.counter; pos++) {
                        auto target = slot.neighbors[pos];
                        if (target_set.get(static_cast<int>(target))) {
                                std::swap(slot.neighbors[pos], slot.neighbors[--slot.counter]);
                                --pos;
                        }
                }
        }

        void restore_edge(NodeID source, NodeID target) {
                one_neigh[source].counter++;

                // update weight stat
                if (maintain_weight_stat && weight_stats[source].init) {
                        weight_stats[source].sum_weight += weights[target];
                        if (weights[target] == weight_stats[source].max_weight) {
                                weight_stats[source].count_max_weight_nodes++;
                                if (weight_stats[source].count_max_weight_nodes == 0) {
                                        weight_stats[source].init = false;
                                }
                        } else if (weights[target] >= weight_stats[source].max_weight) {
                                weight_stats[source].count_max_weight_nodes = 0;
                                weight_stats[source].max_weight = weights[target];
                        }
                }
        }
};

}  // namespace red2pack

#endif  // INC_2_PACKING_SET_M2S_ADVANCED_DYNAMIC_GRAPH_H
