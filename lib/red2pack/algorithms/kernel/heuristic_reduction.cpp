#include "red2pack/algorithms/kernel/heuristic_reduction.h"
#include "red2pack/algorithms/kernel/weighted_reduce_algorithm.h"
#include "red2pack/tools/scoped_timer.h"

#include <random_functions.h>

namespace red2pack {

void heuristic_w2pack::reset() {
        general_reduction_w2pack::reset();
        top.clear();
        top_set.clear();
        candidates_q.clear();
}

void heuristic_w2pack::init(weighted_reduce_algorithm* algo) {
        auto &config = algo->config;
        switch (config.heuristic_rating) {
                case mw2ps_heuristic_ratings::weight:
                        rating_fnc = heuristic_rating_weight;
                case mw2ps_heuristic_ratings::deg:
                        rating_fnc = heuristic_rating_deg;
                case mw2ps_heuristic_ratings::weight_diff:
                default:
                        rating_fnc = heuristic_rating_weight_diff;
        }
}
bool heuristic_w2pack::reduce(weighted_reduce_algorithm* algo) {
        // cout_handler::enable_cout();
        auto& status = algo->global_status;
        auto& config = algo->config;
        size_t oldn = status.remaining_nodes;

        if (oldn == 0) {
                //  proceed with a MIS solver
                return false;
        }

        RatingValue rating_factor = 1;
        // pick status for heuristic candidates
        const two_pack_status new_pack_status = config.heuristic_decision_style;
        if (new_pack_status == two_pack_status::excluded) {
                rating_factor = -1;  // inverse rating
        }

        if (!config.adaptive_rating) {
                if (top.empty()) {
                        // fill
                        for (size_t v = 0; v < status.graph.size(); ++v) {
                                if (status.node_status[v] == two_pack_status::not_set) {
                                        top.push_back(v);
                                        top_value[v] = rating_factor * rating_fnc(status.graph, v);
                                }
                        }
                        std::sort(top.begin(), top.end(),
                                  [&](NodeID first, NodeID second) { return top_value[first] < top_value[second]; });
                }

                ASSERT_TRUE(!top.empty());
                if (config.perturb_prob > 0 && random_functions::nextDouble(0, 1) <= config.perturb_prob) {
                        using std::swap;
                        swap(top[random_functions::nextInt(0, top.size() - 1)], top[top.size() - 1]);
                }
                NodeID candidate = top.back();
                while (status.node_status[candidate] != two_pack_status::not_set) {
                        top.pop_back();
                        ASSERT_TRUE(!top.empty());
                        candidate = top.back();
                }
                if (new_pack_status == two_pack_status::excluded) {
                        algo->set(candidate, two_pack_status::folded);
                        status.folded_queue.push_back(get_reduction_type());
                        algo->add_next_level_neighborhood(candidate);
                        algo->add_next_level_two_neighborhood(candidate);
                } else {
                        algo->set(candidate, two_pack_status::included);
                }
                if (!status.peeled) {
                        status.first_peeled_node = candidate;
                        status.peeled = true;
                }

                // cout_handler::disable_cout();
                return oldn != status.remaining_nodes;
        }
        {
                if (candidates_q.empty() && top.empty()) {
                        // fill first time
                        RED2PACK_SCOPED_TIMER("Determine heuristic candidates first time");
                        candidates_q.reserve(status.n);
                        for (size_t v = 0; v < status.graph.size(); ++v) {
                                if (status.node_status[v] == two_pack_status::not_set) {
                                        auto rating = rating_factor * rating_fnc(status.graph, v);
                                        candidates_q.insert(v, rating);
                                }
                        }
                        top_value.set_size(status.n);
                } else {
                        // update rating for marked vertices
                        for (size_t v_idx = 0; v_idx < marker.current_size(); ++v_idx) {
                                auto v = marker.current_vertex(v_idx);
                                if (status.node_status[v] == two_pack_status::not_set) {
                                        auto rating = rating_factor * rating_fnc(status.graph, v);
                                        if (top_set.get(v)) {
                                                top_value[v] = rating;
                                        } else {
                                                ASSERT_TRUE(candidates_q.contains(v));
                                                candidates_q.changeKey(v, rating);
                                        }
                                } else if (candidates_q.contains(v)) {
                                        candidates_q.deleteNode(v);
                                }
                        }
                }

                ASSERT_TRUE(top.size() + candidates_q.size() >= status.remaining_nodes);

                // filter out non-visible vertices
                size_t i = 0;
                while (i < top.size()) {
                        auto c = top[i];
                        if (status.node_status[c] != two_pack_status::not_set) {
                                using std::swap;
                                swap(top[i], top.back());
                                top.pop_back();
                                top_set.remove(c);
                                continue;
                        }
                        ++i;
                }
                std::sort(top.begin(), top.end(),
                          [&](NodeID first, NodeID second) { return top_value[first] > top_value[second]; });

                auto& buf = algo->buffers[0];
                buf.clear();
                // merge
                i = 0;
                while (i < top.size() && buf.size() <= config.max_num_candidates) {
                        NodeID top_q;
                        if (!candidates_q.empty()) {
                                top_q = candidates_q.maxElement();
                                while (status.node_status[top_q] != two_pack_status::not_set) {
                                        candidates_q.deleteMax();
                                        if (!candidates_q.empty()) {
                                                top_q = candidates_q.maxElement();
                                        } else {
                                                break;
                                        }
                                }
                        }
                        if (candidates_q.empty() || top_value[top[i]] >= candidates_q.maxValue()) {
                                buf.push_back(top[i]);
                                ++i;
                        } else {
                                buf.push_back(top_q);
                                top_value[top_q] = candidates_q.maxValue();
                                candidates_q.deleteMax();
                        }
                }

                while (i < top.size()) {
                        candidates_q.insert(top[i], top_value[top[i]]);
                        ++i;
                }

                top_set.clear();
                top.clear();

                for (auto c : buf) {
                        top.push_back(c);
                        top_set.add(c);
                }

                unsigned candidate_rank = random_functions::nextInt(
                    0, std::min(static_cast<std::size_t>(config.max_num_candidates - 1), status.remaining_nodes - 1));
		// top might not contain enough elements
                for (unsigned j = top.size(); j < candidate_rank + 1; ++j) {
                        auto candidate = candidates_q.maxElement();
                        while (status.node_status[candidate] != two_pack_status::not_set) {
                                candidates_q.deleteMax();
                                candidate = candidates_q.maxElement();
                        }
                        top.push_back(candidate);
                        top_set.add(candidate);
                        top_value[candidate] = candidates_q.maxValue();
                        candidates_q.deleteMax();
                        if (candidates_q.empty()) {
                                break;
                        }
                }
                auto candidate = top[candidate_rank];
                ASSERT_TRUE(status.node_status[candidate] == two_pack_status::not_set);

                if (!status.peeled) {
                        status.first_peeled_node = candidate;
                        status.peeled = true;
                }


#ifndef NDEBUG
		for (size_t v = 0; v < status.n; ++v) {
                                if (status.node_status[v] == two_pack_status::not_set) {
                                        auto rating = rating_factor * rating_fnc(status.graph, v);
                                        if (top_set.get(v)) {
						ASSERT_TRUE(top_value[v] == rating);
					}else {
						ASSERT_TRUE(candidates_q.contains(v));
						ASSERT_TRUE(candidates_q.getKey(v) == rating);
						ASSERT_TRUE(top_value[candidate] >= rating);
                                        }
				}
                }
#endif

                if (new_pack_status == two_pack_status::excluded) {
                        algo->set(candidate, two_pack_status::folded);
                        status.folded_queue.push_back(get_reduction_type());
                        algo->add_next_level_neighborhood(candidate);
                        algo->add_next_level_two_neighborhood(candidate);
                } else {
                        algo->set(candidate, two_pack_status::included);
                }

                std::cout << "heuristic: " << oldn - status.remaining_nodes << " " << status.remaining_nodes
                          << std::endl;
        }
        // cout_handler::disable_cout();
        return oldn != status.remaining_nodes;
}
void heuristic_w2pack::restore(weighted_reduce_algorithm* algo, NodeID node) {
        // node was peeled
        algo->unset(node);
}
void heuristic_w2pack::apply(weighted_reduce_algorithm* algo, NodeID node) {
        auto& status = algo->global_status;

        restore(algo, node);
        // node was peeled; check whether it can still be included
        bool maximal = false;
        for (auto& neigh : status.graph[node]) {
                if (status.node_status[neigh] == two_pack_status::included) {
                        maximal = true;
                        break;
                }
        }
        if (!maximal) {
                // ASSERT_TRUE(global_status.graph.two_neighbors_pre_initialized ||
                // !global_status.graph.init_two_neighbors[v]);
                for (auto& neigh : status.graph.neigh2(node)) {
                        if (status.node_status[neigh] == two_pack_status::included) {
                                maximal = true;
                                break;
                        }
                }
        }
        if (!maximal) {
                status.node_status[node] = two_pack_status::included;
                status.sol_weight += status.graph.weight(node);
        } else {
                status.node_status[node] = two_pack_status::excluded;
        }
}
}  // namespace red2pack
