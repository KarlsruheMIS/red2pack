#ifndef INC_2_PACKING_SET_M2S_DYNAMIC_GRAPH_H
#define INC_2_PACKING_SET_M2S_DYNAMIC_GRAPH_H

#include <algorithm>
#include <queue>
#include <vector>

#include "fast_set.h"
#include "m2s_graph_access.h"

namespace two_packing_set {

class evo_graph;
class m2s_dynamic_graph {
        // We keep a neighbor_list and a 2neighbor_list with similar properties.
        friend evo_graph;

       public:
        // need a two-neighbor list equivalent to neighbor list
        class two_neighbor_list {
                friend m2s_dynamic_graph;

               public:
                using iterator = std::vector<NodeID>::iterator;
                using const_iterator = std::vector<NodeID>::const_iterator;

                two_neighbor_list() = default;
                two_neighbor_list(size_t size) : two_neighbors(size){};
                two_neighbor_list(const std::vector<NodeID> &two_neighbors)
                    : two_neighbors(two_neighbors), counter(two_neighbors.size()) {}
                two_neighbor_list(std::vector<NodeID> &&two_neighbors)
                    : two_neighbors(std::move(two_neighbors)), counter(this->two_neighbors.size()) {}

                iterator begin() { return two_neighbors.begin(); }
                iterator end() { return two_neighbors.begin() + counter; }
                const_iterator begin() const { return two_neighbors.begin(); }
                const_iterator end() const { return two_neighbors.begin() + counter; }
                const_iterator cbegin() const { return two_neighbors.cbegin(); }
                const_iterator cend() const { return two_neighbors.cbegin() + counter; }

                size_t size() const noexcept { return counter; }
                void resize(size_t size) { two_neighbors.resize(size); }

                NodeID &operator[](size_t index) { return two_neighbors[index]; }
                const NodeID &operator[](size_t index) const { return two_neighbors[index]; }

               private:
                std::vector<NodeID> two_neighbors;
                size_t counter = 0;
        };

        class neighbor_list {
                friend m2s_dynamic_graph;

               public:
                using iterator = std::vector<NodeID>::iterator;
                using const_iterator = std::vector<NodeID>::const_iterator;

                neighbor_list() = default;
                neighbor_list(size_t size) : neighbors(size){};
                neighbor_list(const std::vector<NodeID> &neighbors) : neighbors(neighbors), counter(neighbors.size()) {}
                neighbor_list(std::vector<NodeID> &&neighbors)
                    : neighbors(std::move(neighbors)), counter(this->neighbors.size()) {}

                iterator begin() { return neighbors.begin(); }
                iterator end() { return neighbors.begin() + counter; }
                const_iterator begin() const { return neighbors.begin(); }
                const_iterator end() const { return neighbors.begin() + counter; }
                const_iterator cbegin() const { return neighbors.cbegin(); }
                const_iterator cend() const { return neighbors.cbegin() + counter; }

                size_t size() const noexcept { return counter; }
                void resize(size_t size) { neighbors.resize(size); }

                NodeID &operator[](size_t index) { return neighbors[index]; }
                const NodeID &operator[](size_t index) const { return neighbors[index]; }

               private:
                std::vector<NodeID> neighbors;
                size_t counter = 0;
        };

        m2s_dynamic_graph(size_t nodes = 0) : graph1(nodes), graph2(nodes) {
                graph1.reserve(nodes);
                graph2.reserve(nodes);
        }

        m2s_dynamic_graph(m2s_graph_access &G)
            : graph1(G.number_of_nodes()), graph2(G.number_of_nodes()), hided_nodes(G.number_of_nodes(), false) {
                neighbor_list *slotA;

                forall_nodes (G, node) {
                        slotA = &graph1[node];
                        slotA->resize(G.getNodeDegree(node));

                        forall_out_edges (G, edge, node) {
                                slotA->neighbors[slotA->counter++] = G.getEdgeTarget(edge);
                        }
                        endfor
                }
                endfor

                // 2-list.
                two_neighbor_list *slotB;

                for (size_t i = 0; i < G.number_of_nodes(); i++) {
                        slotB = &graph2[i];
                        slotB->resize(G.get2Degree(i));

                        forall_out_edges2(G, e, i) { slotB->two_neighbors[slotB->counter++] = G.getEdgeTarget2(e); }
                        endfor
                }
        }

        // hide node without losing information about connecting vertices
        void hide_node_imprecise(NodeID node) {
                hided_nodes[node] = true;
                // We have to hide the node in the one neighborhoods as well as in the
                // two_neighborhoods.
                for (auto neighbor : graph1[node]) {
                        hide_edge(neighbor, node);
                }
                for (auto two_neighbor : graph2[node]) {
                        hide_path(two_neighbor, node);
                }
        }

        // If we use this data structure we have to be carful, that we always use the
        // right function depending on which case we look at
        void hide_edge(NodeID source, NodeID target) {
                auto &slot = graph1[source];
                for (size_t pos = 0; pos < slot.counter; pos++) {
                        if (slot.neighbors[pos] == target) {
                                std::swap(slot.neighbors[pos], slot.neighbors[--slot.counter]);
                                return;
                        }
                }
        }
        // Only from neighbor perspective this "path" is deleted.
        void hide_path(NodeID source, NodeID target) {
                auto &slot = graph2[source];
                for (size_t pos = 0; pos < slot.counter; pos++) {
                        if (slot.two_neighbors[pos] == target) {
                                std::swap(slot.two_neighbors[pos], slot.two_neighbors[--slot.counter]);
                                return;
                        }
                }
        }

        // If we use this data structure we have to be carful, that we always use the
        // right function depending on which case we look at
        void add_edge_2(NodeID source, NodeID target) {
                auto &slot = graph2[source];
                for (size_t pos = 0; pos < slot.counter; pos++) {
                        if (slot.two_neighbors[pos] == target) return;
                }
                slot.two_neighbors[slot.counter++] = target;
                /* slot.two_neighbors[++slot.counter]=target;  */
                return;
        }

        neighbor_list &operator[](NodeID node) { return graph1[node]; }

        two_neighbor_list &get2neighbor_list(NodeID node) { return graph2[node]; }

        const neighbor_list &operator[](NodeID node) const { return graph1[node]; }

        size_t size() const noexcept { return graph1.size(); }

        std::vector<bool> hided_nodes;

       private:
        std::vector<neighbor_list> graph1;
        std::vector<two_neighbor_list> graph2;
};
}  // namespace two_packing_set

#endif  // INC_2_PACKING_SET_M2S_DYNAMIC_GRAPH_H
