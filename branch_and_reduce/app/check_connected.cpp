//
// Created by Jannick Borowitz on 07.07.23.
//
#include <iostream>

#include "graph_access.h"
#include "graph_io.h"
#include "tools/m2s_log.h"

int main(int argc, char **argv) {

        if (argc < 2) {
                std::cout << "Usage: ./check_connected <graph>";
                exit(1);
        }
        std::string graph_filepath(argv[1]);
        std::cout << "Checking: " << graph_filepath << std::endl;

        // read graph
        graph_access graph;
        graph_io::readGraphWeighted(graph, graph_filepath);

        std::queue<NodeID> expand;
        fast_set touched(graph.number_of_nodes());

        if (graph.number_of_nodes() == 0) {
                std::cout << graph_filepath << " is connected!" << std::endl;
                return 0;
        }

        if (graph.number_of_edges()/2 < graph.number_of_nodes()-1) {
                std::cout << graph_filepath << " is not connected!" << std::endl;
                return 0;
        }

        expand.push(0);
        touched.add(0);
        NodeID count_touched = 1;

        while(!expand.empty()) {
                auto current = expand.front();
                expand.pop();

                forall_out_edges (graph, e, current) {
                      auto target = graph.getEdgeTarget(e);
                      if (!touched.get(target)) {
                              expand.push(target);
                              touched.add(target);
                              count_touched += 1;
                      }
                }
                endfor
        }

        if (graph.number_of_nodes() != count_touched) {
                std::cout << graph_filepath << " is not connected!" << std::endl;
                return 0;
        }

        std::cout << graph_filepath << " is connected!" << std::endl;

        return 0;
}