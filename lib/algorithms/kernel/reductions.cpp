#include "reductions.h"
#include "reduce_algorithm.h"

namespace red2pack {

using pack_status=reduce_algorithm::two_pack_status;

bool deg_one_2reduction::reduce(reduce_algorithm* algo) {
        auto config = algo->config;
        if (config.disable_deg_one) return false;
        auto& status = algo->global_status;
        size_t oldn = status.remaining_nodes;

        for (size_t v_idx = 0; v_idx < marker.current_size(); v_idx++) {
                NodeID v = marker.current_vertex(v_idx);

                if (status.node_status[v] == pack_status::not_set) {
                        // check and apply reductions with degree <= 1
                        if (algo->reduce_deg_leq_one(v)) {
                                algo->set(v, pack_status::included);
                        }
                }
        }

        std::cout << "degree_one_reduction:" << oldn - status.remaining_nodes << std::endl;
        return oldn != status.remaining_nodes;
}

bool deg_two_2reduction::reduce(reduce_algorithm* algo) {
        auto config = algo->config;
        if (config.disable_deg_two) return false;
        auto& status = algo->global_status;
        size_t oldn = status.remaining_nodes;

        auto is_within_triangle = [&status = status, &algo = algo](NodeID& v) {
                // special case of clique reductions
                return algo->two_deg(v) == 0 && algo->deg(status.graph[v][0]) == 2 &&
                       algo->deg(status.graph[v][1]) == 2;
        };

        auto is_within_4_cycle = [&status = status, &algo = algo](NodeID& v) {
                // a 4-cycle check
                // 2-neighborhood of v consists only of some vertex x which is the 2nd neighbor of v's neighbors
                if (algo->two_deg(v) == 1 && algo->deg(status.graph[v][0]) == 2 && algo->deg(status.graph[v][1]) == 2 &&
                    !algo->is_adj(status.graph[v][0], status.graph[v][1])) {
                        return true;
                }
                return false;
        };

        for (size_t v_idx = 0; v_idx < marker.current_size(); v_idx++) {
                NodeID v = marker.current_vertex(v_idx);

                if (status.node_status[v] == pack_status::not_set) {
                        if (algo->reduce_deg_leq_one(v)) {
                                algo->set(v, pack_status::included);
                        } else if (algo->deg(v) == 2) {
                                if (is_within_triangle(v) || is_within_4_cycle(v)) {
                                        // triangle or 4-cycle
                                        algo->set(v, pack_status::included);
                                } else if (algo->two_deg(v) == 0) {
                                        // test for v_shape variant
                                        auto& neighbor1 = status.graph[v][0];
                                        auto& neighbor2 = status.graph[v][1];
                                        // check if deg-1 reduction applies either for neighbor 1 or 2
                                        // otherwise it is v-shape and we can include v
                                        if (algo->deg(neighbor1) == 1 && algo->two_deg(neighbor1) == 1) {
                                                algo->set(neighbor1, pack_status::included);
                                                continue;
                                        }
                                        if (algo->deg(neighbor2) == 1 && algo->two_deg(neighbor2) == 1) {
                                                algo->set(neighbor2, pack_status::included);
                                                continue;
                                        }
                                        // both neighbors have two_deg > 1
                                        algo->set(v, pack_status::included);
                                }
                        }
                }
        }

        std::cout << "degree_two_2reduction:" << oldn - status.remaining_nodes << std::endl;
        return oldn != status.remaining_nodes;
}

bool twin2_reduction::reduce(reduce_algorithm* algo) {
        auto config = algo->config;
        if (config.disable_twin) return false;
        auto& status = algo->global_status;
        size_t oldn = status.remaining_nodes;
        auto& neighbors_set = algo->set_1;

        for (size_t v_idx = 0; v_idx < marker.current_size(); v_idx++) {
                NodeID v = marker.current_vertex(v_idx);

                if (status.node_status[v] == pack_status::not_set) {
                        if (algo->reduce_deg_leq_one(v)) {
                                algo->set(v, pack_status::included);
                        } else if (algo->deg(v) == 2) {
                                neighbors_set.clear();
                                NodeID neighbor1 = status.graph[v][0];
                                NodeID neighbor2 = status.graph[v][1];

                                // Jannick: before: algo->deg(neighbor1) == 3 && algo->deg(neighbor2) == 3
                                //  too strong? now covering V-Shape
                                if (algo->deg(neighbor1) == algo->deg(neighbor2) &&
                                    algo->two_deg(v) == algo->deg(neighbor2) - 1) {
                                        for (NodeID neighbor : status.graph[neighbor1]) {
                                                neighbors_set.add(neighbor);
                                        }

                                        // ensure that neighbor1 and neighbor2 are not adjacent
                                        if (neighbors_set.get(neighbor2)) {
                                                continue;
                                        }

                                        bool equal = true;

                                        for (NodeID neighbor : status.graph[neighbor2]) {
                                                if (!neighbors_set.get(neighbor)) {
                                                        equal = false;
                                                        break;
                                                }
                                        }

                                        if (equal) {
                                                algo->set(v, pack_status::included);
                                        }
                                }
                        }
                }
        }

        std::cout << "twin_reduction:" << oldn - status.remaining_nodes << std::endl;
        return oldn != status.remaining_nodes;
}

bool fast_domination2_reduction::reduce(reduce_algorithm* algo) {
        auto config = algo->config;
        if (config.disable_fast_domination) return false;
        auto& status = algo->global_status;
        auto& neighbors = algo->set_1;
        size_t oldn = status.remaining_nodes;

        for (size_t v_idx = 0; v_idx < marker.current_size(); v_idx++) {
                NodeID v = marker.current_vertex(v_idx);

                if (status.node_status[v] == pack_status::not_set) {
                        if(algo->reduce_deg_leq_one(v)) {
                                algo->set(v, pack_status::included);
                                continue;
                        }


                        size_t neighbors_count = 0;
                        neighbors.clear();

                        for (NodeID neighbor : status.graph[v]) {
                                neighbors.add(neighbor);
                                neighbors_count++;
                        }

                        neighbors.add(v);
                        bool is_subset;

                        for (NodeID u : status.graph[v]) {
                                if (algo->deg(u) + algo->two_deg(u) > neighbors_count) {
                                        continue;
                                }

                                is_subset = true;

                                for (NodeID neighbor : status.graph[u]) {
                                        if (!neighbors.get(neighbor)) {
                                                is_subset = false;
                                                break;
                                        }
                                }

                                if (is_subset) {
                                        algo->set(v, pack_status::excluded);
                                        break;
                                }
                        }
                }
        }

        std::cout << "fast_domination_reduction:" << oldn - status.remaining_nodes << std::endl;
        return oldn != status.remaining_nodes;
}

bool domination2_reduction::reduce(reduce_algorithm* algo) {
        auto config = algo->config;
        if (config.disable_domination) return false;
        auto& status = algo->global_status;
        auto& neighbors = algo->set_1;
        size_t oldn = status.remaining_nodes;

        for (size_t v_idx = 0; v_idx < marker.current_size(); v_idx++) {
                NodeID v = marker.current_vertex(v_idx);

                if (status.node_status[v] == pack_status::not_set) {
                        if(algo->reduce_deg_leq_one(v)) {
                                algo->set(v, pack_status::included);
                                continue;
                        }

                        size_t neighbors_count = 0;
                        neighbors.clear();

                        for (NodeID neighbor : status.graph[v]) {
                                neighbors.add(neighbor);
                                neighbors_count++;
                        }

                        for (NodeID neighbor : status.graph.get2neighbor_list(v)) {
                                neighbors.add(neighbor);
                                neighbors_count++;
                        }

                        neighbors.add(v);
                        bool is_subset;

                        for (NodeID u : status.graph[v]) {
                                if (algo->deg(u) + algo->two_deg(u) > neighbors_count) {
                                        continue;
                                }

                                is_subset = true;

                                for (NodeID neighbor : status.graph[u]) {
                                        if (!neighbors.get(neighbor)) {
                                                is_subset = false;
                                                break;
                                        }
                                }
                                if (is_subset) {
                                        for (NodeID neighbor : status.graph.get2neighbor_list(u)) {
                                                if (!neighbors.get(neighbor)) {
                                                        is_subset = false;
                                                        break;
                                                }
                                        }
                                }

                                if (is_subset) {
                                        algo->set(v, pack_status::excluded);
                                        break;
                                }
                        }
                }
        }

        std::cout << "domination_reduction:" << oldn - status.remaining_nodes << std::endl;
        return oldn != status.remaining_nodes;
}

bool clique2_reduction::reduce(reduce_algorithm* algo) {
        auto config = algo->config;
        if (config.disable_clique) return false;

        auto& status = algo->global_status;
        auto& neighbors_set = algo->set_1;
        auto& neighbors = algo->buffers[0];

        size_t oldn = status.remaining_nodes;

        for (size_t node_idx = 0; node_idx < marker.current_size(); node_idx++) {
                NodeID node = marker.current_vertex(node_idx);

                if (status.node_status[node] == pack_status::not_set) {
                        neighbors.clear();
                        neighbors_set.clear();
                        neighbors_set.add(node);
                        NodeID c = 0;

                        // gather all nodes of N2[node] (potential 2-clique)
                        for (NodeID neighbor : status.graph[node]) {
                                // add neighbors with distance one.
                                neighbors.push_back(neighbor);
                                neighbors_set.add(neighbor);
                        }

                        for (NodeID neighbor2 : status.graph.get2neighbor_list(node)) {
                                // add neighbors with distance two.
                                neighbors.push_back(neighbor2);
                                neighbors_set.add(neighbor2);
                        }

                        // check if 2-clique

                        bool is_clique = true;

                        // TODO: is this correct?
                        //  Checks whether N2[node] is 2-clique (here every two different nodes of N2[node]
                        //   are connected over at *most* 2 vertices)
                        for (auto neighbor : neighbors) {
                                size_t count = 0;

                                for (NodeID neighbor_2nd : status.graph[neighbor]) {
                                        if (neighbors_set.get(neighbor_2nd)) count++;
                                }

                                // same for the neighbors with distance two
                                for (NodeID neighbor_2nd : status.graph.get2neighbor_list(neighbor)) {
                                        if (neighbors_set.get(neighbor_2nd)) count++;
                                }

                                is_clique = count == neighbors.size();
                                if (count != neighbors.size()) {
                                        is_clique = false;
                                        break;
                                }
                        }
                        if (is_clique) {
                                algo->set(node, pack_status::included);
                        }
                        continue;
                }
        }

        std::cout << "clique_reduction:" << oldn - status.remaining_nodes << std::endl;
        return oldn != status.remaining_nodes;
}

}  // namespace red2pack