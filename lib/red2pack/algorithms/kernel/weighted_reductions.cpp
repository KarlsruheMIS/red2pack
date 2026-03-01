#include "red2pack/algorithms/kernel/weighted_reductions.h"
#include "red2pack/algorithms/kernel/weighted_reduce_algorithm.h"

#include <random_functions.h>


namespace red2pack {

using pack_status = two_pack_status;

void vertex_marker_w2pack::shuffle() {
	// used from random_functions.h (KaHIP)
	if(current.size() < 4){
		return;
	}
	int distance = 20; 
	unsigned int size = current.size()-4;
	for( unsigned int i = 0; i < size; i++) {
		unsigned int posA = i;
		unsigned int posB = (posA + random_functions::nextInt(0, distance))%size;
		std::swap(current[posA], current[posB]);
		std::swap(current[posA+1], current[posB+1]); 
		std::swap(current[posA+2], current[posB+2]); 
		std::swap(current[posA+3], current[posB+3]); 
	}
}

bool neighborhood_removal_w2pack::reduce(weighted_reduce_algorithm *algo) {
        auto &config = algo->config;
        if (config.disable_neighborhood_removal) return false;
        auto &status = algo->global_status;
        size_t oldn = status.remaining_nodes;

        for (size_t v_idx_start = 0; v_idx_start < marker.current_size(); v_idx_start += batch_size) {
                if (algo->time_limit_reached()) {
                        break;
                }
                for (size_t v_idx = v_idx_start; v_idx < std::min(marker.current_size(), v_idx_start + batch_size);
                     v_idx++) {
                        NodeID v = marker.current_vertex(v_idx);

                        if (status.node_status[v] == pack_status::not_set) {
                                // check and apply reductions with degree <= 1
                                auto weight_v = status.graph.weight(v);

                                // max weight in direct neighborhood
                                NodeWeight max_weight = status.graph.get_max_weight_of_neigh(v);

                                if (max_weight > weight_v) {
                                        continue;
                                }

                                // sum weight in two neighborhood
                                NodeWeight weight_two_neigh = 0;

                                        // compute with early exit
                                        for (auto u : status.graph.neigh2(v)) {
                                                if (weight_v > status.graph.weight(u)) {
                                                        // u cannot be removed by neighborhood removal unless its
                                                        // neighborhood changes
                                                        // blacklist.add(u);
                                                        //++blacklisted;
                                                }
                                                weight_two_neigh += status.graph.weight(u);
                                                if (weight_v < max_weight + weight_two_neigh) {
                                                        break;
                                                }
                                        }

                                if (weight_v >= max_weight + weight_two_neigh) {
                                        algo->set(v, pack_status::included);
                                        continue;
                                }
                        }
                }
        }

        std::cout << "neighborhood removal:" << oldn - status.remaining_nodes << " " << status.remaining_nodes
                  << std::endl;
        return oldn != status.remaining_nodes;
}

bool direct_neighbor_removal_w2pack::reduce(weighted_reduce_algorithm *algo) {
        auto &config = algo->config;
        if (config.disable_direct_neighbor_removal) return false;
        auto &status = algo->global_status;
        size_t oldn = status.remaining_nodes;

        for (size_t v_idx_start = 0; v_idx_start < marker.current_size(); v_idx_start += batch_size) {
                if (algo->time_limit_reached()) {
                        break;
                }
                for (size_t v_idx = v_idx_start; v_idx < std::min(marker.current_size(), v_idx_start + batch_size);
                     v_idx++) {
                        NodeID v = marker.current_vertex(v_idx);

                        if (status.node_status[v] == pack_status::not_set) {
                                // check and apply reductions with degree <= 1
                                auto weight_v = status.graph.weight(v);

                                auto &two_neigh_v_set = algo->set_1;
                                auto &feasible_one_neigh_v = algo->buffers[0];
                                feasible_one_neigh_v.clear();
                                two_neigh_v_set.clear();

                                for (auto u : status.graph[v]) {
                                        if (status.graph.weight(u) <= weight_v) {
                                                feasible_one_neigh_v.push_back(u);
                                        }
                                }

                                if (feasible_one_neigh_v.empty()) {
                                        continue;
                                }

                                NodeWeight sum_weight_two_neigh_v = status.get_weight_two_neigh(v);

                                if (status.graph.get_max_weight_of_neigh(v) + sum_weight_two_neigh_v <= weight_v) {
                                        algo->set(v, pack_status::included);
                                        continue;
                                }

                                for (auto &w : status.graph.neigh2(v)) {
                                        two_neigh_v_set.add(static_cast<int>(w));
                                        // sum_weight_two_neigh_v += status.graph.weight(w);
                                }

                                for (auto u : feasible_one_neigh_v) {
                                        reduce_direct_neighbor(algo, v, u, sum_weight_two_neigh_v);
                                        if (status.graph.get_max_weight_of_neigh(v) + sum_weight_two_neigh_v <=
                                            weight_v) {
                                                algo->set(v, pack_status::included);
                                                break;
                                        }
                                }

                        }
                }
        }

        std::cout << "direct neighbor removal:" << oldn - status.remaining_nodes << " " << status.remaining_nodes
                  << std::endl;
        return oldn != status.remaining_nodes;
}

bool two_neighbor_removal_w2pack::reduce(weighted_reduce_algorithm *algo) {
        auto &config = algo->config;
        if (config.disable_two_neighbor_removal) return false;
        auto &status = algo->global_status;
        size_t oldn = status.remaining_nodes;

        for (size_t v_idx_start = 0; v_idx_start < marker.current_size(); v_idx_start += batch_size) {
                if (algo->time_limit_reached()) {
                        break;
                }
                for (size_t v_idx = v_idx_start; v_idx < std::min(marker.current_size(), v_idx_start + batch_size);
                     v_idx++) {
                        NodeID v = marker.current_vertex(v_idx);

                        if (status.node_status[v] == pack_status::not_set) {
                                NodeWeight weight_v = status.graph.weight(v);

                                auto &two_neigh_v_set = algo->set_1;
                                auto &one_neigh_v_set = algo->set_2;
                                auto &feasible_two_neigh_v = algo->buffers[1];
                                feasible_two_neigh_v.clear();

                                two_neigh_v_set.clear();
                                one_neigh_v_set.clear();

                                NodeWeight sum_weight_two_neigh_v = status.get_weight_two_neigh(v);

                                if (status.graph.get_max_weight_of_neigh(v) + sum_weight_two_neigh_v <= weight_v) {
                                        algo->set(v, pack_status::included);
                                        continue;
                                }

                                for (auto &w : status.graph.neigh2(v)) {
                                        two_neigh_v_set.add(static_cast<int>(w));
                                        if (status.graph.weight(w) <= weight_v) {
                                                feasible_two_neigh_v.push_back(w);
                                        }
                                        // sum_weight_two_neigh_v += status.graph.weight(w);
                                }

                                if (feasible_two_neigh_v.empty()) {
                                        continue;
                                }

                                for (auto &w : status.graph[v]) {
                                        one_neigh_v_set.add(static_cast<int>(w));
                                }

                                for (NodeID &u : feasible_two_neigh_v) {
                                        if (reduce_two_neighbor(algo, v, u, sum_weight_two_neigh_v)) {
                                                two_neigh_v_set.remove(static_cast<int>(u));
                                                sum_weight_two_neigh_v -= status.graph.weight(u);
                                                // status.graph.init_two_neighbors[v] = true;
                                                // status.graph.hide_two_edge(v, u);

                                                if (status.graph.get_max_weight_of_neigh(v) + sum_weight_two_neigh_v <=
                                                    weight_v) {
                                                        algo->set(v, pack_status::included);
                                                        break;
                                                }
                                        }
                                }

                        }
                }
        }

        std::cout << "two neighbor removal:" << oldn - status.remaining_nodes << " " << status.remaining_nodes
                  << std::endl;
        return oldn != status.remaining_nodes;
}

bool split_neighbor_removal_w2pack::reduce(weighted_reduce_algorithm *algo) {
        auto &config = algo->config;
        if (config.disable_split_neighbor_removal) return false;
        auto &status = algo->global_status;
        size_t oldn = status.remaining_nodes;

        for (size_t v_idx_start = 0; v_idx_start < marker.current_size(); v_idx_start += batch_size) {
                if (algo->time_limit_reached()) {
                        break;
                }
                for (size_t v_idx = v_idx_start; v_idx < std::min(marker.current_size(), v_idx_start + batch_size);
                     v_idx++) {
                        NodeID v = marker.current_vertex(v_idx);

                        if (status.node_status[v] == pack_status::not_set) {
                                NodeWeight weight_v = status.graph.weight(v);
                                NodeWeight sum_weight_two_neigh_v = 0;

                                auto &two_neigh_v_set = algo->set_1;
                                auto &one_neigh_v_set = algo->set_2;

                                auto &feasible_one_neigh_v = algo->buffers[0];
                                auto &feasible_two_neigh_v = algo->buffers[1];
                                feasible_one_neigh_v.clear();
                                feasible_two_neigh_v.clear();

                                two_neigh_v_set.clear();
                                one_neigh_v_set.clear();

                                for (auto &w : status.graph[v]) {
                                        one_neigh_v_set.add(static_cast<int>(w));
                                        if (weight_v >= status.graph.weight(w)) {
                                                feasible_one_neigh_v.push_back(w);
                                        }
                                }
                                for (auto &w : status.graph.neigh2(v)) {
                                        two_neigh_v_set.add(static_cast<int>(w));
                                        if (weight_v >= status.graph.weight(w)) {
                                                feasible_two_neigh_v.push_back(w);
                                        }
                                        sum_weight_two_neigh_v += status.graph.weight(w);
                                }

                                if (status.graph.get_max_weight_of_neigh(v) + sum_weight_two_neigh_v <= weight_v) {
                                        algo->set(v, pack_status::included);
                                        continue;
                                }

                                for (NodeID &u : feasible_one_neigh_v) {
                                        if (one_neigh_v_set.get(static_cast<int>(u)) &&
                                            reduce_direct_neighbor(algo, v, u, sum_weight_two_neigh_v)) {
                                                one_neigh_v_set.remove(static_cast<int>(u));
                                                if (status.graph.get_max_weight_of_neigh(v) + sum_weight_two_neigh_v <=
                                                    weight_v) {
                                                        break;
                                                }
                                        }
                                }

                                if (status.graph.get_max_weight_of_neigh(v) + sum_weight_two_neigh_v <= weight_v) {
                                        algo->set(v, pack_status::included);
                                        break;
                                }

                                for (NodeID &u : feasible_two_neigh_v) {
                                        if (reduce_two_neighbor(algo, v, u, sum_weight_two_neigh_v)) {
                                                two_neigh_v_set.remove(static_cast<int>(u));
                                                sum_weight_two_neigh_v -= status.graph.weight(u);
                                                if (status.graph.get_max_weight_of_neigh(v) + sum_weight_two_neigh_v <=
                                                    weight_v) {
                                                        break;
                                                }
                                        }
                                }

                                if (status.graph.get_max_weight_of_neigh(v) + sum_weight_two_neigh_v <= weight_v) {
                                        algo->set(v, pack_status::included);
                                        break;
                                }
                        }
                }
        }

        std::cout << "split neighbor removal:" << oldn - status.remaining_nodes << " " << status.remaining_nodes
                  << std::endl;
        return oldn != status.remaining_nodes;
}

bool split_neighbor_removal_w2pack::reduce_two_neighbor(weighted_reduce_algorithm *algo, NodeID v, NodeID u,
                                                        NodeWeight sum_weight_two_neigh_v) {
        auto &status = algo->global_status;

        // assume the neighborhoods of v were set
        const auto &two_neigh_v_set = algo->set_1;
        const auto &one_neigh_v_set = algo->set_2;

        const auto weight_v = status.graph.weight(v);
        const auto weight_u = status.graph.weight(u);

        auto max_weight = status.graph.get_max_weight_of_neigh(v);
        auto sum_weight_neigh_v = status.graph.get_sum_weight_of_neigh(v);

        if (weight_u > weight_v) {
                return false;
        }

        // determine upper bound U
        NodeWeight upper_bound_1 = sum_weight_neigh_v + sum_weight_two_neigh_v;
        NodeWeight upper_bound_2 = max_weight + sum_weight_two_neigh_v;

        // upper_bound == sum weight two neighborhood of v
        if (upper_bound_1 <= weight_v || upper_bound_2 <= weight_v) {
                // remove u
                algo->set(u, pack_status::excluded);
                return true;
        }

        // search for stronger upper bound
        for (auto &w : status.graph[u]) {
                if (two_neigh_v_set.get(static_cast<int>(w))) {
                        upper_bound_1 -= status.graph.weight(w);
                        upper_bound_2 -= status.graph.weight(w);
                        if (upper_bound_1 <= weight_v || upper_bound_2 <= weight_v) {
                                // remove u
                                algo->set(u, pack_status::excluded);
                                return true;
                        }
                } else if (one_neigh_v_set.get(static_cast<int>(w))) {
                        upper_bound_1 -= status.graph.weight(w);
                        if (upper_bound_1 <= weight_v) {
                                // remove u
                                algo->set(u, pack_status::excluded);
                                return true;
                        }
                }
        }

        // search for stronger upper bound
        for (auto &w : status.graph.neigh2(u)) {
                if (two_neigh_v_set.get(static_cast<int>(w))) {
                        upper_bound_1 -= status.graph.weight(w);
                        upper_bound_2 -= status.graph.weight(w);

                        if (upper_bound_1 <= weight_v || upper_bound_2 <= weight_v) {
                                // remove u
                                algo->set(u, pack_status::excluded);
                                return true;
                        }
                } else if (one_neigh_v_set.get(static_cast<int>(w))) {
                        upper_bound_1 -= status.graph.weight(w);

                        if (upper_bound_1 <= weight_v) {
                                // remove u
                                algo->set(u, pack_status::excluded);
                                return true;
                        }
                }
        }

        return false;
}

bool split_neighbor_removal_w2pack::reduce_direct_neighbor(weighted_reduce_algorithm *algo, NodeID v, NodeID u,
                                                           NodeWeight sum_weight_two_neigh_v) {
        auto &status = algo->global_status;

        // assume the two neighborhood of v was added
        auto &two_neigh_v_set = algo->set_1;

        auto weight_v = status.graph.weight(v);
        auto weight_u = status.graph.weight(u);

        if (weight_u > weight_v) {
                return false;
        }
        const auto delta = weight_v - weight_u;

        // determine upper bound U
        NodeWeight upper_bound = sum_weight_two_neigh_v;

        if (upper_bound <= delta ||
            upper_bound + status.graph.get_sum_weight_of_neigh(v) <= status.graph.get_sum_weight_of_neigh(u)) {
                // remove u
                algo->set(u, pack_status::excluded);
                return true;
        }

        // search for stronger upper bound
        for (auto &w : status.graph[u]) {
                // ASSERT_TRUE(two_neigh_v_set.get(static_cast<int>(w)) == (w != v));
                if (two_neigh_v_set.get(static_cast<int>(w))) {
                        upper_bound -= status.graph.weight(w);
                        if (upper_bound <= delta) {
                                // remove u
                                algo->set(u, pack_status::excluded);
                                return true;
                        }
                }
        }

        // search for stronger upper bound
        for (auto &w : status.graph.neigh2(u)) {
                if (two_neigh_v_set.get(static_cast<int>(w))) {
                        upper_bound -= status.graph.weight(w);

                        if (upper_bound <= delta) {
                                // remove u
                                algo->set(u, pack_status::excluded);
                                return true;
                        }
                }
        }

        return false;
}

bool single_fast_domination_w2pack::reduce(weighted_reduce_algorithm *algo) {
        auto &config = algo->config;
        if (config.disable_single_fast_domination) return false;
        auto &status = algo->global_status;
        size_t oldn = status.remaining_nodes;

        for (size_t v_idx_start = 0; v_idx_start < marker.current_size(); v_idx_start += batch_size) {
                if (algo->time_limit_reached()) {
                        break;
                }
                for (size_t v_idx = v_idx_start; v_idx < std::min(marker.current_size(), v_idx_start + batch_size);
                     v_idx++) {
                        NodeID v = marker.current_vertex(v_idx);

                        if (status.node_status[v] == pack_status::not_set) {
                                NodeWeight weight_v = status.graph.weight(v);
                                auto &one_neigh_v_set = algo->set_2;
                                auto &feasible_one_neigh_v = algo->buffers[0];
                                feasible_one_neigh_v.clear();

                                for (auto u : status.graph[v]) {
                                        one_neigh_v_set.add(static_cast<int>(u));
                                        if (status.graph.weight(u) <= weight_v) {
                                                feasible_one_neigh_v.push_back(u);
                                        }
                                }

                                for (auto u : feasible_one_neigh_v) {
                                        if (status.graph.deg(v) > status.graph.deg(u)) {
                                                continue;
                                        }

                                        if (!status.graph.init_link_neighbors[v]) {
                                                // subset test of direct neighborhood before using 2-neighborhood
                                                // is N[v]\subseteq N[u] ?
                                                // assume neighborhood was set
                                                NodeID subset_counter = 1;  // account one for direct neighbor u and v
                                                for (auto &w : status.graph[u]) {
                                                        if (one_neigh_v_set.get(static_cast<int>(w))) {
                                                                subset_counter++;
                                                        }
                                                }
                                                if (subset_counter != status.graph.deg(v)) {
                                                        continue;
                                                }
                                        }

                                        if (status.graph.deg2(v) + status.graph.deg(v) == status.graph.deg(u)) {
                                                algo->set(u, pack_status::excluded);
                                                one_neigh_v_set.remove(static_cast<int>(u));
                                                continue;
                                        }
                                }
                        }
                }
        }

        std::cout << "single fast domination:" << oldn - status.remaining_nodes << " " << status.remaining_nodes
                  << std::endl;
        return oldn != status.remaining_nodes;
}
bool split_intersection_removal_w2pack::reduce(weighted_reduce_algorithm *algo) {
        auto &config = algo->config;
        if (config.disable_split_intersection_removal) return false;
        auto &status = algo->global_status;
        size_t oldn = status.remaining_nodes;

        for (size_t v_idx_start = 0; v_idx_start < marker.current_size(); v_idx_start += batch_size) {
                if (algo->time_limit_reached()) {
                        break;
                }
                for (size_t v_idx = v_idx_start; v_idx < std::min(marker.current_size(), v_idx_start + batch_size);
                     v_idx++) {
                        NodeID v = marker.current_vertex(v_idx);

                        if (status.node_status[v] == pack_status::not_set) {
                                auto weight_v = status.graph.weight(v);
                                NodeWeight sum_weight_two_neigh_v = status.get_weight_two_neigh(v);
                                NodeWeight sum_weight_one_neigh_v = status.graph.get_sum_weight_of_neigh(v);
                                NodeWeight max_weight_v = status.graph.get_max_weight_of_neigh(v);

                                NodeWeight max_weight_v_without_max_weight =
                                    0;  // max weight in direct neighborhood of v without a vertex of max weight
                                bool max_weight_v_without_max_weight_set = false;

                                // check for neighborhood removal
                                if (weight_v >= sum_weight_two_neigh_v + max_weight_v) {
                                        algo->set(v, pack_status::included);
                                        continue;
                                }

                                // consider all direct neighbors of v
                                for (auto u : status.graph[v]) {
                                        NodeWeight max_weight_v_without_u =
                                            0;  // max weight in direct neighborhood of v without u
                                        if (max_weight_v > status.graph.weight(u)) {
                                                // u has not max weight, therefore we can use max weight of v
                                                max_weight_v_without_u = max_weight_v;
                                        } else if (max_weight_v_without_max_weight_set) {
                                                // u has max weight, but we already know the max weight in the direct
                                                // neighborhood without a max weight vertex
                                                max_weight_v_without_u = max_weight_v_without_max_weight;
                                        } else {
                                                // u has max weight; what is the max weight in the direct neighborhood
                                                // of u if a max weight vertex is not contained
                                                max_weight_v_without_max_weight = 0;
                                                max_weight_v_without_max_weight_set = true;

                                                for (auto &w : status.graph[v]) {
                                                        if (w != u) {
                                                                if (max_weight_v_without_max_weight <
                                                                    status.graph.weight(w)) {
                                                                        max_weight_v_without_max_weight =
                                                                            status.graph.weight(w);
                                                                }
                                                        }
                                                }
                                                max_weight_v_without_u = max_weight_v_without_max_weight;
                                        }

                                        if (weight_v >= sum_weight_two_neigh_v + max_weight_v_without_u) {
                                                auto [reduced, intersection_weight] =
                                                    reduce_intersection_of_direct_neighbors(algo, v, u);

                                                if (reduced) {
                                                        ASSERT_TRUE(status.graph.get_max_weight_of_neigh(v) ==
                                                                    status.graph.weight(u));
                                                        ASSERT_TRUE(status.graph.get_sum_weight_of_neigh(v) ==
                                                                    status.graph.weight(u));
                                                        sum_weight_two_neigh_v =
                                                            sum_weight_two_neigh_v + sum_weight_one_neigh_v -
                                                            intersection_weight - status.graph.weight(u);
#ifndef NDEBUG
                                                        // check if sum_weight_two_neigh_v was computed correctly
                                                        NodeWeight check = 0;
                                                        for (auto x : status.graph.neigh2(v)) {
                                                                check += status.graph.weight(x);
                                                        }
                                                        ASSERT_TRUE(check == sum_weight_two_neigh_v);
#endif
                                                        max_weight_v =
                                                            status.graph.get_max_weight_of_neigh(v);  // weight of u
                                                        sum_weight_one_neigh_v = status.graph.get_sum_weight_of_neigh(
                                                            v);  // equals to max weight
                                                        break;
                                                }
                                        }
                                }

                                // check for neighborhood removal
                                if (weight_v >= sum_weight_two_neigh_v + max_weight_v) {
                                        algo->set(v, pack_status::included);
                                        continue;
                                }

                                // prepare copy of 2-neighborhood
                                auto &two_neigh_v = algo->buffers[0];
                                two_neigh_v.clear();

                                for (auto u : status.graph.neigh2(v)) {
                                        two_neigh_v.push_back(u);
                                }

                                // consider all two neighbors of v
                                for (auto u : two_neigh_v) {
                                        ASSERT_TRUE(sum_weight_one_neigh_v == status.graph.get_sum_weight_of_neigh(v));
                                        if (status.node_status[u] == two_pack_status::not_set &&
                                            weight_v >=
                                                sum_weight_two_neigh_v - status.graph.weight(u) + max_weight_v) {
                                                auto [reduced, intersection_weight] =
                                                    reduce_intersection_of_two_neighbors(algo, v, u);
                                                if (reduced) {
                                                        // use old sum_weight_one_neigh_v and intersection_weight to
                                                        // compute new sum_weight_two_neigh_v
                                                        sum_weight_two_neigh_v =
                                                            sum_weight_two_neigh_v + sum_weight_one_neigh_v -
                                                            status.graph.get_sum_weight_of_neigh(v) -
                                                            intersection_weight;
#ifndef NDEBUG
                                                        // check if sum_weight_two_neigh_v was computed correctly
                                                        NodeWeight check = 0;
                                                        for (auto x : status.graph.neigh2(v)) {
                                                                check += status.graph.weight(x);
                                                        }
                                                        ASSERT_TRUE(check == sum_weight_two_neigh_v);
#endif
                                                        // update sum_weight_one_neigh_v
                                                        sum_weight_one_neigh_v =
                                                            status.graph.get_sum_weight_of_neigh(v);
                                                        // update sum_weight_one_neigh_v
                                                        max_weight_v = status.graph.get_max_weight_of_neigh(v);

                                                        if (weight_v >=
                                                            sum_weight_two_neigh_v + sum_weight_one_neigh_v) {
                                                                algo->set(v, pack_status::included);
                                                                break;
                                                        }
                                                }
                                        }
                                }

                        }
                }
        }

        std::cout << "split intersection:" << oldn - status.remaining_nodes << " " << status.remaining_nodes
                  << std::endl;
        return oldn != status.remaining_nodes;
}

std::pair<bool, NodeWeight> split_intersection_removal_w2pack::reduce_intersection_of_direct_neighbors(
    weighted_reduce_algorithm *algo, NodeID v, NodeID u) {
        auto &status = algo->global_status;
        NodeWeight intersection_weight = 0;

        // intersection
        auto &neigh_v = algo->set_1;
        neigh_v.clear();

        auto &intersection = algo->buffers[1];
        intersection.clear();

        for (auto x : status.graph[v]) {
                neigh_v.add(static_cast<int>(x));
        }

        for (auto x : status.graph.neigh2(v)) {
                neigh_v.add(static_cast<int>(x));
        }

        for (auto x : status.graph[u]) {
                if (x != v) {
                        intersection.push_back(x);
                }
        }

        for (auto x : status.graph.neigh2(u)) {
                if (neigh_v.get(static_cast<int>(x))) {
                        intersection.push_back(x);
                }
        }

        for (auto x : intersection) {
                intersection_weight += status.graph.weight(x);
                algo->set(x, pack_status::excluded);
        }

        return {!intersection.empty(), intersection_weight};
}
std::pair<bool, NodeWeight> split_intersection_removal_w2pack::reduce_intersection_of_two_neighbors(
    weighted_reduce_algorithm *algo, NodeID v, NodeID u) {
        auto &status = algo->global_status;
        NodeWeight intersection_weight = 0;

        // intersection
        auto &neigh_v = algo->set_1;
        neigh_v.clear();

        auto &intersection = algo->buffers[1];
        intersection.clear();

        for (auto x : status.graph[v]) {
                neigh_v.add(static_cast<int>(x));
        }

        for (auto x : status.graph.neigh2(v)) {
                neigh_v.add(static_cast<int>(x));
        }

        for (auto x : status.graph[u]) {
                if (neigh_v.get(static_cast<int>(x))) {
                        intersection.push_back(x);
                }
        }

        for (auto x : status.graph.neigh2(u)) {
                if (neigh_v.get(static_cast<int>(x))) {
                        intersection.push_back(x);
                }
        }

        for (auto x : intersection) {
                intersection_weight += status.graph.weight(x);
                algo->set(x, pack_status::excluded);
        }

        return {!intersection.empty(), intersection_weight};
}
bool domination_w2pack::reduce(weighted_reduce_algorithm *algo) {
        auto config = algo->config;
        if (config.disable_wdomination) return false;
        auto &status = algo->global_status;
        size_t oldn = status.remaining_nodes;

        for (size_t v_idx_start = 0; v_idx_start < marker.current_size(); v_idx_start += batch_size) {
                if (algo->time_limit_reached()) {
                        break;
                }
                for (size_t v_idx = v_idx_start; v_idx < std::min(marker.current_size(), v_idx_start + batch_size);
                     v_idx++) {
                        NodeID v = marker.current_vertex(v_idx);

                        if (status.node_status[v] == pack_status::not_set) {
                                auto weight_v = status.graph.weight(v);

                                for (auto u : status.graph[v]) {
                                        // check whether closed 2-neighborhood of v is a subset of u

                                        // early exit
                                        if (status.graph.deg(v) <= status.graph.deg(u)) {
                                                if (status.graph.deg(v) + status.graph.deg2(v) == status.graph.deg(u)) {
                                                        // it holds N_2[v]=N[u]
                                                        // reduce

                                                        if (weight_v >=
                                                            std::max(status.graph.get_max_weight_of_neigh(u),
                                                                     status.graph.weight(u))) {
                                                                algo->set(v, pack_status::included);
                                                                break;
                                                        }
                                                        // does u fulfill the weight inequality with v?
                                                        if (2 * weight_v >= status.graph.get_sum_weight_of_neigh(u)) {
                                                                auto &one_neigh_u = algo->buffers[1];
                                                                one_neigh_u.clear();
                                                                for (auto x : status.graph[u]) {
                                                                        if (x != v) {
                                                                                one_neigh_u.push_back(x);
                                                                        }
                                                                }
                                                                for (auto &x : one_neigh_u) {
                                                                        algo->set(x, pack_status::excluded);
                                                                }
                                                                break;  // we reduced the direct neighborhood of u and
                                                                        // hence, the neighborhood of v except of u
                                                        }
                                                }
                                        }
                                }
                        }
                }
        }

        std::cout << "domination:" << oldn - status.remaining_nodes << " " << status.remaining_nodes << std::endl;
        return oldn != status.remaining_nodes;
}


bool weight_transfer_w2pack::reduce(weighted_reduce_algorithm *algo) {
        auto &config = algo->config;
        if (config.disable_weight_transfer) return false;
        auto &status = algo->global_status;
        size_t oldn = status.remaining_nodes;

        auto &blacklist = algo->set_3;
        blacklist.clear();

        for (size_t v_idx_start = 0; v_idx_start < marker.current_size(); v_idx_start += batch_size) {
                if (algo->time_limit_reached()) {
                        break;
                }
                for (size_t v_idx = v_idx_start; v_idx < std::min(marker.current_size(), v_idx_start + batch_size);
                     v_idx++) {
                        NodeID v = marker.current_vertex(v_idx);

                        if (status.node_status[v] == pack_status::not_set && !blacklist.get(v)) {
                                auto weight_v = status.graph.weight(v);

                                bool two_clique = true;
                                NodeID neigh_size = status.graph.deg(v) + status.graph.deg2(v);

                                for (auto u : status.graph[v]) {
                                        // if degree equality holds, we know that v is 2-simplicial
                                        if (neigh_size == status.graph.deg(u) &&
                                            weight_v >= std::max(status.graph.get_max_weight_of_neigh(u),
                                                                 status.graph.weight(u))) {
                                                algo->set(v, pack_status::included);
                                                // set two clique to false to continue with another v
                                                two_clique = false;

                                                break;
                                        }
                                        // early exit
                                        if (status.graph.deg(u) + status.graph.deg2(u) < neigh_size) {
                                                two_clique = false;
                                                break;
                                        } else if (status.graph.deg(u) + status.graph.deg2(u) >= neigh_size) {
                                                // if u is 2-simplicial it will be handled right away in the following
                                                // if u has larger overall degree, it cannot be simplicial
                                                // as it will be adjacent to a lower degree vertex v;
                                                // hence, we do not need to consider it again in this scan over all
                                                // marked vertices
                                                blacklist.add(u);
                                        }
                                }
                                if (!two_clique) {
                                        continue;
                                }
                                NodeWeight weight_two_neigh = 0;
                                for (auto u : status.graph.neigh2(v)) {
                                        weight_two_neigh += status.graph.weight(u);
                                        if (status.graph.deg(u) + status.graph.deg2(u) < neigh_size) {
                                                two_clique = false;
                                                break;
                                        } else if (status.graph.deg(u) + status.graph.deg2(u) >= neigh_size) {
                                                blacklist.add(u);
                                        }
                                }
                                if (!two_clique) {
                                        continue;
                                }
                                if (status.graph.get_max_weight_of_neigh(v) + weight_two_neigh <= weight_v) {
                                        algo->set(v, pack_status::included);
                                        continue;
                                }

                                auto &one_neigh_v_set = algo->set_1;
                                auto &closed_two_neigh_v_set = algo->set_2;
                                auto &closed_two_neigh_v_without_v = algo->buffers[0];

                                auto &simplicial_nodes = algo->buffers[1];
                                std::vector<NodeID> non_simplicial;
                                size_t max_simplicial_idx = 0;
                                weighted_node max_simplicial{v, weight_v};
                                weighted_node max_non_simplicial{0, 0};

                                simplicial_nodes.clear();
                                simplicial_nodes.push_back(v);

                                one_neigh_v_set.clear();
                                closed_two_neigh_v_without_v.clear();
                                closed_two_neigh_v_set.clear();

                                closed_two_neigh_v_set.add(static_cast<int>(v));
                                for (auto u : status.graph[v]) {
                                        closed_two_neigh_v_set.add(static_cast<int>(u));
                                        one_neigh_v_set.add(static_cast<int>(u));
                                        closed_two_neigh_v_without_v.push_back(u);
                                }
                                for (auto u : status.graph.neigh2(v)) {
                                        closed_two_neigh_v_set.add(static_cast<int>(u));
                                        closed_two_neigh_v_without_v.push_back(u);
                                }

                                for (auto &x : closed_two_neigh_v_without_v) {
                                        NodeID count_clique_members = 0;

                                        // if x is a direct neighbor of v, all direct neighbors of x, except for v, are
                                        // neighbors of v
                                        if (one_neigh_v_set.get(static_cast<int>(x))) {
                                                count_clique_members += status.graph.deg(x);
                                        } else {
                                                for (auto &w : status.graph[x]) {
                                                        if (closed_two_neigh_v_set.get(static_cast<int>(w))) {
                                                                ++count_clique_members;
                                                        }
                                                }
                                        }

                                        // early exit?
                                        // does x has enough two-neighbors to cover all neighbors of v and v (potential
                                        // clique members)
                                        if (status.graph.deg2(x) <
                                            closed_two_neigh_v_without_v.size() - count_clique_members) {
                                                two_clique = false;
                                                break;
                                        }

                                        for (auto &w : status.graph.neigh2(x)) {
                                                if (closed_two_neigh_v_set.get(static_cast<int>(w))) {
                                                        ++count_clique_members;
                                                        if (count_clique_members ==
                                                            closed_two_neigh_v_without_v.size()) {
                                                                break;
                                                        }
                                                }
                                        }

                                        if (count_clique_members != closed_two_neigh_v_without_v.size()) {
                                                // no 2-clique
                                                two_clique = false;
                                                break;
                                        }

                                        if (closed_two_neigh_v_without_v.size() ==
                                            status.graph.deg(x) + status.graph.deg2(x)) {
                                                // 2-simplicial
                                                simplicial_nodes.push_back(x);
                                                if (status.graph.weight(x) > max_simplicial.weight) {
                                                        max_simplicial = {x, status.graph.weight(x)};
                                                        max_simplicial_idx = simplicial_nodes.size() - 1;
                                                }
                                        } else {
                                                non_simplicial.push_back(x);
                                                if (status.graph.weight(x) > max_non_simplicial.weight) {
                                                        max_non_simplicial = {x, status.graph.weight(x)};
                                                }
                                        }
                                }

                                if (!two_clique) {
                                        continue;
                                }

                                if (max_simplicial.weight >= max_non_simplicial.weight) {
                                        // add v for bulk hide
                                        closed_two_neigh_v_without_v.push_back(v);
                                        algo->set_two_simplicial_included(
                                            max_simplicial.node, closed_two_neigh_v_without_v, closed_two_neigh_v_set);
                                        continue;
                                }

                                // remove simplicial node with maximum weight from simplicial_nodes
                                simplicial_nodes[max_simplicial_idx] = simplicial_nodes.back();
                                simplicial_nodes.pop_back();

                                auto &exclude_set = algo->set_3;
                                exclude_set.clear();

                                for (auto x : simplicial_nodes) {
                                        exclude_set.add(static_cast<int>(x));
                                }

                                for (size_t i = 0; i < non_simplicial.size(); i++) {
                                        NodeID neighbor = non_simplicial[i];
                                        if (status.graph.weight(neighbor) <= max_simplicial.weight) {
                                                simplicial_nodes.push_back(neighbor);
                                                exclude_set.add(static_cast<int>(neighbor));
                                                non_simplicial[i] = non_simplicial.back();
                                                non_simplicial.pop_back();
                                                i--;
                                        }
                                }

                                algo->bulk_exclude(simplicial_nodes, exclude_set);

                                fold(algo, std::move(max_simplicial), std::move(non_simplicial));
                        }
                }
        }

        std::cout << "weight transfer:" << oldn - status.remaining_nodes << " " << status.remaining_nodes << std::endl;
        return oldn != status.remaining_nodes;
}
void weight_transfer_w2pack::fold(weighted_reduce_algorithm *algo,
                                  const weight_transfer_w2pack::weighted_node &simplicial,
                                  std::vector<NodeID> &&non_simplicial_nodes) {
        auto &status = algo->global_status;

        algo->set(simplicial.node, pack_status::folded);
        status.reduction_offset += simplicial.weight;

        for (auto node : non_simplicial_nodes) {
                status.graph.set_weight(node, status.graph.weight(node) - simplicial.weight);
                algo->add_next_level_node(node);
                algo->add_next_level_neighborhood(node);
                algo->add_next_level_two_neighborhood(node);
        }

        status.folded_queue.push_back(get_reduction_type());

        restore_vec.emplace_back(simplicial, std::move(non_simplicial_nodes));
}
void weight_transfer_w2pack::apply(weighted_reduce_algorithm *algo, NodeID node) {
        auto &status = algo->global_status;
        auto simplicial = restore_vec.back().simplicial.node;

        bool set_simplicial = true;

        for (auto target : restore_vec.back().non_simplicial) {
                if (status.node_status[target] == pack_status::included) {
                        set_simplicial = false;
                        break;
                }
        }

        status.sol_weight += restore_vec.back().simplicial.weight;

        restore(algo, node);

        if (set_simplicial) {
                status.node_status[simplicial] = pack_status::included;
        } else {
                status.node_status[simplicial] = pack_status::excluded;
        }
}
void weight_transfer_w2pack::restore(weighted_reduce_algorithm *algo, NodeID node) {
        auto &status = algo->global_status;
        auto &data = restore_vec.back();

        algo->unset(data.simplicial.node);
        status.reduction_offset -= data.simplicial.weight;

        for (auto target : data.non_simplicial) {
                status.graph.set_weight(target, status.graph.weight(target) + data.simplicial.weight);
        }

        restore_vec.pop_back();
}
bool fast_neighborhood_removal_w2pack::reduce(weighted_reduce_algorithm *algo) {
        auto &config = algo->config;
        if (config.disable_fast_neighborhood_removal) return false;

        // we only run fast neighborhood removal once
        config.disable_fast_neighborhood_removal = true;

        auto &status = algo->global_status;
        size_t oldn = status.remaining_nodes;

        auto &sum_weights = algo->weights_neigh;

        for (NodeID v_idx = 0; v_idx < marker.current_size(); v_idx++) {
                NodeID v = marker.current_vertex(v_idx);
                if (status.node_status[v] == pack_status::not_set) {
                        // N_2[v] is a 2-clique because every 2-neighbor of v is a direct neighbor of some of
                        // direct neighbor of v

                        auto weight_v = status.graph.weight(v);

                        // max weight in direct neighborhood
                        NodeWeight max_weight = status.graph.get_max_weight_of_neigh(v);

                        if (max_weight > weight_v) {
                                continue;
                        }

                        // ub for weight in two neigh
                        NodeWeight sum_two_neigh_ub = sum_weights[v];

                        if (weight_v >= max_weight + sum_two_neigh_ub) {
                                algo->set(v, pack_status::included, true, false, false);
                                continue;
                        }
                }
        }

        std::cout << "fast neighborhood removal:" << oldn - status.remaining_nodes << " " << status.remaining_nodes
                  << std::endl;
        return oldn != status.remaining_nodes;
}

bool fast_degree_two_removal_w2pack::reduce(weighted_reduce_algorithm *algo) {
        auto &config = algo->config;
        if (config.disable_fast_degree_two_removal) return false;

        // we only run fast neighborhood removal once
        config.disable_fast_degree_two_removal = true;

        auto &status = algo->global_status;
        size_t oldn = status.remaining_nodes;

        for (NodeID v_idx = 0; v_idx < marker.current_size(); v_idx++) {
                NodeID v = marker.current_vertex(v_idx);
                if (status.node_status[v] == pack_status::not_set && status.graph[v].capacity() == 2) {
                        auto weight_v = status.graph.weight(v);
                        auto u = status.graph[v][0];
                        auto w = status.graph[v][1];

                        if (status.graph.deg(u) > status.graph.deg(w)) {
                                std::swap(u, w);
                        }

                        auto &deg_two_twins = algo->buffers[1];
                        deg_two_twins.clear();

                        // deg_one_simplicials.push_back(v); we push back later
                        NodeID max_twin = v;
                        NodeWeight max_twin_weight = weight_v;
                        NodeWeight max_weight_u = 0;
                        NodeWeight max_weight_w = 0;

                        deg_two_twins.push_back(v);
                        bool u_w_adj = false;
                        for (auto x : status.graph[u]) {
                                if (status.node_status[x] == pack_status::not_set && x != v) {
                                        if (status.graph[x].capacity() == 2 &&
                                            (status.graph[x][0] == w || status.graph[x][1] == w)) {
                                                if (status.graph.weight(x) > max_twin_weight) {
                                                        max_twin = x;
                                                        max_twin_weight = status.graph.weight(x);
                                                }

                                                deg_two_twins.push_back(x);
                                        }
                                }
                        }

                        for (auto x : status.graph[u]) {
                                if (status.node_status[x] == pack_status::not_set && x != max_twin && x != w &&
                                    status.graph.weight(x) > max_weight_u) {
                                        max_weight_u = status.graph.weight(x);
                                }
                                if (x == w) {
                                        u_w_adj = true;
                                }
                        }

                        for (auto x : status.graph[w]) {
                                if (status.node_status[x] == pack_status::not_set && x != max_twin && x != u &&
                                    status.graph.weight(x) > max_weight_w) {
                                        max_weight_w = status.graph.weight(x);
                                }
                                if (x == w) {
                                        u_w_adj = true;
                                }
                        }

                        NodeWeight weight_u = 0;
                        if (status.node_status[u] == pack_status::not_set) {
                                if (u_w_adj && max_twin_weight >= status.graph.weight(u)) {
                                        algo->set(u, pack_status::excluded);
                                } else {
                                        weight_u = status.graph.weight(u);
                                }
                        }
                        NodeWeight weight_w = 0;
                        if (status.node_status[w] == pack_status::not_set) {
                                if (u_w_adj && max_twin_weight >= status.graph.weight(w)) {
                                        algo->set(w, pack_status::excluded);
                                } else {
                                        weight_w = status.graph.weight(w);
                                }
                        }

                        if (max_twin_weight >= max_weight_u + max_weight_w &&
                            (max_twin_weight >= std::max(weight_u + max_weight_w, max_weight_u + weight_w) ||
                             (u_w_adj && max_twin_weight >= status.graph.get_max_weight_of_neigh(max_twin)))) {
                                algo->set(max_twin, pack_status::included, true, false, false);
                                continue;
                        } else {
                                // exclude other degree-one/zero vertices
                                //  since they have not a larger weight than max_simplicial (domination)
                                for (auto x : deg_two_twins) {
                                        if (x != max_twin) {
                                                algo->set(x, pack_status::excluded, true, false, false);
                                        }
                                }

                                continue;
                        }
                }
        }

        return oldn != status.remaining_nodes;
}

bool fast_track_complete_degree_one_removal_w2pack::reduce(weighted_reduce_algorithm *algo) {
        auto &config = algo->config;
        if (config.disable_fast_complete_degree_one_removal) return false;

        // we only run fast neighborhood removal once
        config.disable_fast_complete_degree_one_removal = true;

        auto &status = algo->global_status;
        size_t oldn = status.remaining_nodes;

        auto &current_marked = algo->set_1;
        auto &next_marked = algo->set_2;
        auto &blacklist = algo->set_3;
        auto &check = algo->buffers[0];
        check.clear();
        current_marked.clear();
        next_marked.clear();
        blacklist.clear();

        for (NodeID v_idx = 0; v_idx < marker.current_size(); v_idx++) {
                NodeID v = marker.current_vertex(v_idx);
                ASSERT_TRUE(status.node_status[v] == pack_status::not_set);
                if (status.graph.deg(v) == 1) {
                        marker.add(v);
                        next_marked.add(v);
                }
        }

        while (marker.next_size() > 0) {
                marker.get_next();
                std::swap(current_marked, next_marked);
                next_marked.clear();
                for (NodeID v_idx = 0; v_idx < marker.current_size(); v_idx++) {
                        NodeID v = marker.current_vertex(v_idx);
                        if (status.node_status[v] == pack_status::not_set  && !blacklist.get(v)) {
                                auto weight_v = status.graph.weight(v);
                                ASSERT_TRUE(status.graph[v].capacity() > 0);
                                auto u = status.graph[v][0];

                                auto &deg_one_simplicials = algo->buffers[1];
                                deg_one_simplicials.clear();
                                std::vector<NodeID> weight_shifted;

                                // deg_one_simplicials.push_back(v); we push back later
                                weighted_node max_simplicial = {.node = v, .weight = weight_v};
                                NodeWeight max_weight = weight_v;

                                if (status.graph.deg(v) == 1) {
                                        weight_shifted.push_back(u);
                                }
                                for (auto x : status.graph[u]) {
                                        if (status.node_status[x] == pack_status::not_set) {
                                                if (status.graph.deg(x) <= 1 && current_marked.get(x) && !blacklist.get(x)) {
                                                        // x has degree one and all 2-edges are defined over the visible
                                                        //   direct neighborhood -> 2-simplicial
                                                        if (status.graph.weight(x) > max_simplicial.weight) {
                                                                max_simplicial.node = x;
                                                                max_simplicial.weight = status.graph.weight(x);
                                                        }
                                                        deg_one_simplicials.push_back(x);
                                                } else {
                                                        weight_shifted.push_back(x);
                                                }
                                                if (status.graph.weight(x) > max_weight) {
                                                        max_weight = status.graph.weight(x);
                                                }
                                        }
                                }

                                if (max_simplicial.weight >= std::max(status.graph.weight(u), max_weight) ||
                                    (status.graph.deg(v) == 0 && max_simplicial.weight >= max_weight)) {
                                        //  2-simplicial v has max weight in its 2-clique N_2[v]
                                        for (auto x : status.graph[u]) {
                                                if(status.node_status[x] == pack_status::not_set) {
                                                        check.push_back(x);
                                                }
                                        }

                                        // include max_simplicial.node without excluding 2-neighbors and without initializing
                                        //  any 2-neighborhoods; we exclude its two-neighbors by via the direct neighborhood
                                        // of u
                                        algo->set_included_fast(max_simplicial.node);
                                        for (auto x : status.graph[u]) {
                                                if(status.node_status[x] == pack_status::not_set) {
                                                        algo->set(x, pack_status::excluded, true, false, false);
                                                }
                                        }


                                        continue;
                                } else {
                                        // exclude other degree-one/zero vertices
                                        //  since they have not a larger weight than max_simplicial (domination)
                                        for (auto x : deg_one_simplicials) {
                                                if (x != max_simplicial.node) {
                                                        algo->set(x, pack_status::excluded, true, false, false);
                                                }
                                        }
                                        check.push_back(v);

                                        for (size_t i = 0; i < weight_shifted.size(); ++i) {
                                                NodeID x = weight_shifted[i];
                                                if (status.graph.weight(x) <= max_simplicial.weight) {
                                                        algo->set(x, pack_status::excluded, true, false, false);
                                                        check.push_back(x);

                                                        weight_shifted[i] = weight_shifted.back();
                                                        weight_shifted.pop_back();
                                                        --i;
                                                }
                                        }

                                        fold(algo, max_simplicial, std::move(weight_shifted));

                                        continue;
                                }
                        }
                }
                for(auto x : check) {
                        NodeID visibles = 0;
                        bool mark = false;
                        NodeID marked;
                        for (auto w : status.graph[x]) {
                                if (status.node_status[w] == pack_status::not_set) {
                                        //if(++unsets > 1){break;};
                                        ++visibles;
                                        if (status.graph.deg(w) <= 1) {
                                                marked = w;
                                                mark = true;
                                        }
                                }
                        }
                        if(visibles == 1 && mark && !blacklist.get(marked)) {
                                marker.add(marked);
                                next_marked.add(marked);
                        }else if(visibles > 1) {
                                for (auto w : status.graph[x]) {
                                        if (status.node_status[w] == pack_status::not_set) {
                                                // w has 2-neighbors via hidden neighbors
                                                blacklist.add(w);
                                        }
                                }
                        }
                }
                check.clear();
        }

        return oldn != status.remaining_nodes;
}

bool fast_complete_degree_one_removal_w2pack::reduce(weighted_reduce_algorithm *algo) {
        auto &config = algo->config;
        if (config.disable_fast_complete_degree_one_removal) return false;

        // we only run fast neighborhood removal once
        config.disable_fast_complete_degree_one_removal = true;

        auto &status = algo->global_status;
        size_t oldn = status.remaining_nodes;

        for (NodeID v_idx = 0; v_idx < marker.current_size(); v_idx++) {
                NodeID v = marker.current_vertex(v_idx);
                if (status.node_status[v] == pack_status::not_set && status.graph[v].capacity() == 1) {
                        auto weight_v = status.graph.weight(v);
                        auto u = status.graph[v][0];

                        auto &deg_one_simplicials = algo->buffers[1];
                        deg_one_simplicials.clear();
                        std::vector<NodeID> weight_shifted;

                        // deg_one_simplicials.push_back(v); we push back later
                        weighted_node max_simplicial = {.node = v, .weight = weight_v};
                        NodeWeight max_weight = weight_v;

                        if (status.graph.deg(v) == 1) {
                                weight_shifted.push_back(u);
                        }
                        for (auto x : status.graph[u]) {
                                if (status.node_status[x] == pack_status::not_set) {
                                        if (status.graph[x].capacity() == 1) {
                                                if (status.graph.weight(x) > max_simplicial.weight) {
                                                        max_simplicial.node = x;
                                                        max_simplicial.weight = status.graph.weight(x);
                                                }
                                                deg_one_simplicials.push_back(x);
                                        } else {
                                                weight_shifted.push_back(x);
                                        }
                                        if (status.graph.weight(x) > max_weight) {
                                                max_weight = status.graph.weight(x);
                                        }
                                }
                        }

                        if (max_simplicial.weight >= std::max(status.graph.weight(u), max_weight) ||
                            (status.graph.deg(v) == 0 && max_simplicial.weight >= max_weight)) {
                                //  2-simplicial v has max weight in its 2-clique N_2[v]
                                algo->set(max_simplicial.node, pack_status::included, true, false, false);
                                continue;
                        } else {
                                // exclude other degree-one/zero vertices
                                //  since they have not a larger weight than max_simplicial (domination)
                                for (auto x : deg_one_simplicials) {
                                        if (x != max_simplicial.node) {
                                                algo->set(x, pack_status::excluded, true, false, false);
                                        }
                                }

                                for (size_t i = 0; i < weight_shifted.size(); ++i) {
                                        NodeID x = weight_shifted[i];
                                        if (status.graph.weight(x) <= max_simplicial.weight) {
                                                algo->set(x, pack_status::excluded, true, false, false);
                                                weight_shifted[i] = weight_shifted.back();
                                                weight_shifted.pop_back();
                                                --i;
                                        }
                                }

                                fold(algo, std::move(max_simplicial), std::move(weight_shifted));

                                continue;
                        }
                }
        }

        return oldn != status.remaining_nodes;
}

void fast_complete_degree_one_removal_w2pack::fold(
    weighted_reduce_algorithm *algo, const fast_complete_degree_one_removal_w2pack::weighted_node &simplicial,
    std::vector<NodeID> &&weight_shifted) {
        auto &status = algo->global_status;

        algo->set(simplicial.node, pack_status::folded, true, false, false);
        status.reduction_offset += simplicial.weight;

        for (auto node : weight_shifted) {
                ASSERT_TRUE(status.graph.weight(node) > simplicial.weight);
                status.graph.set_weight(node, status.graph.weight(node) - simplicial.weight);
        }

        status.folded_queue.push_back(get_reduction_type());

        restore_vec.emplace_back(simplicial, std::move(weight_shifted));
}
void fast_complete_degree_one_removal_w2pack::apply(weighted_reduce_algorithm *algo, NodeID node) {
        auto &status = algo->global_status;
        auto simplicial = restore_vec.back().simplicial.node;

        bool set_simplicial = true;

        for (auto target : restore_vec.back().non_simplicial) {
                if (status.node_status[target] == pack_status::included) {
                        set_simplicial = false;
                        break;
                }
        }

        status.sol_weight += restore_vec.back().simplicial.weight;

        restore(algo, node);

        if (set_simplicial) {
                status.node_status[simplicial] = pack_status::included;
        } else {
                status.node_status[simplicial] = pack_status::excluded;
        }
}
void fast_complete_degree_one_removal_w2pack::restore(weighted_reduce_algorithm *algo, NodeID node) {
        auto &status = algo->global_status;
        auto &data = restore_vec.back();

        algo->unset(data.simplicial.node);
        status.reduction_offset -= data.simplicial.weight;

        for (auto target : data.non_simplicial) {
                status.graph.set_weight(target, status.graph.weight(target) + data.simplicial.weight);
        }

        restore_vec.pop_back();
}

bool neighborhood_folding_w2pack::reduce(weighted_reduce_algorithm *algo) {
        auto config = algo->config;
        if (config.disable_neighborhood_folding) return false;
        // config.maintain_ub_sum_weight_two_neigh = false;
        auto &status = algo->global_status;
        size_t oldn = status.remaining_nodes;

        for (size_t v_idx_start = 0; v_idx_start < marker.current_size(); v_idx_start += batch_size) {
                if (algo->time_limit_reached()) {
                        break;
                }
                for (size_t v_idx = v_idx_start; v_idx < std::min(marker.current_size(), v_idx_start + batch_size);
                     v_idx++) {
                        NodeID v = marker.current_vertex(v_idx);

                        if (status.node_status[v] == pack_status::not_set) {
                                if (status.graph.deg(v) > 1) {
                                        continue;
                                }
                                auto &neigh = algo->set_1;
                                neigh.clear();
                                bool two_ps = true;

                                NodeWeight two_neigh_v = 0;
                                NodeWeight min_weight = std::numeric_limits<NodeWeight>::max();
                                for (auto u : status.graph[v]) {
                                        neigh.add(u);
                                        if (min_weight > status.graph.weight(u)) {
                                                min_weight = status.graph.weight(u);
                                        }
                                }

                                for (auto u : status.graph.neigh2(v)) {
                                        neigh.add(u);
                                        two_neigh_v += status.graph.weight(u);
                                        if (min_weight > status.graph.weight(u)) {
                                                min_weight = status.graph.weight(u);
                                        }
                                }

                                if (two_neigh_v + status.graph.get_sum_weight_of_neigh(v) <= status.graph.weight(v)) {
                                        algo->set(v, pack_status::included);
                                        continue;
                                }

                                if (two_neigh_v + status.graph.get_sum_weight_of_neigh(v) - min_weight >
                                    status.graph.weight(v)) {
                                        continue;
                                }

                                for (auto u : status.graph[v]) {
                                        for (auto x : status.graph[u]) {
                                                if (neigh.get(x)) {
                                                        two_ps = false;
                                                        break;
                                                }
                                        }
                                        if (!two_ps) {
                                                break;
                                        }
                                        for (auto x : status.graph.neigh2(u)) {
                                                if (neigh.get(x)) {
                                                        two_ps = false;
                                                        break;
                                                }
                                        }
                                        if (!two_ps) {
                                                break;
                                        }
                                }
                                if (!two_ps) {
                                        continue;
                                }
                                for (auto u : status.graph.neigh2(v)) {
                                        for (auto x : status.graph[u]) {
                                                if (neigh.get(x)) {
                                                        two_ps = false;
                                                        break;
                                                }
                                        }
                                        if (!two_ps) {
                                                break;
                                        }
                                        for (auto x : status.graph.neigh2(u)) {
                                                if (neigh.get(x)) {
                                                        two_ps = false;
                                                        break;
                                                }
                                        }
                                        if (!two_ps) {
                                                break;
                                        }
                                }
                                if (two_ps) {
                                        fold(algo, v, neigh, two_neigh_v + status.graph.get_sum_weight_of_neigh(v));
                                }
                        }
                }
        }

        std::cout << "neighborhood folding:" << oldn - status.remaining_nodes << " " << status.remaining_nodes
                  << std::endl;
        return oldn != status.remaining_nodes;
}
void neighborhood_folding_w2pack::fold(weighted_reduce_algorithm *algo, NodeID main_node, fast_set &MW2S_set,
                                       NodeWeight MW2S_weight) {
        auto &status = algo->global_status;

        restore_vec.emplace_back();
        auto &data = restore_vec.back();
        data.main_weight = status.graph.weight(main_node);
        data.MW2S_weight = MW2S_weight;

        // folded vertices
        auto &nodes = data.nodes;
        nodes.main = main_node;

        // data.main_one_neighbor = status.graph[main_node][0];
        data.main_two_neighbor_list = status.graph.neigh2(main_node);

        if (status.graph.deg(main_node) > 0) {
                auto u = status.graph[main_node][0];
                if (MW2S_set.get(u)) {
                        nodes.MW2S.push_back(u);
                } else {
                        algo->set(u, pack_status::excluded);
                }
        }

        for (auto u : data.main_two_neighbor_list) {
                if (MW2S_set.get(u)) {
                        nodes.MW2S.push_back(u);
                } else {
                        algo->set(u, pack_status::excluded);
                }
        }

        for (int i = nodes.MW2S.size() - 1; i >= 1; i--) {
                ASSERT_TRUE(status.graph.init_link_neighbors[nodes.MW2S[i]]);
                algo->set(nodes.MW2S[i], pack_status::folded, false);
        }

        algo->set(nodes.MW2S[0], pack_status::folded, true);

        data.main_two_neighbor_list = status.graph.neigh2(main_node);

        std::vector<NodeID> new_neighbors;
        auto &neighbors = MW2S_set;
        neighbors.clear();
        neighbors.add(main_node);

        // update neighborhood of folded vertex
        for (size_t idx = 0; idx < nodes.MW2S.size(); idx++) {
                NodeID MW2S_node = nodes.MW2S[idx];
                std::vector<NodeID> one_neigh;
                std::vector<NodeID> two_neigh;

                for (auto u : status.graph[MW2S_node]) {
                        if (neighbors.add(u)) {
                                new_neighbors.push_back(u);
                                // update dynamic graph
                                // insert link from u to main_node
                                // replace first hidden one neighbor (=MW2S_node) with
                                ASSERT_TRUE(*(status.graph[u].end()) == MW2S_node);
                                status.graph.restore_two_edge_and_replace(u, nodes.main, true);
                                status.graph.disable_reset_link_neigh(u);
                                one_neigh.push_back(u);
                        }
                }
                for (auto u : status.graph.neigh2(MW2S_node)) {
                        if (neighbors.add(u)) {
                                new_neighbors.push_back(u);

                                // update dynamic graph
                                status.graph.restore_two_edge_and_replace(u, nodes.main);
                                status.graph.disable_reset_link_neigh(u);
                                two_neigh.push_back(u);
                        }
                }

                data.MW2S_one_neigh.push_back(std::move(one_neigh));
                data.MW2S_two_neigh.push_back(std::move(two_neigh));
        }

        status.graph.neigh2(nodes.main) = m2s_advanced_dynamic_graph::neighbor_list(std::move(new_neighbors));
        status.graph.init_link_neighbors[nodes.main] = true;
        status.graph.disable_reset_link_neigh(nodes.main);

        // update offset and weight of folded vertex
        status.reduction_offset += data.main_weight;
        status.graph.set_weight(nodes.main, MW2S_weight - data.main_weight);

        status.folded_queue.push_back(get_reduction_type());
        algo->add_next_level_node(main_node);
        algo->add_next_level_two_neighborhood(main_node);

        ASSERT_TRUE(status.graph.init_link_neighbors[main_node]);
}
void neighborhood_folding_w2pack::apply(weighted_reduce_algorithm *algo, NodeID node) {
        auto &status = algo->global_status;
        auto nodes = restore_vec.back().nodes;
        auto main_status = status.node_status[nodes.main];
#ifndef NDEBUG
        NodeWeight sol = status.sol_weight + status.reduction_offset;
#endif
        restore(algo, node);

        if (main_status == pack_status::included) {
                status.node_status[nodes.main] = pack_status::excluded;

                for (auto target : nodes.MW2S) {
                        status.node_status[target] = pack_status::included;
                }
                status.sol_weight += status.graph.weight(nodes.main);
                // status.sol_weight += MW2S_weight;
        } else {
                status.node_status[nodes.main] = pack_status::included;

                for (auto target : nodes.MW2S) {
                        status.node_status[target] = pack_status::excluded;
                }

                status.sol_weight += status.graph.weight(nodes.main);
        }
        ASSERT_TRUE(status.sol_weight + status.reduction_offset == sol);
}
void neighborhood_folding_w2pack::restore(weighted_reduce_algorithm *algo, NodeID node) {
        auto &status = algo->global_status;
        auto &data = restore_vec.back();

#ifndef NDEBUG
        for (auto &u : status.graph.neigh2(data.nodes.main)) {
                ASSERT_TRUE(status.graph.init_link_neighbors[u]);
        }
#endif

        status.graph.hide_node(data.nodes.main);  // remove 2-edges towards data.nodes.main
        status.graph.set_visible(data.nodes.main);

        status.graph.enable_reset_link_neigh(data.nodes.main);
        status.graph.neigh2(data.nodes.main) = std::move(data.main_two_neighbor_list);

        for (size_t i = 0; i < data.nodes.MW2S.size(); i++) {
                algo->unset(data.nodes.MW2S[i]);

                // is "restored" in following loops
                for (auto neighbor : data.MW2S_one_neigh[i]) {
                        // resolve backup hidden two edge as hidden one edge
                        ASSERT_TRUE(*(status.graph.neigh2(neighbor).end()) == data.nodes.main);
                        status.graph.replace_last_hidden_two_edge(neighbor, *(status.graph[neighbor].end() - 1));
                        status.graph.replace_last_restored_one_edge(neighbor, data.nodes.MW2S[i]);
                        status.graph.enable_reset_link_neigh(neighbor);
                        /*if(status.graph.reset_two_neigh_allowed(neighbor)) {
                                status.graph.init_two_neighbors[neighbor] = false;
                        }*/
                }

                for (auto neighbor : data.MW2S_two_neigh[i]) {
                        status.graph.replace_last_restored_two_edge(neighbor, data.nodes.MW2S[i]);
                        status.graph.enable_reset_link_neigh(neighbor);
                        /*if(status.graph.reset_two_neigh_allowed(neighbor)) {
                                status.graph.init_two_neighbors[neighbor] = false;
                        }*/
                }
        }

        /*if(status.graph.reset_two_neigh_allowed(data.nodes.main)) {
                status.graph.init_two_neighbors[data.nodes.main] = false;
        }*/

        status.graph.set_weight(data.nodes.main, data.main_weight);
        status.reduction_offset -= data.main_weight;

        restore_vec.pop_back();
}
bool fold2_w2pack::reduce(weighted_reduce_algorithm *algo) {
        auto &config = algo->config;
        if (config.disable_fold2) return false;
        // config.maintain_ub_sum_weight_two_neigh = false;
        auto &status = algo->global_status;
        size_t oldn = status.remaining_nodes;

        for (size_t v_idx_start = 0; v_idx_start < marker.current_size(); v_idx_start += batch_size) {
                if (algo->time_limit_reached()) {
                        break;
                }
                for (size_t v_idx = v_idx_start; v_idx < std::min(marker.current_size(), v_idx_start + batch_size);
                     v_idx++) {
                        NodeID v = marker.current_vertex(v_idx);

                        if (status.node_status[v] == pack_status::not_set) {
                                if (status.graph.deg(v) > 1) {
                                        algo->remove_next_level_node(v, mw2ps_reduction_type::neighborhood_folding);
                                        continue;
                                }
                                if (status.graph.deg(v) + status.graph.deg2(v) != 2) {
                                        algo->remove_next_level_node(v, mw2ps_reduction_type::neighborhood_folding);
                                        continue;
                                }
                                bool two_ps = true;

                                NodeWeight two_neigh_v = 0;
                                NodeWeight max_weight = status.graph.get_max_weight_of_neigh(v);
                                std::vector<NodeID> neighbors(2, 0);
                                NodeID neighbors_count = 0;
                                for (auto u : status.graph[v]) {
                                        neighbors[neighbors_count++] = u;
                                }

                                for (auto u : status.graph.neigh2(v)) {
                                        neighbors[neighbors_count++] = u;
                                        two_neigh_v += status.graph.weight(u);
                                        if (max_weight < status.graph.weight(u)) {
                                                max_weight = status.graph.weight(u);
                                        }
                                }

                                if (two_neigh_v + status.graph.get_sum_weight_of_neigh(v) <= status.graph.weight(v)) {
                                        algo->set(v, pack_status::included);
                                        continue;
                                }

                                if (max_weight > status.graph.weight(v)) {
                                        algo->remove_next_level_node(v, mw2ps_reduction_type::neighborhood_folding);
                                        continue;
                                }

                                auto &u = neighbors[0];
                                for (auto x : status.graph[u]) {
                                        if (neighbors[1] == x) {
                                                two_ps = false;
                                                break;
                                        }
                                }
                                if (!two_ps) {
                                        algo->remove_next_level_node(v, mw2ps_reduction_type::neighborhood_folding);
                                        continue;
                                }
                                for (auto x : status.graph.neigh2(u)) {
                                        if (neighbors[1] == x) {
                                                two_ps = false;
                                                break;
                                        }
                                }
                                if (two_ps) {
                                        fold(algo, {v, {neighbors[0], neighbors[1]}});
                                } else {
                                        algo->remove_next_level_node(v, mw2ps_reduction_type::neighborhood_folding);
                                }
                        }
                }
        }

        std::cout << "fold2:" << oldn - status.remaining_nodes << " " << status.remaining_nodes << std::endl;
        return oldn != status.remaining_nodes;
}
void fold2_w2pack::fold(weighted_reduce_algorithm *algo, const fold2_w2pack::folded_nodes &nodes) {
        auto &status = algo->global_status;

        algo->set(nodes.neighbors[1], pack_status::folded, false);
        algo->set(nodes.neighbors[0], pack_status::folded, true);

        restore_vec.push_back({nodes, status.graph.weight(nodes.main), status.graph.neigh2(nodes.main), {}, {}});

        std::vector<NodeID> new_neighbors;
        auto &neighbors = algo->set_3;
        neighbors.clear();
        neighbors.add(nodes.main);

        // update neighborhood of folded vertex
        for (size_t idx = 0; idx < 2; idx++) {
                NodeID MW2S_node = nodes.neighbors[idx];
                std::vector<NodeID> one_neigh;
                std::vector<NodeID> two_neigh;

                for (auto u : status.graph[MW2S_node]) {
                        if (neighbors.add(u)) {
                                new_neighbors.push_back(u);
                                // update dynamic graph
                                // insert link from u to main_node
                                // replace first hidden one neighbor (=MW2S_node) with
                                ASSERT_TRUE(*(status.graph[u].end()) == MW2S_node);
                                status.graph.restore_two_edge_and_replace(u, nodes.main, true);
                                status.graph.disable_reset_link_neigh(u);
                                one_neigh.push_back(u);
                        }
                }
                for (auto u : status.graph.neigh2(MW2S_node)) {
                        if (neighbors.add(u)) {
                                new_neighbors.push_back(u);

                                // update dynamic graph
                                status.graph.restore_two_edge_and_replace(u, nodes.main);
                                status.graph.disable_reset_link_neigh(u);
                                two_neigh.push_back(u);
                        }
                }

                restore_vec.back().MW2S_one_neigh[idx] = std::move(one_neigh);
                restore_vec.back().MW2S_two_neigh[idx] = std::move(two_neigh);
        }

        status.graph.neigh2(nodes.main) = m2s_advanced_dynamic_graph::neighbor_list(std::move(new_neighbors));
        status.graph.init_link_neighbors[nodes.main] = true;
        status.graph.disable_reset_link_neigh(nodes.main);

        // update offset and weight of folded vertex
        status.reduction_offset += restore_vec.back().main_weight;
        status.graph.set_weight(nodes.main, status.graph.weight(nodes.neighbors[0]) +
                                                status.graph.weight(nodes.neighbors[1]) -
                                                status.graph.weight(nodes.main));

        status.folded_queue.push_back(get_reduction_type());
        algo->add_next_level_node(nodes.main);
        algo->add_next_level_two_neighborhood(nodes.main);

        // ASSERT_TRUE(status.graph.init_two_neighbors[main_node]);
}
void fold2_w2pack::apply(weighted_reduce_algorithm *algo, NodeID node) {
        auto &status = algo->global_status;
        auto nodes = restore_vec.back().nodes;
        auto main_status = status.node_status[nodes.main];

/*#ifndef NDEBUG
        auto &buf = algo->buffers[0];
        buf.clear();
        buf.push_back(nodes.main);

        for (auto v : buf) {
                if (status.node_status[v] == pack_status::excluded) {
                        // check for maximality
                        bool free_node=true;
                        for(auto u : status.graph[v]) {
                                if(status.node_status[u] == pack_status::included) {
                                        free_node = false;
                                        break;
                                }
                        }
                        if(free_node) {
                                for(auto u : status.graph.neigh2(v)) {
                                        if(status.node_status[u] == pack_status::included) {
                                                free_node = false;
                                                break;
                                        }
                                }
                        }
                        ASSERT_TRUE(!free_node);
                }
        }
#endif*/

#ifndef NDEBUG
        NodeWeight sol = status.sol_weight + status.reduction_offset;
#endif
        restore(algo, node);

        if (main_status == pack_status::included) {
                status.node_status[nodes.main] = pack_status::excluded;
                status.node_status[nodes.neighbors[0]] = pack_status::included;
                status.node_status[nodes.neighbors[1]] = pack_status::included;

                status.sol_weight += status.graph.weight(nodes.main);
                // status.graph.weight(nodes.neighbors[0]) + status.graph.weight(nodes.neighbors[1]);
        } else {
                status.node_status[nodes.main] = pack_status::included;
                status.node_status[nodes.neighbors[0]] = pack_status::excluded;
                status.node_status[nodes.neighbors[1]] = pack_status::excluded;

                status.sol_weight += status.graph.weight(nodes.main);
        }
/*#ifndef NDEBUG
        buf.clear();
        buf.push_back(nodes.main);
        buf.push_back(nodes.neighbors[1]);
        buf.push_back(nodes.neighbors[0]);

        for (auto v : buf) {
                if (status.node_status[v] == pack_status::excluded) {
                        // check for maximality
                        bool free_node=true;
                        for(auto u : status.graph[v]) {
                                if(status.node_status[u] == pack_status::included) {
                                        free_node = false;
                                        break;
                                }
                        }
                        if(free_node) {
                                for(auto u : status.graph.neigh2(v)) {
                                        if(status.node_status[u] == pack_status::included) {
                                                free_node = false;
                                                break;
                                        }
                                }
                        }
                        ASSERT_TRUE(!free_node);
                }
        }
#endif*/

        ASSERT_TRUE(status.sol_weight + status.reduction_offset == sol);
}
void fold2_w2pack::restore(weighted_reduce_algorithm *algo, NodeID node) {
        auto &status = algo->global_status;
        ASSERT_TRUE(!restore_vec.empty());
        auto &data = restore_vec.back();

#ifndef NDEBUG
        for (auto &u : status.graph.neigh2(data.nodes.main)) {
                ASSERT_TRUE(status.graph.init_link_neighbors[u]);
        }
#endif

        // remove 2-edges towards data.nodes.main, but do set data.nodes.main as hide
        status.graph.hide_node(data.nodes.main);
        status.graph.set_visible(data.nodes.main);

        // restore old two neighborhood of data.nodes.main
        status.graph.enable_reset_link_neigh(data.nodes.main);
        status.graph.neigh2(data.nodes.main) = std::move(data.main_two_neighbor_list);

        for (size_t i = 0; i < 2; i++) {
                algo->unset(data.nodes.neighbors[i]);

                // is "restored" in following loops
                for (auto neighbor : data.MW2S_one_neigh[i]) {
                        // resolve backup hidden two edge as hidden one edge
                        ASSERT_TRUE(*(status.graph.neigh2(neighbor).end()) == data.nodes.main);
                        status.graph.replace_last_hidden_two_edge(neighbor, *(status.graph[neighbor].end() - 1));
                        status.graph.replace_last_restored_one_edge(neighbor, data.nodes.neighbors[i]);
                        status.graph.enable_reset_link_neigh(neighbor);
                        /*if(status.graph.reset_two_neigh_allowed(neighbor)) {
                                status.graph.init_two_neighbors[neighbor] = false;
                        }*/
                }

                for (auto neighbor : data.MW2S_two_neigh[i]) {
                        status.graph.replace_last_restored_two_edge(neighbor, data.nodes.neighbors[i]);
                        status.graph.enable_reset_link_neigh(neighbor);
                        /*if(status.graph.reset_two_neigh_allowed(neighbor)) {
                                status.graph.init_two_neighbors[neighbor] = false;
                        }*/
                }
        }

        /*if(status.graph.reset_two_neigh_allowed(data.nodes.main)) {
                status.graph.init_two_neighbors[data.nodes.main] = false;
        }*/

        status.graph.set_weight(data.nodes.main, data.main_weight);
        status.reduction_offset -= data.main_weight;

        restore_vec.pop_back();
}
bool fast_direct_neighbor_removal_w2pack::reduce(weighted_reduce_algorithm *algo) {
        auto &config = algo->config;
        if (config.disable_fast_direct_neighbor_removal) return false;
        auto &status = algo->global_status;
        size_t oldn = status.remaining_nodes;

        for (size_t v_idx_start = 0; v_idx_start < marker.current_size(); v_idx_start += batch_size) {
                if (algo->time_limit_reached()) {
                        break;
                }
                for (size_t v_idx = v_idx_start; v_idx < std::min(marker.current_size(), v_idx_start + batch_size);
                     v_idx++) {
                        NodeID v = marker.current_vertex(v_idx);

                        if (status.node_status[v] == pack_status::not_set) {
                                // check and apply reductions with degree <= 1
                                auto weight_v = status.graph.weight(v);

                                auto &two_neigh_v_set = algo->set_1;
                                auto &feasible_one_neigh_v = algo->buffers[0];
                                feasible_one_neigh_v.clear();
                                two_neigh_v_set.clear();

                                for (auto u : status.graph[v]) {
                                        if (status.graph.weight(u) <= weight_v) {
                                                feasible_one_neigh_v.push_back(u);
                                        }
                                }

                                if (feasible_one_neigh_v.empty()) {
                                        continue;
                                }

                                NodeWeight sum_weight_two_neigh_v = status.get_weight_two_neigh(v);

                                if (status.graph.get_max_weight_of_neigh(v) + sum_weight_two_neigh_v <= weight_v) {
                                        algo->set(v, pack_status::included);
                                        continue;
                                }

                                for (auto &w : status.graph.neigh2(v)) {
                                        two_neigh_v_set.add(static_cast<int>(w));
                                }

                                if (status.graph.get_max_weight_of_neigh(v) + sum_weight_two_neigh_v <= weight_v) {
                                        algo->set(v, pack_status::included);
                                        continue;
                                }

                                for (auto u : feasible_one_neigh_v) {
                                        fast_reduce_direct_neighbor(algo, v, u, sum_weight_two_neigh_v);
                                        if (status.graph.get_max_weight_of_neigh(v) + sum_weight_two_neigh_v <=
                                            weight_v) {
                                                algo->set(v, pack_status::included);
                                                break;
                                        }
                                }
                        }
                }
        }

        std::cout << "fast direct neighbor removal:" << oldn - status.remaining_nodes << " " << status.remaining_nodes
                  << std::endl;
        return oldn != status.remaining_nodes;
}
bool fast_direct_neighbor_removal_w2pack::fast_reduce_direct_neighbor(weighted_reduce_algorithm *algo, NodeID v,
                                                                      NodeID u, NodeWeight sum_weight_two_neigh_v) {
        auto &status = algo->global_status;

        auto weight_v = status.graph.weight(v);
        auto weight_u = status.graph.weight(u);

        if (weight_u > weight_v) {
                return false;
        }
        auto delta = weight_v - weight_u;

        // assume the two neighborhood of v was added
        auto &two_neigh_v_set = algo->set_1;

        // determine upper bound U
        NodeWeight upper_bound = sum_weight_two_neigh_v;

        // upper_bound == sum weight two neighborhood of v
        if (upper_bound <= delta ||
            upper_bound + status.graph.get_sum_weight_of_neigh(v) <= status.graph.get_sum_weight_of_neigh(u)) {
                // remove u
                algo->set(u, pack_status::excluded);
                return true;
        }

        // upper_bound - status.graph.get_sum_weight_of_neigh(u) + status.graph.weight(v) > delta
        if (upper_bound > status.graph.get_sum_weight_of_neigh(u) - weight_v + delta) {
                // even if every direct neighbor of u is in the 2-neighborhood of v, we cannot safely exclude u
                return false;
        }

        // search for stronger upper bound
        for (auto &w : status.graph[u]) {
                // ASSERT_TRUE(two_neigh_v_set.get(static_cast<int>(w)) == (w != v));
                if (two_neigh_v_set.get(static_cast<int>(w))) {
                        upper_bound -= status.graph.weight(w);
                        if (upper_bound <= delta) {
                                // remove u
                                algo->set(u, pack_status::excluded);
                                return true;
                        }
                }
        }

        return false;
}
bool fast_domination_w2pack::reduce(weighted_reduce_algorithm *algo) {
        auto config = algo->config;
        if (config.disable_fast_wdomination) return false;
        auto &status = algo->global_status;
        size_t oldn = status.remaining_nodes;

        for (size_t v_idx_start = 0; v_idx_start < marker.current_size(); v_idx_start += batch_size) {
                if (algo->time_limit_reached()) {
                        break;
                }
                for (size_t v_idx = v_idx_start; v_idx < std::min(marker.current_size(), v_idx_start + batch_size);
                     v_idx++) {
                        NodeID v = marker.current_vertex(v_idx);

                        if (status.node_status[v] == pack_status::not_set) {
                                auto weight_v = status.graph.weight(v);

                                // initialize for subset test
                                auto &closed_one_neigh_v_set = algo->set_2;
                                closed_one_neigh_v_set.clear();

                                closed_one_neigh_v_set.add(static_cast<int>(v));
                                for (auto u : status.graph[v]) {
                                        closed_one_neigh_v_set.add(static_cast<int>(u));
                                }

                                // consider all direct neighbors of v
                                for (auto u : status.graph[v]) {
                                        if (status.graph.deg(v) > status.graph.deg(u)) {
                                                // early exit
                                                // we require in both case the degree equality
                                                continue;
                                        }

                                        // subset test
                                        if (!status.graph.init_link_neighbors[v]) {
                                                // early exit
                                                if (weight_v < std::max(status.graph.get_max_weight_of_neigh(u),
                                                                        status.graph.weight(u)) ||
                                                    2 * weight_v < status.graph.get_sum_weight_of_neigh(u)) {
                                                        continue;
                                                }

                                                // fast domination
                                                // test whether N[v]\subseteq N[u]
                                                // this is helpful because we do not need to initialize 2-neighborhoods
                                                // if the subset test fails

                                                NodeID intersection = 1;  // start with 1 to account for u
                                                for (auto x : status.graph[u]) {
                                                        if (closed_one_neigh_v_set.get(x)) {
                                                                intersection++;
                                                        }
                                                }
                                                if (intersection < status.graph.deg(v)) {
                                                        // N[v] is not a subset of N[v]
                                                        continue;
                                                }
                                        }

                                        // now we need the two neighborhood of v
                                        if (status.graph.deg(v) + status.graph.deg2(v) > status.graph.deg(u)) {
                                                continue;
                                        }

                                        if (weight_v >=
                                            std::max(status.graph.get_max_weight_of_neigh(u), status.graph.weight(u))) {
                                                algo->set(v, pack_status::included);
                                                break;
                                        }
                                        if (2 * weight_v >= status.graph.get_sum_weight_of_neigh(u)) {
                                                auto &one_neigh_u = algo->buffers[1];
                                                one_neigh_u.clear();
                                                for (auto x : status.graph[u]) {
                                                        if (x != v) {
                                                                one_neigh_u.push_back(x);
                                                        }
                                                }
                                                for (auto &x : one_neigh_u) {
                                                        algo->set(x, pack_status::excluded);
                                                }
                                                break;
                                        }
                                }
                        }
                }
        }

        std::cout << "fast domination:" << oldn - status.remaining_nodes << " " << status.remaining_nodes << std::endl;
        return oldn != status.remaining_nodes;
}

}  // namespace red2pack
