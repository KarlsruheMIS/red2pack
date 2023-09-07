#include "heuristic.h"

#include "online_ils.h"
bool red2pack::heuristic::solve_mis(graph_access& condensed_graph) {

        using namespace onlinemis;
        mis_cfg.time_limit = get_mis_solve_time();

        online_ils solver;
        solver.perform_ils(mis_cfg, condensed_graph, mis_cfg.ils_iterations);

        return false;
}

void red2pack::heuristic::build_condensed_graph_from_access(graph_access& condensed_graph,
                                                                    m2s_graph_access& graph, NodeID nodes,
                                                                    EdgeID edges) {
        condensed_graph.start_construction(nodes, edges);
        std::vector<NodeID> adjA(nodes, 0);
        for (size_t i = 0; i < graph.number_of_nodes(); i++) {
                EdgeID local_count_edges = 0;
                condensed_graph.new_node();
                condensed_graph.setNodeWeight(reduced_node_id[i], 1);
                for (size_t j = graph.get_first_edge(i); j < graph.get_first_invalid_edge(i); j++) {
                        adjA[local_count_edges++] = reduced_node_id[graph.getEdgeTarget(j)];
                }
                for (size_t j = graph.get_first_edge2(i); j < graph.get_first_invalid_edge2(i); j++) {
                        adjA[local_count_edges++] = reduced_node_id[graph.getEdgeTarget2(j)];
                }

                auto end = adjA.begin();
                std::advance(end, local_count_edges);
                std::sort(adjA.begin(), end);

                for (NodeID idx = 0; idx < local_count_edges; idx++) {
                        EdgeID e_bar = condensed_graph.new_edge(reduced_node_id[i], adjA[idx]);
                        condensed_graph.setEdgeWeight(e_bar, 1);
                }
        }
        condensed_graph.finish_construction();
}

void red2pack::heuristic::build_condensed_graph_from_status(
    graph_access& condensed_graph, m2s_dynamic_graph& reduced_graph,
    std::vector<reduce_algorithm::two_pack_status>& reduced_node_status, NodeID nodes, EdgeID edges) {
        condensed_graph.start_construction(nodes, edges);
        std::vector<NodeID> adjA(nodes, 0);

        for (size_t i = 0; i < reduced_graph.size(); i++) {
                if (reduced_node_status[i] == reduce_algorithm::two_pack_status::not_set) {
                        EdgeID local_count_edges = 0;

                        condensed_graph.new_node();
                        condensed_graph.setNodeWeight(reduced_node_id[i], 1);
                        for (size_t j = 0; j < reduced_graph[i].size(); j++) {
                                if (reduced_node_status[reduced_graph[i][j]] ==
                                    reduce_algorithm::two_pack_status::not_set) {
                                        adjA[local_count_edges++] = reduced_node_id[reduced_graph[i][j]];
                                }
                        }
                        for (size_t j = 0; j < reduced_graph.get2neighbor_list(i).size(); j++) {
                                if (reduced_node_status[reduced_graph.get2neighbor_list(i)[j]] ==
                                    reduce_algorithm::two_pack_status::not_set) {
                                        adjA[local_count_edges++] =
                                            reduced_node_id[reduced_graph.get2neighbor_list(i)[j]];
                                }
                        }

                        auto end = adjA.begin();
                        std::advance(end, local_count_edges);
                        std::sort(adjA.begin(), end);

                        for (NodeID idx = 0; idx < local_count_edges; idx++) {
                                EdgeID e_bar = condensed_graph.new_edge(reduced_node_id[i], adjA[idx]);
                                condensed_graph.setEdgeWeight(e_bar, 1);
                        }
                }
        }
        condensed_graph.finish_construction();
}

void red2pack::heuristic::attach(std::unique_ptr<m2s_graph_access> G, red2pack::M2SConfig m2s_cfg, onlinemis::MISConfig mis_cfg) {
        reduce_and_transform::attach(std::move(G), std::move(m2s_cfg));
        this->mis_cfg = std::move(mis_cfg);
}
