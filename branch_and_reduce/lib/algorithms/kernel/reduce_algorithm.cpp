#include "reduce_algorithm.h"

#include <algorithm>
#include <chrono>
#include <numeric>

#include "m2s_config.h"

namespace two_packing_set {

#include <queue>

struct deg_node {
       public:
        size_t v;
        size_t deg;

        bool operator<(const deg_node& rhs) const { return deg < rhs.deg; }
};
/* reduce_algorithm::reduce_algorithm(graph_access& G, bool called_from_fold)  */
reduce_algorithm::reduce_algorithm(m2s_graph_access& G, const M2SConfig& mis_config)  //, bool called_from_fold)
    : config(mis_config),
      global_status(G),
      set_1(G.number_of_nodes()),
      set_2(G.number_of_nodes()),
      double_set(G.number_of_nodes() * 2),
      buffers(2, sized_vector<NodeID>(G.number_of_nodes())) {
        if (config.reduction_style2 == M2SConfig::Reduction_Style2::extended) {
                global_status.reductions2 =
                    make_2reduction_vector<deg_one_2reduction_e, cycle2_reduction_e, twin2_reduction_e,
                                           domination2_reduction_e, clique2_reduction_e>(global_status.n);
        } else if (config.reduction_style2 == M2SConfig::Reduction_Style2::compact) {
                global_status.reductions2 =
                    make_2reduction_vector<clique2_reduction_e, domination2_reduction_e>(global_status.n);
        } else {  //  initial
                global_status.reductions2 =
                    make_2reduction_vector<cycle2_reduction, twin2_reduction, domination2_reduction, clique2_reduction>(
                        global_status.n);
        }
        reduction_map.resize(m2ps_REDUCTION_NUM);
        for (size_t i = 0; i < global_status.reductions2.size(); i++) {
                reduction_map[global_status.reductions2[i]->get_reduction_type()] = i;  // Hm....
        }
}

void reduce_algorithm::set_imprecise(NodeID node, pack_status mpack_status) {
        if (mpack_status == pack_status::included) {
                status.node_status[node] = mpack_status;
                status.remaining_nodes--;
                status.pack_weight += status.weights[node];
                status.graph.hide_node_imprecise(node);

                for (auto neighbor : status.graph[node]) {
                        status.node_status[neighbor] = pack_status::excluded;
                        status.remaining_nodes--;
                        status.graph.hide_node_imprecise(neighbor);
                }

                for (auto neighbor : status.graph.get2neighbor_list(node)) {
                        status.node_status[neighbor] = pack_status::unsafe;
                        /* status.remaining_nodes--; */
                }
        } else {
                status.node_status[node] = mpack_status;
                status.remaining_nodes--;
                status.graph.hide_node_imprecise(node);
        }
}

void reduce_algorithm::set(NodeID node, pack_status mpack_status) {
        if (mpack_status == pack_status::included) {
                status.node_status[node] = mpack_status;
                status.remaining_nodes--;
                status.graph.hide_node_imprecise(node);

                for (auto neighbor : status.graph[node]) {
                        set(neighbor, excluded);
                }

                for (auto neighbor : status.graph.get2neighbor_list(node)) {
                        set(neighbor, excluded);
                }

        } else {  // exclude
                status.node_status[node] = mpack_status;
                status.remaining_nodes--;
                status.graph.hide_node_imprecise(node);
        }
}

size_t reduce_algorithm::deg(NodeID node) const { return status.graph[node].size(); }

size_t reduce_algorithm::two_deg(NodeID node) { return status.graph.get2neighbor_list(node).size(); }

void reduce_algorithm::add_next_level_node(NodeID node) {
        // mark node for next round of status.reductions...
        for (auto& reduction : status.reductions2) {
                if (reduction->has_run) {
                        reduction->marker.add(node);  // mark: Probably because something has changed?
                }
        }
}

void reduce_algorithm::add_next_level_neighborhood(NodeID node) {
        // node has been excluded in M2PS -> neighbouring vertices are interesting for the next round of reduction.
        for (auto neighbor : status.graph[node]) {
                add_next_level_node(neighbor);
        }

        // same for the neighbors with distance two...
        for (auto neighbor2 : status.graph.get2neighbor_list(node)) {
                add_next_level_node(neighbor2);
        }
}

// For this function no adjustement is needed.
void reduce_algorithm::add_next_level_neighborhood(const std::vector<NodeID>& nodes) {
        for (auto node : nodes) {
                add_next_level_neighborhood(node);
        }
}

void reduce_algorithm::init_reduction_step() {
        if (!status.reductions2[active_reduction_index]->has_run) {
                status.reductions2[active_reduction_index]->marker.fill_current_ascending(status.n);
                status.reductions2[active_reduction_index]->marker.clear_next();
                status.reductions2[active_reduction_index]->has_run = true;
        } else {
                status.reductions2[active_reduction_index]->marker.get_next();
        }
}

void reduce_algorithm::initial_reduce() {
        /* 	std::swap(global_reduction_map, local_reduction_map);  */
        status = std::move(global_status);
        reduce_graph_internal();

        global_status = std::move(status);
        /* 	std::swap(global_reduction_map, local_reduction_map);  */
}

void reduce_algorithm::reduce_graph_internal() {
        bool progress;

        do {
                progress = false;

                for (auto& reduction : status.reductions2) {
                        active_reduction_index = reduction_map[reduction->get_reduction_type()];

                        init_reduction_step();
                        progress = reduction->reduce(this);

                        if (progress) break;

                        active_reduction_index++;
                }
        } while (progress && config.time_limit > t.elapsed());
        if (config.time_limit > t.elapsed()) std::cout << "\%timeout" << std::endl;
}

void reduce_algorithm::run_reductions() {
        t.restart();
        initial_reduce();
}

// gives us the actual two-packing set so far
/* std::vector<NodeID> reduce_algorithm::get_status() { */
/* } */

void reduce_algorithm::get_solution(std::vector<bool>& solution_vec) {
        // Now check whether we include nodes that are in conflict...
        for (size_t i = 0; i < solution_vec.size(); i++) {
                if (global_status.node_status[i] == 1) {
                        solution_vec[i] = true;
                }
        }
}

}  // namespace two_packing_set