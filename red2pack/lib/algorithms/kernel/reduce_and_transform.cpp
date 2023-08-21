#include "reduce_and_transform.h"

#include <chrono>
#include <utility>

#include "algorithms/kernel/reduce_algorithm.h"
#include "branch_and_reduce_algorithm.h"
#include "tools/m2s_log.h"

namespace red2pack {
reduce_and_transform::reduce_and_transform(m2s_graph_access& G, M2SConfig m2s_cfg)
    : m2s_cfg(std::move(m2s_cfg)),
      solution_status(G.number_of_nodes(), false),
      reduced_node_id(G.number_of_nodes(), G.number_of_nodes()),
      former_node_id(G.number_of_nodes()) {
        G.copy(this->graph);
}

bool reduce_and_transform::run_reduce_and_transform(graph_access& condensed_graph) {
        auto start_t = std::chrono::system_clock::now();

        // all reductions disabled?
        if (m2s_cfg.disable_fast_domination && m2s_cfg.disable_domination && m2s_cfg.disable_deg_two &&
            m2s_cfg.disable_deg_one && m2s_cfg.disable_twin && m2s_cfg.disable_clique) {
                transform_without_reductions(condensed_graph);
        } else {
                transform_with_reductions(condensed_graph);
        }

        auto stop_preprocessing = std::chrono::system_clock::now();

        if (condensed_graph.number_of_nodes() == 0) {
                return true;
        }

        return false;
}

void reduce_and_transform::transform_without_reductions(graph_access& condensed_graph) {
        auto start_t = std::chrono::system_clock::now();
        graph.construct_2neighborhood();
        auto stop_t = std::chrono::system_clock::now();
        std::chrono::duration<double> time = stop_t - start_t;
        double reduction_time = time.count();
        m2s_log::instance()->print_reduction(solution_offset_size, graph.number_of_nodes(), 0, reduction_time);

        for (size_t i = 0; i < graph.number_of_nodes(); i++) {
                reduced_node_id[i] = i;
                former_node_id[i] = i;
        }
        auto start_construction = std::chrono::system_clock::now();
        EdgeID count_edges = 0;
        for (size_t i = 0; i < graph.number_of_nodes(); i++) {
                count_edges += graph.getNodeDegree(i) + graph.get2Degree(i);
        }

        build_condensed_graph_from_access(condensed_graph, graph, graph.number_of_nodes(), count_edges);

        auto stop_construction = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_time_construction = stop_construction - start_construction;
        m2s_log::instance()->print_condensed_graph(condensed_graph.number_of_nodes(),
                                                   condensed_graph.number_of_edges() / 2,
                                                   elapsed_time_construction.count());
}

void reduce_and_transform::transform_with_reductions(graph_access& condensed_graph) {
        auto start_t = std::chrono::system_clock::now();
        // construct 2neighborhood
        if (!m2s_cfg.on_demand_two_neighborhood) {
                graph.construct_2neighborhood();
        }
        // run first reductions
        reduce_algorithm reducer(graph, m2s_cfg);
        reducer.run_reductions();

        // set solution status
        reducer.get_solution(solution_status);
        solution_offset_size = 0;
        for (auto&& status_of_node : solution_status) {
                if (status_of_node) solution_offset_size++;
        }

        // compute new graph access
        auto& status = reducer.global_status;
        auto n = status.graph.size();
        reduced_node_id.resize(n);

        NodeID kernel_nodes = 0;

        // Implemented optimization: we don't need to sort adjacency arrays to run KaMIS/wmis

        // compute node id mapping for graph and kernel
        for (size_t i = 0; i < status.graph.size(); i++) {
                if (status.node_status[i] == reduce_algorithm::pack_status::not_set) {
                        reduced_node_id[i] = kernel_nodes;
                        former_node_id[kernel_nodes] = i;
                        kernel_nodes++;
                }
        }

        auto stop_t = std::chrono::system_clock::now();
        std::chrono::duration<double> time = stop_t - start_t;
        double reduction_time = time.count();
        m2s_log::instance()->print_reduction(solution_offset_size, kernel_nodes, 0, reduction_time);

        auto start_construction = std::chrono::system_clock::now();
        // compute adjacency arrays
        if (kernel_nodes == 0) {
                condensed_graph.start_construction(0, 0);
                condensed_graph.finish_construction();
        } else {
                EdgeID count_edges = 0;
                for (size_t i = 0; i < status.graph.size(); i++) {
                        if (status.node_status[i] == reduce_algorithm::pack_status::not_set) {
                                for (size_t j = 0; j < status.graph[i].size(); j++) {
                                        if (status.node_status[status.graph[i][j]] ==
                                            reduce_algorithm::pack_status::not_set) {
                                                count_edges++;
                                        }
                                }
                                for (size_t j = 0; j < status.graph.get2neighbor_list(i).size(); j++) {
                                        if (status.node_status[status.graph.get2neighbor_list(i)[j]] ==
                                            reduce_algorithm::pack_status::not_set) {
                                                count_edges++;
                                        }
                                }
                        }
                }
                build_condensed_graph_from_status(condensed_graph, status.graph, status.node_status, kernel_nodes, count_edges);
        }
        auto stop_construction = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_time_construction = stop_construction - start_construction;
        m2s_log::instance()->print_condensed_graph(condensed_graph.number_of_nodes(),
                                                   condensed_graph.number_of_edges() / 2,
                                                   elapsed_time_construction.count());
}

void reduce_and_transform::build_condensed_graph_from_status(
    graph_access& condensed_graph, m2s_dynamic_graph& reduced_graph,
    std::vector<reduce_algorithm::pack_status>& reduced_node_status, NodeID nodes, EdgeID edges) {
        condensed_graph.start_construction(nodes, edges);
        for (size_t i = 0; i < reduced_graph.size(); i++) {
                if (reduced_node_status[i] == reduce_algorithm::pack_status::not_set) {
                        condensed_graph.new_node();
                        condensed_graph.setNodeWeight(reduced_node_id[i], 1);
                        for (size_t j = 0; j < reduced_graph[i].size(); j++) {
                                if (reduced_node_status[reduced_graph[i][j]] ==
                                    reduce_algorithm::pack_status::not_set) {
                                        EdgeID e_bar = condensed_graph.new_edge(reduced_node_id[i],
                                                                 reduced_node_id[reduced_graph[i][j]]);
                                        condensed_graph.setEdgeWeight(e_bar, 1);
                                }
                        }
                        for (size_t j = 0; j < reduced_graph.get2neighbor_list(i).size(); j++) {
                                if (reduced_node_status[reduced_graph.get2neighbor_list(i)[j]] ==
                                    reduce_algorithm::pack_status::not_set) {
                                        EdgeID e_bar = condensed_graph.new_edge(
                                            reduced_node_id[i], reduced_node_id[reduced_graph.get2neighbor_list(i)[j]]);
                                        condensed_graph.setEdgeWeight(e_bar, 1);
                                }
                        }
                }
        }
        condensed_graph.finish_construction();
}

void reduce_and_transform::build_condensed_graph_from_access(graph_access& condensed_graph, m2s_graph_access& graph,
                                                             NodeID nodes, EdgeID edges) {

        condensed_graph.start_construction(nodes, edges);
        for (size_t i = 0; i < graph.number_of_nodes(); i++) {
                condensed_graph.new_node();
                condensed_graph.setNodeWeight(reduced_node_id[i], 1);
                for (size_t j = graph.get_first_edge(i); j < graph.get_first_invalid_edge(i); j++) {
                        EdgeID e_bar = condensed_graph.new_edge(reduced_node_id[i], reduced_node_id[graph.getEdgeTarget(j)]);
                        condensed_graph.setEdgeWeight(e_bar, 1);
                }
                for (size_t j = graph.get_first_edge2(i); j < graph.get_first_invalid_edge2(i); j++) {
                        EdgeID e_bar = condensed_graph.new_edge(reduced_node_id[i], reduced_node_id[graph.getEdgeTarget2(j)]);
                        condensed_graph.setEdgeWeight(e_bar, 1);
                }
        }
        condensed_graph.finish_construction();

}


}  // namespace red2pack
