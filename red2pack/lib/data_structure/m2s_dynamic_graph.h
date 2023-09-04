#ifndef INC_2_PACKING_SET_M2S_DYNAMIC_GRAPH_H
#define INC_2_PACKING_SET_M2S_DYNAMIC_GRAPH_H

#include <algorithm>
#include <queue>
#include <vector>

#include "fast_set.h"
#include "m2s_graph_access.h"

namespace red2pack {

class m2s_dynamic_graph {
       public:
        class neighbor_list {
                friend m2s_dynamic_graph;

               public:
                using iterator = std::vector<NodeID>::iterator;
                using const_iterator = std::vector<NodeID>::const_iterator;

                neighbor_list() = default;
                explicit neighbor_list(size_t size) : neighbors(size){};
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

               private:
                std::vector<NodeID> neighbors;
                size_t counter = 0;
        };

        explicit m2s_dynamic_graph(size_t nodes = 0)
            : one_neigh(nodes),
              two_neigh(nodes),
              hided_nodes(nodes, false),
              init_two_neighbors(nodes, false),
              set(nodes) {
                one_neigh.reserve(nodes);
                two_neigh.reserve(nodes);
        }

        m2s_dynamic_graph(m2s_graph_access &G, bool two_neighbors_initialized)
            : one_neigh(G.number_of_nodes()),
              two_neigh(G.number_of_nodes()),
              hided_nodes(G.number_of_nodes(), false),
              init_two_neighbors(G.number_of_nodes(), two_neighbors_initialized),
              set(G.number_of_nodes()) {
                neighbor_list *slotA;

                forall_nodes (G, node) {
                        slotA = &one_neigh[node];
                        slotA->resize(G.getNodeDegree(node));

                        forall_out_edges (G, edge, node) {
                                slotA->neighbors[slotA->counter++] = G.getEdgeTarget(edge);
                        }
                        endfor
                }
                endfor

                // two neighborhoods
                if (two_neighbors_initialized) {
                        neighbor_list *slotB;

                        for (size_t i = 0; i < G.number_of_nodes(); i++) {
                                slotB = &two_neigh[i];
                                slotB->resize(G.get2Degree(i));

                                forall_out_edges2(G, e, i) { slotB->neighbors[slotB->counter++] = G.getEdgeTarget2(e); }
                                endfor
                        }
                }
        }

        /**
         * Hides a node by hiding all 1-edges and 2-edges pointing to node
         * @param node
         */
        void hide_node(NodeID node) {
                hided_nodes[node] = true;

                // hide 1-edges pointing to node
                for (auto neighbor : one_neigh[node]) {
                        if (!hided_nodes[neighbor]) {
                                hide_edge(neighbor, node);
                        }
                }

                if (!init_two_neighbors[node]) {
                        // iterate 2-neigh of node to hide all 2-edges pointing to node
                        set.clear();
                        for (size_t k = 0; k < one_neigh[node].neighbors.size(); k++) {
                                set.add(one_neigh[node][k]);
                        }
                        set.add(node);
                        for (size_t k = 0; k < one_neigh[node].neighbors.size(); k++) {
                                auto target1 = one_neigh[node][k];
                                for (size_t l = 0; l < one_neigh[target1].counter; l++) {
                                        auto target2 = one_neigh[target1][l];
                                        if (!set.get(target2) && !hided_nodes[target2] && init_two_neighbors[target2]) {
                                                hide_two_edge(target2, node);
                                                set.add(target2);
                                        }
                                }
                        }
                } else {
                        // hide 2-edges pointing to node
                        for (auto two_neighbor : two_neigh[node]) {
                                if (!hided_nodes[two_neighbor] && init_two_neighbors[two_neighbor]) {
                                        hide_two_edge(two_neighbor, node);
                                }
                        }
                }
        }

        // hide the 1-edge (source, target)
        void hide_edge(NodeID source, NodeID target) {
                auto &slot = one_neigh[source];
                for (size_t pos = 0; pos < slot.counter; pos++) {
                        if (slot.neighbors[pos] == target) {
                                std::swap(slot.neighbors[pos], slot.neighbors[--slot.counter]);
                                return;
                        }
                }
        }

        // hide the 2-edge (source, target)
        void hide_two_edge(NodeID source, NodeID target) {
                auto &slot = two_neigh[source];
                for (size_t pos = 0; pos < slot.counter; pos++) {
                        if (slot.neighbors[pos] == target) {
                                std::swap(slot.neighbors[pos], slot.neighbors[--slot.counter]);
                                return;
                        }
                }
        }

        neighbor_list &operator[](NodeID node) { return one_neigh[node]; }

        neighbor_list &get2neighbor_list(NodeID node) {
                if (!init_two_neighbors[node]) {
                        set.clear();
                        for (size_t k = 0; k < one_neigh[node].neighbors.size(); k++) {
                                set.add(one_neigh[node][k]);
                        }
                        set.add(node);
                        for (size_t k = 0; k < one_neigh[node].neighbors.size(); k++) {
                                auto target1 = one_neigh[node][k];
                                for (size_t l = 0; l < one_neigh[target1].counter; l++) {
                                        auto target2 = one_neigh[target1][l];
                                        if (!set.get(target2) && !hided_nodes[target2]) {
                                                queue.push(target2);
                                                set.add(target2);
                                        }
                                }
                        }
                        two_neigh[node].resize(queue.size());
                        while (!queue.empty()) {
                                two_neigh[node][two_neigh[node].counter++] = queue.front();
                                queue.pop();
                        }
                        init_two_neighbors[node] = true;
                }
                return two_neigh[node];
        }

        const neighbor_list &operator[](NodeID node) const { return one_neigh[node]; }

        [[nodiscard]] size_t size() const noexcept { return one_neigh.size(); }

        std::vector<bool> hided_nodes;         // which vertices are hidden
        std::vector<bool> init_two_neighbors;  // for which vertices do we store the 2-edges

       private:
        std::vector<neighbor_list> one_neigh;  // stores 1-edges for each vertex
        std::vector<neighbor_list> two_neigh;  // stored 2-edges for each vertex
        fast_set set;
        std::queue<NodeID> queue;
};

}  // namespace red2pack

#endif  // INC_2_PACKING_SET_M2S_DYNAMIC_GRAPH_H
