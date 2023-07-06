#include "reduce_algorithm.h"

#include <numeric>
#include <queue>
#include <utility>

#include "m2s_config.h"

namespace two_packing_set {

struct deg_node {
       public:
        size_t v;
        size_t deg;

        bool operator<(const deg_node& rhs) const { return deg < rhs.deg; }
};
/* reduce_algorithm::reduce_algorithm(graph_access& G, bool called_from_fold)  */
reduce_algorithm::reduce_algorithm(m2s_graph_access& G, const M2SConfig &m2s_config)  //, bool called_from_fold)
    : config(m2s_config),
      global_status(G, !m2s_config.on_demand_two_neighborhood),
      set_1(G.number_of_nodes()),
      set_2(G.number_of_nodes()),
      double_set(G.number_of_nodes() * 2),
      buffers(2, sized_vector<NodeID>(G.number_of_nodes())) {
        if (config.reduction_style2 == M2SConfig::Reduction_Style2::extended) {
                global_status.reductions2 =
                    make_2reduction_vector<deg_one_2reduction_e, deg_two_2reduction_e, twin2_reduction_e,
                                           fast_domination2_reduction_e, domination2_reduction_e, clique2_reduction_e>(
                        global_status.n);

        } else if (config.reduction_style2 == M2SConfig::Reduction_Style2::compact) {
                global_status.reductions2 =
                    make_2reduction_vector<clique2_reduction_e, domination2_reduction_e>(global_status.n);
        }
        reduction_map.resize(m2ps_REDUCTION_NUM);
        for (size_t i = 0; i < global_status.reductions2.size(); i++) {
                reduction_map[global_status.reductions2[i]->get_reduction_type()] = i;  // Hm....
        }
}

/*void reduce_algorithm::set_imprecise(NodeID node, pack_status mpack_status) {
        if (mpack_status == pack_status::included) {
                global_status.node_status[node] = mpack_status;
                global_status.remaining_nodes--;
                global_status.pack_weight += global_status.weights[node];
                global_status.graph.hide_node_imprecise(node);

                for (auto neighbor : global_status.graph[node]) {
                        global_status.node_status[neighbor] = pack_status::excluded;
                        global_status.remaining_nodes--;
                        global_status.graph.hide_node_imprecise(neighbor);
                }

                for (auto neighbor : global_status.graph.get2neighbor_list(node)) {
                        global_status.node_status[neighbor] = pack_status::unsafe;
                        /* global_status.remaining_nodes--;
                }
        } else {
                global_status.node_status[node] = mpack_status;
                global_status.remaining_nodes--;
                global_status.graph.hide_node_imprecise(node);
        }
}*/

void reduce_algorithm::set(NodeID node, pack_status mpack_status) {
        if (mpack_status == pack_status::included) {
                global_status.node_status[node] = mpack_status;
                global_status.remaining_nodes--;
                global_status.pack_weight += global_status.weights[node];
                global_status.graph.hided_nodes[node] = true;
                //global_status.graph.hide_node_imprecise(node);

                for (auto neighbor : global_status.graph[node]) {
                        set(neighbor, excluded);
                        /*global_status.node_status[node] = pack_status::excluded;
                        global_status.remaining_nodes--;
                        global_status.graph.hided_nodes[node] = true;
                        for (auto two_neighbor : global_status.graph.get2neighbor_list(neighbor)) {
                                if(!global_status.graph.hided_nodes[two_neighbor]) {
                                        global_status.graph.hide_path(two_neighbor, neighbor);
                                }
                        }*/
                }
                for (auto neighbor : global_status.graph.get2neighbor_list(node)) {
                        set(neighbor, excluded);
                        /*global_status.node_status[neighbor] = pack_status::excluded;
                        global_status.remaining_nodes--;
                        for (auto neighbor2 : global_status.graph[neighbor]) {
                                global_status.graph.hide_edge(neighbor2, neighbor);
                        }
                        for (auto two_neighbor : global_status.graph.get2neighbor_list(neighbor)) {
                                global_status.graph.hide_path(two_neighbor, neighbor);
                        }*/
                }

        } else {  // exclude
                global_status.node_status[node] = mpack_status;
                global_status.remaining_nodes--;
                global_status.graph.hide_node_imprecise(node);
        }
}

size_t reduce_algorithm::deg(NodeID node) const { return global_status.graph[node].size(); }

size_t reduce_algorithm::two_deg(NodeID node) { return global_status.graph.get2neighbor_list(node).size(); }

void reduce_algorithm::init_reduction_step() {
        if (!global_status.reductions2[active_reduction_index]->has_run) {
                global_status.reductions2[active_reduction_index]->marker.fill_current_ascending(global_status.n);
                global_status.reductions2[active_reduction_index]->marker.clear_next();
                global_status.reductions2[active_reduction_index]->has_run = true;
        } else {
                global_status.reductions2[active_reduction_index]->marker.get_next();
        }
}

void reduce_algorithm::reduce_graph_internal() {
        bool progress;

        do {
                progress = false;

                for (auto& reduction : global_status.reductions2) {
                        active_reduction_index = reduction_map[reduction->get_reduction_type()];

                        init_reduction_step();
                        progress = reduction->reduce(this);

                        if (progress) break;

                        active_reduction_index++;
                }
        } while (progress && config.time_limit > t.elapsed());
        if (config.time_limit < t.elapsed()) std::cout << "%timeout" << std::endl;
}

void reduce_algorithm::run_reductions() {
        t.restart();
        reduce_graph_internal();
}

// gives us the actual two-packing set so far
void reduce_algorithm::get_solution(std::vector<bool>& solution_vec) {
        // Now check whether we include nodes that are in conflict...
        for (size_t i = 0; i < solution_vec.size(); i++) {
                if (global_status.node_status[i] == 1) {
                        solution_vec[i] = true;
                }
        }
}

bool reduce_algorithm::is_adj(NodeID first, NodeID second) {
        for (auto& target : global_status.graph[second]) {
                if (target == first) {
                        return true;
                }
        }
        return false;
}

bool reduce_algorithm::is_two_adj(NodeID first, NodeID second) {
        for (auto& target : global_status.graph.get2neighbor_list(second)) {
                if (target == first) {
                        return true;
                }
        }
        return false;
}

bool reduce_algorithm::reduce_deg_leq_one(NodeID node) {
        if (deg(node) == 0) {
                if (two_deg(node) <= 1) {
                        // Degree Zero Reduction
                        return true;
                } else if (two_deg(node) == 2) {
                        auto& neighbor1 = global_status.graph.get2neighbor_list(node)[0];
                        auto& neighbor2 = global_status.graph.get2neighbor_list(node)[1];
                        if (is_adj(neighbor1, neighbor2) || is_two_adj(neighbor1, neighbor2)) {
                                // Degree Zero Triangle Reduction
                                return true;
                        }
                }
        } else if (deg(node) == 1) {
                if (two_deg(node) < deg(global_status.graph[node][0])) {
                        // Degree 1 Reduction
                        return true;
                }
        }
        return false;
}

}  // namespace two_packing_set