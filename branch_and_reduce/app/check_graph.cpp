#include <chrono>
#include <iostream>

#include "algorithms/branch_and_reduce.h"
#include "data_structures/m2s_graph_access.h"
#include "m2s_config.h"
#include "m2s_parse_parameters.h"
#include "mis_config.h"
#include "tools/m2s_graph_io.h"
#include "tools/m2s_log.h"

int main(int argc, char **argv) {
        using namespace two_packing_set;
        // SETUP
        m2s_log::instance()->restart_total_timer();
        m2s_log::instance()->print_title();

        if (argc < 2) {
                std::cout << "Usage: ./check_graph <graph>";
                exit(1);
        }
        std::string graph_filepath(argv[1]);
        std::cout << "Checking: " << graph_filepath << std::endl;

        // read graph
        m2s_graph_access graph;
        m2s_graph_io::readGraphWeighted(graph, graph_filepath);
        std::cout << "Nodes: " << graph.number_of_nodes() << std::endl;
        std::cout << "Edges: " << graph.number_of_edges()/2 << std::endl;

        // check transform amount 2-edges
        EdgeID edges_count = 0;

        for (size_t i = 0; i < graph.number_of_nodes(); i++) {
                EdgeID edges = graph.getNodeDegree(i);
                if (edges_count+edges < edges_count) {
                        std::cout << "Graph can not be transformed to graph_access because close 2-Neighborhood is too large. Only 32-bit unsigned integers are supported."
                                  << std::endl;
                        exit(1);
                }
                edges_count += edges;
        }
        fast_set touched(graph.number_of_nodes());
        for (size_t i = 0; i < graph.number_of_nodes(); i++) {
                for (int k = graph.get_first_edge(i); k < graph.get_first_invalid_edge(i); k++) {
                        auto source = graph.getEdgeTarget(k);
                        for (EdgeID j = graph.get_first_edge(source);
                             j < graph.get_first_invalid_edge(source); j++) {
                                NodeID target = graph.getEdgeTarget(j);
                                if (!touched.get(target)) {
                                        touched.add(target);
                                        if(edges_count + 1 < edges_count) {
                                                std::cout << "Graph can not be transformed to graph_access because close 2-Neighborhood is too large. Only 32-bit unsigned integers are supported."
                                                          << std::endl;
                                                exit(1);
                                        }
                                        edges_count++;
                                }
                        }
                }
                touched.clear();
        }
        std::cout << "Transformed Edges: " << edges_count/2 << std::endl;

        return 0;
}