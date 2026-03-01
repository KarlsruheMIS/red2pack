//
// Created by Jannick Borowitz on 14.01.24.
//

#ifndef RED2PACK_CONVERT_GRAPH_H
#define RED2PACK_CONVERT_GRAPH_H

#include <algorithm>
#include <red2pack/data_structure/m2s_graph_access.h>
namespace red2pack {

template<typename graph_access>
inline void convert(m2s_graph_access& input_graph, graph_access& graph) {
        graph.start_construction(input_graph.number_of_nodes(),
                                 input_graph.number_of_edges() + input_graph.number_of_links());
        for (size_t i = 0; i < input_graph.number_of_nodes(); i++) {
                graph.new_node();
                graph.setNodeWeight(i, input_graph.getNodeWeight(i));
                for (size_t j = input_graph.get_first_edge(i); j < input_graph.get_first_invalid_edge(i); j++) {
                        EdgeID e_bar = graph.new_edge(i, input_graph.getEdgeTarget(j));
                        graph.setEdgeWeight(e_bar, 1);
                }
                for (size_t j = input_graph.get_first_link(i); j < input_graph.get_first_invalid_link(i); j++) {
                        EdgeID e_bar = graph.new_edge(i, input_graph.getLinkTarget(j));
                        graph.setEdgeWeight(e_bar, 1);
                }
        }
        graph.finish_construction();
}

template<typename graph_access>
inline void convert_sorted(m2s_graph_access& input_graph, graph_access& graph) {
        graph.start_construction(input_graph.number_of_nodes(),
                                 input_graph.number_of_edges() + input_graph.number_of_links());
        std::vector<NodeID> adjA(input_graph.number_of_nodes(), 0);
        for (size_t i = 0; i < input_graph.number_of_nodes(); i++) {
                EdgeID local_count_edges = 0;
                graph.new_node();
                graph.setNodeWeight(i, input_graph.getNodeWeight(i));
                for (size_t j = input_graph.get_first_edge(i); j < input_graph.get_first_invalid_edge(i); j++) {
                        adjA[local_count_edges++] = input_graph.getEdgeTarget(j);
                }
                for (size_t j = input_graph.get_first_link(i); j < input_graph.get_first_invalid_link(i); j++) {
                        adjA[local_count_edges++] = input_graph.getLinkTarget(j);
                }

                auto end = adjA.begin();
                std::advance(end, local_count_edges);
                std::sort(adjA.begin(), end);

                for (NodeID idx = 0; idx < local_count_edges; idx++) {
                        EdgeID e_bar = graph.new_edge(i, adjA[idx]);
                        graph.setEdgeWeight(e_bar, 1);
                }
        }
        graph.finish_construction();
}

}  // namespace red2pack

#endif  // RED2PACK_CONVERT_GRAPH_H
