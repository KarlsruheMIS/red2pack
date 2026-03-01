//
// Created by Jannick Borowitz on 16.10.23.
//

#include "red2pack/algorithms/kernel/weighted_reduce_algorithm.h"
#include "red2pack/algorithms/kernel/heuristic_reduction.h"
#include "red2pack/algorithms/kernel/weighted_reductions.h"
#include "red2pack/tools/m2s_log.h"

#include <branch_and_reduce_algorithm.h>
#include <random_functions.h>

#include <iostream>
#include <utility>



namespace red2pack {

weighted_reduce_algorithm::weighted_reduce_algorithm(m2s_graph_access& G, M2SConfig m2s_config)
    : set_1(G.number_of_nodes()),
      set_2(G.number_of_nodes()),  // so far on demand two neighborhood is not supported
      set_3(G.number_of_nodes()),
      weights_neigh(G.number_of_nodes()),
      buffers(2, sized_vector<NodeID>(G.number_of_nodes())),
      signed_buffer(G.number_of_nodes()),
      global_status(G, !m2s_config.on_demand_two_neighborhood, m2s_config.maintain_two_neigh),
      config(std::move(m2s_config)) {
        if (config.reduction_style == M2SConfig::Reduction_Style2::strong) {
                global_status.reductions =
                    make_w2reduction_vector<fast_track_complete_degree_one_removal_w2pack,
                                            fast_degree_two_removal_w2pack, fast_neighborhood_removal_w2pack,
                                            neighborhood_removal_w2pack, weight_transfer_w2pack,
                                            split_intersection_removal_w2pack, direct_neighbor_removal_w2pack,
                                            two_neighbor_removal_w2pack, fold2_w2pack, neighborhood_folding_w2pack>(
                        global_status.n);
                global_status.fast_reductions_offset = 3;
        } else if (config.reduction_style == M2SConfig::Reduction_Style2::main) {
                global_status.reductions =
                    make_w2reduction_vector<neighborhood_removal_w2pack, fast_domination_w2pack,
                                            single_fast_domination_w2pack, weight_transfer_w2pack,
                                            split_intersection_removal_w2pack, direct_neighbor_removal_w2pack,
                                            two_neighbor_removal_w2pack, neighborhood_folding_w2pack>(global_status.n);
                global_status.fast_reductions_offset = 0;
        } else if (config.reduction_style == M2SConfig::Reduction_Style2::fast) {
                global_status.reductions =
                    make_w2reduction_vector<fast_track_complete_degree_one_removal_w2pack,
                                            fast_degree_two_removal_w2pack, fast_neighborhood_removal_w2pack>(
                        global_status.n);
                global_status.fast_reductions_offset = 3;
        }  else if (config.reduction_style == M2SConfig::Reduction_Style2::full) {
                global_status.reductions =
                    make_w2reduction_vector<fast_track_complete_degree_one_removal_w2pack,
                                            fast_degree_two_removal_w2pack, fast_neighborhood_removal_w2pack,
                                            neighborhood_removal_w2pack, fast_domination_w2pack,
                                            single_fast_domination_w2pack, weight_transfer_w2pack,
                                            split_intersection_removal_w2pack, direct_neighbor_removal_w2pack,
                                            two_neighbor_removal_w2pack, fold2_w2pack, neighborhood_folding_w2pack>(
                        global_status.n);
                global_status.fast_reductions_offset = 3;
        } else if (config.reduction_style == M2SConfig::Reduction_Style2::heuristic) {
                global_status.reductions = make_w2reduction_vector<
                    neighborhood_removal_w2pack, fast_domination_w2pack, single_fast_domination_w2pack,
                    weight_transfer_w2pack, split_intersection_removal_w2pack, direct_neighbor_removal_w2pack,
                    two_neighbor_removal_w2pack, fold2_w2pack, neighborhood_folding_w2pack,
                    heuristic_w2pack>(global_status.n);
        
                global_status.fast_reductions_offset = 0;
        }
        reduction_map.resize(MW2PS_REDUCTION_NUM);
        for (size_t i = 0; i < global_status.reductions.size(); i++) {
                reduction_map[static_cast<size_t>(global_status.reductions[i]->get_reduction_type())] = i;
        }
}

void weighted_reduce_algorithm::set(NodeID node, two_pack_status new_two_pack_status, bool push_modified,
                                    bool hide_two_edges, bool mark_neigh) {
        if (push_modified) {
                global_status.modified_queue.push_back(node);
        }

        RED2PACK_ASSERT_TRUE(global_status.node_status[node] == two_pack_status::not_set);
        if (new_two_pack_status == two_pack_status::included) {
                RED2PACK_ASSERT_TRUE(global_status.node_status[node] == two_pack_status::not_set);
                global_status.node_status[node] = new_two_pack_status;
                global_status.remaining_nodes--;
                global_status.sol_weight += global_status.graph.weight(node);

                {
                        // RED2PACK_SCOPED_TIMER("hide included");
                        global_status.graph.hide_node(node, hide_two_edges);
                }

                {
                        // RED2PACK_SCOPED_TIMER("hide for one-neigh");
                        for (auto neighbor : global_status.graph[node]) {
                                RED2PACK_ASSERT_TRUE(global_status.graph.visible(neighbor));
                                set(neighbor, two_pack_status::excluded, true, hide_two_edges, mark_neigh);
                        }
                }

                {
                        // RED2PACK_SCOPED_TIMER("hide for two-neigh");
                        for (auto neighbor : global_status.graph.neigh2(node)) {
                                RED2PACK_ASSERT_TRUE(global_status.graph.visible(neighbor));
                                set(neighbor, two_pack_status::excluded, true, hide_two_edges, mark_neigh);
                        }
                }

        } else {  // exclude or fold
                global_status.node_status[node] = new_two_pack_status;
                global_status.remaining_nodes--;
                global_status.graph.hide_node(node, hide_two_edges);
                if (mark_neigh && new_two_pack_status != two_pack_status::folded) {
                        add_next_level_neighborhood(node);
                        add_next_level_two_neighborhood(node);
                }
        }
}

void weighted_reduce_algorithm::set_included_fast(NodeID node) {
        global_status.modified_queue.push_back(node);

        RED2PACK_ASSERT_TRUE(global_status.node_status[node] == two_pack_status::not_set);

        RED2PACK_ASSERT_TRUE(global_status.node_status[node] == two_pack_status::not_set);
        global_status.node_status[node] = two_pack_status::included;
        global_status.remaining_nodes--;
        global_status.sol_weight += global_status.graph.weight(node);

        {
                // RED2PACK_SCOPED_TIMER("hide included");
                global_status.graph.hide_node(node, false);
        }

        {
                // RED2PACK_SCOPED_TIMER("hide for one-neigh");
                for (auto neighbor : global_status.graph[node]) {
                        RED2PACK_ASSERT_TRUE(global_status.graph.visible(neighbor));
                        set(neighbor, two_pack_status::excluded, true, false, false);
                }
        }
}

void weighted_reduce_algorithm::set_two_simplicial_included(NodeID included_node, sized_vector<NodeID>& two_clique,
                                                            fast_set& two_clique_set) {
        {
                // RED2PACK_SCOPED_TIMER("hide two-clique");
                global_status.graph.bulk_hide(two_clique, two_clique_set);
                for (auto x : global_status.graph.get_last_one_border_bulk_hide()) {
                        add_next_level_node(x);
                }
                for (auto x : global_status.graph.get_last_two_border_bulk_hide()) {
                        add_next_level_node(x);
                }
        }

        RED2PACK_ASSERT_TRUE(global_status.node_status[included_node] == two_pack_status::not_set);
        global_status.node_status[included_node] = two_pack_status::included;
        global_status.remaining_nodes--;
        global_status.sol_weight += global_status.graph.weight(included_node);

        // we need to set the status and push the modifications
        // bulk_hide has hidden the 2-clique members in the order as defined by sorting (done by bulk_hide) two_clique
        for (auto member : two_clique) {
                if (member != included_node) {
                        global_status.node_status[member] = two_pack_status::excluded;
                        global_status.remaining_nodes--;
                }
                global_status.modified_queue.push_back(member);
        }
}

void weighted_reduce_algorithm::bulk_exclude(sized_vector<NodeID>& nodes, fast_set& nodes_set) {
        {
                // RED2PACK_SCOPED_TIMER("bulk hide");
                global_status.graph.bulk_hide(nodes, nodes_set);
                for (auto x : global_status.graph.get_last_one_border_bulk_hide()) {
                        add_next_level_node(x);
                }
                for (auto x : global_status.graph.get_last_two_border_bulk_hide()) {
                        add_next_level_node(x);
                }
        }

        for (auto member : nodes) {
                global_status.node_status[member] = two_pack_status::excluded;
                global_status.remaining_nodes--;
                global_status.modified_queue.push_back(member);
        }
}

void weighted_reduce_algorithm::unset(NodeID node, bool restore) {
        if (global_status.node_status[node] == two_pack_status::included) {
                global_status.sol_weight -= global_status.graph.weight(node);
        }

        global_status.node_status[node] = two_pack_status::not_set;
        global_status.remaining_nodes++;

        if (restore) {
                global_status.graph.restore_node(node);
        }
}

void weighted_reduce_algorithm::init_reduction_step() {
        if (!global_status.reductions[active_reduction_index]->has_run) {
                global_status.reductions[active_reduction_index]->marker.fill_current_ascending(global_status.n);
                global_status.reductions[active_reduction_index]->marker.clear_next();
                global_status.reductions[active_reduction_index]->has_run = true;
        } else {
                global_status.reductions[active_reduction_index]->marker.get_next();
        }
        if (config.shuffle_redu_order) {
                global_status.reductions[active_reduction_index]->marker.shuffle();
        }
}

void weighted_reduce_algorithm::add_remaining_nodes_greedily() {
        if (global_status.remaining_nodes > 0) {
                heuristic_w2pack::RatingFunc rating_fn;
                switch (config.heuristic_rating) {
                        case mw2ps_heuristic_ratings::weight:
                                rating_fn = heuristic_w2pack::heuristic_rating_weight;
                        case mw2ps_heuristic_ratings::deg:
                                rating_fn = heuristic_w2pack::heuristic_rating_deg;
                        case mw2ps_heuristic_ratings::weight_diff:
                        default:
                                rating_fn = heuristic_w2pack::heuristic_rating_weight_diff;
                }

                auto& nodes = buffers[0];
                auto& rating = signed_buffer;
                rating.set_size(global_status.graph.size());
                nodes.clear();

                for (size_t node = 0; node < global_status.n; ++node) {
                        if (global_status.node_status[node] == two_pack_status::not_set) {
                                nodes.push_back(node);
                                rating[node] = rating_fn(global_status.graph, node);
                        }
                }

                // sort in descending order by node weights
                std::sort(nodes.begin(), nodes.end(),
                          [&rating](NodeID lhs, NodeID rhs) { return rating[lhs] > rating[rhs]; });

                for (NodeID node : nodes) {
                        bool free_node = true;

                        for (NodeID neighbor : global_status.graph[node]) {
                                if (global_status.node_status[neighbor] == two_pack_status::included) {
                                        free_node = false;
                                        break;
                                }
                        }
                        if (free_node) {
                                for (NodeID neighbor : global_status.graph.neigh2(node)) {
                                        if (global_status.node_status[neighbor] == two_pack_status::included) {
                                                free_node = false;
                                                break;
                                        }
                                }
                        }

                        if (free_node) {
                                global_status.node_status[node] = two_pack_status::included;
                                global_status.sol_weight += global_status.graph.weight(node);
                        } else {
                                global_status.node_status[node] = two_pack_status::excluded;
                        }
                }
        }
}

void weighted_reduce_algorithm::reduce_graph_internal() {
        random_functions::setSeed(config.seed);

        // upper bound for fast_neighborhood_removal
        if (!config.disable_fast_neighborhood_removal) {
                for (NodeID v = 0; v < global_status.n; v++) {
                        weights_neigh[v] = 0;
                        NodeWeight weight_v = global_status.graph.weight(v);
                        for (auto& u : global_status.graph[v]) {
                                weights_neigh[v] += global_status.graph.get_sum_weight_of_neigh(u) - weight_v;
                        }
                }
        }
        for (auto& reduction : global_status.reductions) {
                reduction->init(this);
        }

        {
                bool progress;
                do {
                        progress = false;
                        for (auto& reduction : global_status.reductions) {
                                active_reduction_index =
                                    reduction_map[static_cast<size_t>(reduction->get_reduction_type())];

                                cout_handler::disable_cout();
                                {
                                        init_reduction_step();
                                        NodeID old_n = global_status.remaining_nodes;
                                        auto start_t = std::chrono::system_clock::now();
                                        progress = reduction->reduce(this);
                                        auto end_t = std::chrono::system_clock::now();
                                        std::chrono::duration<double> elapsed_time = end_t - start_t;
                                        m2s_log::instance()->add_reduced_nodes_mw2ps(
                                            reduction->get_reduction_type(), old_n - global_status.remaining_nodes,
                                            elapsed_time.count());
                                }
                                cout_handler::enable_cout();

                                if (progress || time_limit_reached()) break;
                        }
                } while ((progress) && !time_limit_reached());
        }

        if (config.uses_heuristic_reduce_style()) {
                // heuristic variants should fully heuristically reduce the graph
                // we add remaining nodes greedily
                add_remaining_nodes_greedily();
        }

        if (config.time_limit < t.elapsed()) std::cout << "%timeout" << std::endl;
}

void weighted_reduce_algorithm::run_reductions() {
        t.restart();
        reduce_graph_internal();
}

void weighted_reduce_algorithm::restore() {
        while (!global_status.modified_queue.empty()) {
                NodeID node = global_status.modified_queue.back();
                global_status.modified_queue.pop_back();

                if (global_status.node_status[node] == two_pack_status::folded) {
                        auto type = global_status.folded_queue.back();
                        global_status.folded_queue.pop_back();
                        global_status.reductions[reduction_map[static_cast<size_t>(type)]]->restore(this, node);
                } else if (!global_status.graph.visible(
                               node)) {  // not might be already visible (see set_two_simplicial_included)
                        global_status.graph.restore_node(node);
                        global_status.node_status[node] = two_pack_status::not_set;
                }
        }
        global_status.peeled = false;
        global_status.remaining_nodes = global_status.n;
        global_status.sol_weight = 0;
        global_status.reduction_offset = 0;
        for (auto& reduction : global_status.reductions) {
                reduction->reset();
        }
        RED2PACK_ASSERT_TRUE(global_status.reduction_offset == 0);
        RED2PACK_ASSERT_TRUE(global_status.sol_weight == 0);
        RED2PACK_ASSERT_TRUE(global_status.remaining_nodes == global_status.n);
        RED2PACK_ASSERT_TRUE(global_status.modified_queue.empty());
        RED2PACK_ASSERT_TRUE(global_status.folded_queue.empty());
}

// assume restore or build solution was called before
// resetting status so that it can be used again
void weighted_reduce_algorithm::reset() {
        RED2PACK_ASSERT_TRUE(global_status.modified_queue.empty());
        RED2PACK_ASSERT_TRUE(global_status.folded_queue.empty());
        std::fill(global_status.node_status.begin(), global_status.node_status.end(), two_pack_status::not_set);
        global_status.peeled = false;
        global_status.remaining_nodes = global_status.n;
        global_status.sol_weight = 0;
        global_status.reduction_offset = 0;
        for (auto& reduction : global_status.reductions) {
                reduction->reset();
        }
        RED2PACK_ASSERT_TRUE(global_status.reduction_offset == 0);
        RED2PACK_ASSERT_TRUE(global_status.sol_weight == 0);
        RED2PACK_ASSERT_TRUE(global_status.remaining_nodes == global_status.n);
        RED2PACK_ASSERT_TRUE(global_status.modified_queue.empty());
        RED2PACK_ASSERT_TRUE(global_status.folded_queue.empty());
}

void weighted_reduce_algorithm::restore_exact_kernel() {
        RED2PACK_ASSERT_TRUE(config.uses_heuristic_reduce_style());
        if (!global_status.peeled) {
                return;
        }
        while (!global_status.modified_queue.empty()) {
                NodeID node = global_status.modified_queue.back();

                global_status.modified_queue.pop_back();

                if (global_status.node_status[node] == two_pack_status::folded) {
                        auto type = global_status.folded_queue.back();
                        global_status.folded_queue.pop_back();
                        global_status.reductions[reduction_map[static_cast<size_t>(type)]]->apply(this, node);
                } else if (!global_status.graph.visible(node)) {
                        global_status.graph.restore_node(node);
                }

                if (node == global_status.first_peeled_node) {
                        break;
                }
        }
}

void weighted_reduce_algorithm::apply_heuristic_solution(m2s_graph_access& exact_reduced_graph,
                                                         const std::vector<NodeID>& former_node_id) {
        RED2PACK_ASSERT_TRUE(config.uses_heuristic_reduce_style());
        NodeWeight heuristic_sol_weight = 0;
        for (size_t node = 0; node < exact_reduced_graph.number_of_nodes(); node++) {
                if (global_status.graph.visible(former_node_id[node])) {
                        if (global_status.node_status[former_node_id[node]] == two_pack_status::included) {
                                exact_reduced_graph.setPartitionIndex(node, 1);
                                heuristic_sol_weight += exact_reduced_graph.getNodeWeight(node);
                        } else {
                                exact_reduced_graph.setPartitionIndex(node, 0);
                        }
                }
        }
        RED2PACK_ASSERT_TRUE(global_status.sol_weight >= heuristic_sol_weight);
        // restore sol_weight in global_status
        global_status.sol_weight -= heuristic_sol_weight;
}

void weighted_reduce_algorithm::build_solution(std::vector<bool>& solution_vec) {
        RED2PACK_ASSERT_TRUE(solution_vec.size() == global_status.n);
        if (global_status.remaining_nodes > 0) {
                // set solution for exact kernel
                for (NodeID node = 0; node < global_status.n; node++) {
                        if (global_status.node_status[node] == two_pack_status::not_set) {
                                if (solution_vec[node]) {
                                        global_status.node_status[node] = two_pack_status::included;
                                } else {
                                        global_status.node_status[node] = two_pack_status::excluded;
                                }
                        }
                }
        }

        // restore exact reductions
        while (!global_status.modified_queue.empty()) {
                NodeID node = global_status.modified_queue.back();
                global_status.modified_queue.pop_back();

                if (global_status.node_status[node] == two_pack_status::folded) {
                        auto type = global_status.folded_queue.back();
                        global_status.folded_queue.pop_back();
                        global_status.reductions[reduction_map[static_cast<size_t>(type)]]->apply(this, node);
                } else if (!global_status.graph.visible(
                               node)) {  // not might be already visible (see set_two_simplicial_included)
                        global_status.graph.restore_node(node);
                }
        }

        // build global solution
        for (size_t i = 0; i < solution_vec.size(); i++) {
                if (global_status.node_status[i] == two_pack_status::included) {
                        solution_vec[i] = true;
                } else {
                        solution_vec[i] = false;
                }
        }

        // ensure solution is maximized (solution might not be maximal yet if kernel solution was not optimal)
        // the maximization algorithm only works if every link is defined via common direct neighbors
        if (config.on_demand_two_neighborhood) {
                auto& distance_one_nodes = set_1;  // nodes with a direct neighbor that is part of the solution
                distance_one_nodes.clear();
                auto& free_nodes = buffers[0];
                free_nodes.clear();
                for (NodeID node = 0; node < global_status.n; ++node) {
                        if (solution_vec[node]) {
                                for (auto target : global_status.graph[node]) {
                                        distance_one_nodes.add(target);
                                }
                        }
                }
                for (NodeID node = 0; node < global_status.n; ++node) {
                        if (!solution_vec[node] && !distance_one_nodes.get(node)) {
                                bool free_node = true;
                                for (auto target : global_status.graph[node]) {
                                        // note: node cannot have distance one to some direct neighbor; otherwise it
                                        // would be added to distance_one_nodes
                                        RED2PACK_ASSERT_TRUE(!solution_vec[target]);
                                        if (distance_one_nodes.get(target)) {
                                                free_node = false;
                                                break;
                                        }
                                }
                                if (free_node) {
                                        free_nodes.push_back(node);
                                }
                        }
                }
                std::sort(free_nodes.begin(), free_nodes.end(),
                          [&g = global_status.graph](NodeID first, NodeID second) {
                                  return g.weight(first) > g.weight(second);
                          });
                for (NodeID node : free_nodes) {
                        RED2PACK_ASSERT_TRUE(!solution_vec[node]);
                        bool free = !distance_one_nodes.get(node);
                        if (free) {
                                for (auto target : global_status.graph[node]) {
                                        if (distance_one_nodes.get(target)) {
                                                free = false;
                                                break;
                                        }
                                }
                        }

                        if (free) {
                                solution_vec[node] = true;
                                global_status.node_status[node] = two_pack_status::included;
                                global_status.sol_weight += global_status.graph.weight(node);
                                for (auto target : global_status.graph[node]) {
                                        distance_one_nodes.add(target);
                                }
                        }
                }
        }
}

NodeWeight weighted_reduce_algorithm::get_solution_weight() const {
        return global_status.sol_weight + global_status.reduction_offset;
}
void weighted_reduce_algorithm::get_exact_kernel(m2s_graph_access& reduced_graph, std::vector<NodeID>& reduced_node_id,
                                                 std::vector<NodeID>& former_node_id) {
        if (config.rnp_warm_start_for_kernel) {
                restore_exact_kernel();
        }

        auto& status = global_status;

        reduced_node_id.resize(status.n);
        former_node_id.resize(status.n);

        NodeID reduced_nodes = 0;
        std::size_t reduced_edges = 0;
        std::size_t reduced_two_edges = 0;

        for (size_t i = 0; i < status.n; i++) {
                if (status.graph.visible(i)) {
                        reduced_node_id[i] = reduced_nodes;
                        former_node_id[reduced_nodes++] = i;

                        reduced_edges += status.graph.deg(i);
                        reduced_two_edges += status.graph.deg2(i);
                }
        }

        // check for overflows (there might be too many edges)
        if (reduced_two_edges > std::numeric_limits<EdgeID>::max()) {
                std::cout << "ERROR: Cannot represent all 2-edges of reduced graph with EdgeID!" << std::endl;
                exit(1);
        }

        reduced_graph.start_construction(reduced_nodes, reduced_edges, reduced_two_edges);

        for (size_t i = 0; i < reduced_nodes; i++) {
                auto node = former_node_id[i];
                reduced_graph.new_node();
                reduced_graph.setNodeWeight(i, status.graph.weight(node));
                for (size_t j = 0; j < status.graph[node].size(); j++) {
                        auto target = status.graph[node][j];
                        if (status.graph.visible(target)) {
                                auto e = reduced_graph.new_edge(i, reduced_node_id[target]);
                                reduced_graph.setEdgeWeight(e, 1);
                        }
                }
                for (size_t j = 0; j < status.graph.neigh2(node).size(); j++) {
                        auto target = status.graph.neigh2(node)[j];
                        if (status.graph.visible(target)) {
                                auto e = reduced_graph.new_link(i, reduced_node_id[target]);
                                reduced_graph.setLinkWeight(e, 1);
                        }
                }
        }

        reduced_graph.finish_construction();

        if (config.rnp_warm_start_for_kernel) {
                apply_heuristic_solution(reduced_graph, former_node_id);
        }
}

void weighted_reduce_algorithm::add_next_level_node(NodeID node) {
        // mark node for next round of status.reductions
        for (auto reduction_iter = global_status.reductions.begin() + global_status.fast_reductions_offset;
             reduction_iter != global_status.reductions.end(); reduction_iter++) {
                auto& reduction = *reduction_iter;
                if (reduction->has_run) {
                        reduction->marker.add(node);
                }
        }
}

void weighted_reduce_algorithm::remove_next_level_node(NodeID node, mw2ps_reduction_type reduction_type) {
        // unmark node for next round of status.reductions
        global_status.reductions[reduction_map[static_cast<size_t>(reduction_type)]]->marker.remove(node);
}

void weighted_reduce_algorithm::add_next_level_node_no_fast(NodeID node) {
        // mark node for next round of status.reductions
        for (auto& reduction : global_status.reductions) {
                if (reduction->has_run &&
                    reduction->get_reduction_type() != mw2ps_reduction_type::fast_neighborhood_removal) {
                        reduction->marker.add(node);
                }
        }
}

void weighted_reduce_algorithm::add_next_level_nodes(const std::vector<NodeID>& nodes) {
        for (auto node : nodes) {
                add_next_level_node(node);
        }
}

void weighted_reduce_algorithm::add_next_level_nodes(const sized_vector<NodeID>& nodes) {
        for (auto node : nodes) {
                add_next_level_node(node);
        }
}

void weighted_reduce_algorithm::add_next_level_neighborhood(NodeID node) {
        // node has been excluded in mis -> neighboring vertices are interesting for next round of reduction
        for (auto neighbor : global_status.graph[node]) {
                add_next_level_node(neighbor);
        }
}

void weighted_reduce_algorithm::add_next_level_two_neighborhood(NodeID node) {
        // node has been excluded in mis -> neighboring vertices are interesting for next round of reduction
        for (auto neighbor : global_status.graph.neigh2(node)) {
                add_next_level_node(neighbor);
        }
}

void weighted_reduce_algorithm::add_next_level_neighborhood(const std::vector<NodeID>& nodes) {
        for (auto node : nodes) {
                add_next_level_neighborhood(node);
        }
}

}  // namespace red2pack
