#include "reductions.h"

#include <utility>

#include "reduce_algorithm.h"

namespace two_packing_set {

typedef reduce_algorithm::pack_status pack_status;

bool deg_one_2reduction_e::reduce(reduce_algorithm* algo) {
        auto config = algo->config;
        if (config.disable_deg_one) return false;
        auto& status = algo->global_status;
        size_t oldn = status.remaining_nodes;

        // checks if v is completely isolated (no edges and 2-edges incident to v)
        auto is_isolated = [&algo = algo](NodeID& v) { return algo->deg(v) == 0 && algo->two_deg(v) == 0; };
        // check if v has degree 1 and N2[v] == N[neighbor_of_v]
        auto is_deg1_2isolated = [&algo = algo](NodeID& v) {
                return algo->deg(v) == 1 && algo->two_deg(v) + 1 == algo->deg(algo->global_status.graph[v][0]);
        };

        for (size_t v_idx = 0; v_idx < marker.current_size(); v_idx++) {
                NodeID v = marker.current_vertex(v_idx);

                /*if (status.node_status[v] == pack_status::unsafe) {
                        if (algo->deg(v) <= 1) {
                                algo->set(v, pack_status::excluded);
                        }
                }

                if (status.node_status[v] == pack_status::not_set) {
                        if (algo->deg(v) == 0 && algo->two_deg(v) <= 1) {
                                // TODO: can this case exist
                                std::cout << "special case deg 1" << std::endl;
                                algo->set(v, pack_status::included);
                                continue;

                        } else if (algo->deg(v) == 1 && algo->two_deg(v) == 0) {
                                // TODO: v has deg 1 and empty 2-neighborhood
                                //   why does the 2-neighborhood must be empty?
                                algo->set(v, pack_status::included);
                                continue;
                        }
                }*/

                // Jannick: new variant impl of deg-1 reduction
                // Paper: deg-1 is sufficient -- but this is not sufficient after 2-neighborhood information for N3[v]
                // are left
                //      we need that N2[v]==N[neighbor_of_v] if v has deg 1
                if (status.node_status[v] == pack_status::not_set) {
                        if (is_isolated(v) || is_deg1_2isolated(v)) {
                                // this removes N2[v] of G and only leaves 2-edges due to a common neighbor in N2(v),
                                //  and adds v to the solution
                                algo->set(v, pack_status::included);
                        }
                }
        }

        std::cout << "degree_one_reduction_e:" << oldn - status.remaining_nodes << std::endl;
        return oldn != status.remaining_nodes;
}

bool deg_two_2reduction_e::reduce(reduce_algorithm* algo) {
        auto config = algo->config;
        if (config.disable_deg_two) return false;
        auto& status = algo->global_status;
        size_t oldn = status.remaining_nodes;

        // checks if v is completely isolated (no edges and 2-edges incident to v)
        auto is_isolated = [&algo = algo](NodeID& v) { return algo->deg(v) == 0 && algo->two_deg(v) == 0; };
        // check if v has degree 1 and N2[v] == N[neighbor_of_v]
        auto is_deg1_2isolated = [&algo = algo](NodeID& v) {
                return algo->deg(v) == 1 && algo->two_deg(v) + 1 == algo->deg(algo->global_status.graph[v][0]);
        };

        auto is_within_triangle = [&status = status, &algo = algo](NodeID& v) {
                // special case of clique reductions
                return algo->two_deg(v) == 0 && algo->deg(status.graph[v][0]) == 2 && algo->deg(status.graph[v][1]) == 2;
        };

        auto is_within_4_cycle = [&status = status, &algo = algo](NodeID& v) {
                // a 4-cycle check
                // 2-neighborhood of v consists only of some vertex x which is the 2nd neighbor of v's neighbors
                return algo->two_deg(v) == 1 && algo->deg(status.graph[v][0]) == 2 && algo->deg(status.graph[v][1]) == 2;
        };

        auto is_within_V_shape = [&status = status, &algo = algo](NodeID& v) {
                return algo->two_deg(v) == 2 && algo->deg(status.graph[v][0]) == 2 && algo->deg(status.graph[v][1]) == 2;
        };

        for (size_t v_idx = 0; v_idx < marker.current_size(); v_idx++) {
                NodeID v = marker.current_vertex(v_idx);

                if (status.node_status[v] == pack_status::not_set) {
                        if (is_isolated(v) || is_deg1_2isolated(v)) {
                                // this removes N2[v] of G and only leaves 2-edges due to a common neighbor in N2(v),
                                //  and adds v to the solution
                                algo->set(v, pack_status::included);
                        }
                        else if(algo->deg(v) == 2) {
                                if(is_within_triangle(v)){
                                        algo->set(v, pack_status::included);
                                        std::cout <<"applied triangle reduction" << std::endl;
                                }
                                else if(is_within_4_cycle(v)) {
                                        // 4-cycle
                                        algo->set(v, pack_status::included);
                                        std::cout <<"applied v-cycle reduction" << std::endl;
                                }
                                else if(is_within_V_shape(v)) {
                                        algo->set(v, pack_status::included);
                                        std::cout <<"applied V-shape reduction" << std::endl;
                                }
                        }
                }
        }

        std::cout << "deg_two_2reduction_e:" << oldn-status.remaining_nodes << std::endl;
        return oldn != status.remaining_nodes;
}

bool twin2_reduction_e::reduce(reduce_algorithm* algo) {
        auto config = algo->config;
        if (config.disable_twin) return false;
        auto& status = algo->global_status;
        size_t oldn = status.remaining_nodes;
        fast_set set_1(algo->global_status.n);

        for (size_t v_idx = 0; v_idx < marker.current_size(); v_idx++) {
                NodeID v = marker.current_vertex(v_idx);

                if (status.node_status[v] == pack_status::not_set) {
                        if (algo->deg(v) == 2) {
                                NodeID neighbor1 = status.graph[v][0];
                                NodeID neighbor2 = status.graph[v][1];

                                if (status.node_status[neighbor1] == pack_status::not_set &&
                                    status.node_status[neighbor2] == pack_status::not_set) {
                                        if (algo->deg(neighbor1) == 3 && algo->deg(neighbor2) == 3) {
                                                for (NodeID neighbor : status.graph[neighbor1]) {
                                                        set_1.add(neighbor);
                                                }

                                                bool set_false = false;

                                                for (NodeID neighbor : status.graph[neighbor2]) {
                                                        if (!set_1.get(neighbor)) {
                                                                set_false = true;
                                                        }
                                                }

                                                if (!set_false) {
                                                        algo->set(v, pack_status::included);
                                                        continue;
                                                }
                                        }
                                } else {
                                        continue;
                                }
                        }
                }
        }

        /* std::cout << "twin_reduction_e:" << oldn-status.remaining_nodes << std::endl; */
        return oldn != status.remaining_nodes;
}

bool domination2_reduction_e::reduce(reduce_algorithm* algo) {
        auto config = algo->config;
        if (config.disable_domination) return false;
        auto& status = algo->global_status;
        auto& neighbors = algo->set_1;
        size_t oldn = status.remaining_nodes;

        // checks if v is completely isolated (no edges and 2-edges incident to v)
        auto is_isolated = [&algo = algo](NodeID& v) { return algo->deg(v) == 0 && algo->two_deg(v) == 0; };
        // check if v has degree 1 and N2[v] == N[neighbor_of_v]
        auto is_deg1_2isolated = [&algo = algo](NodeID& v) {
                return algo->deg(v) == 1 && algo->two_deg(v) + 1 == algo->deg(algo->global_status.graph[v][0]);
        };

        for (size_t v_idx = 0; v_idx < marker.current_size(); v_idx++) {
                NodeID v = marker.current_vertex(v_idx);

                if (status.node_status[v] == pack_status::not_set) {

                        // TODO: deg-1 reduction is now checked here
                        if (is_isolated(v) || is_deg1_2isolated(v)) {
                                // this removes N2[v] of G and only leaves 2-edges due to a common neighbor in N2(v),
                                //  and adds v to the solution
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

                        // TODO: check deg-1 reduction
                        /*if (neighbors_count <= 1) {
                                algo->set(v, pack_status::included);
                                continue;
                        }*/


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

        std::cout << "domination_reduction_e:" << oldn-status.remaining_nodes << std::endl;
        return oldn != status.remaining_nodes;
}

bool clique2_reduction_e::reduce(reduce_algorithm* algo) {
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
                                if (status.node_status[neighbor] == pack_status::not_set) {
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
                        }
                        if (is_clique) {
                                algo->set(node, pack_status::included);
                        }
                        continue;
                }
        }

        std::cout << "clique_reduction_e:" << oldn - status.remaining_nodes << std::endl;
        return oldn != status.remaining_nodes;
}

}  // namespace two_packing_set