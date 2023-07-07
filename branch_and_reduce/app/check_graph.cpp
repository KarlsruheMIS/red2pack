//
// Created by Jannick Borowitz on 07.07.23.
//
#include <chrono>
#include <iostream>

#include "algorithms/branch_and_reduce.h"
#include "data_structures/m2s_graph_access.h"
#include "m2s_config.h"
#include "m2s_parse_parameters.h"
#include "mis_config.h"
#include "tools/graph_io.h"
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
        std::string graph_filepath(argv[0]);

        // read graph
        two_packing_set::m2s_graph_access graph;
        two_packing_set::graph_io::readGraphWeighted(graph, graph_filepath);

        graph.construct_2neighborhood();

        // check transform amount 2-edges
        EdgeID edges_count = 0;

        for (size_t i = 0; i < graph.number_of_nodes(); i++) {
                EdgeID edges = graph.getNodeDegree(i) + graph.get2Degree(i);
                if (edges + edges_count < edges_count) {
                        std::cout << "Graph can not be transformed to graph_access because close 2-Neighborhood is too large. Only 32-bit unsigned integers are supported."
                                  << std::endl;
                        exit(1);
                }
                edges_count += edges;
        }

        return 0;
}