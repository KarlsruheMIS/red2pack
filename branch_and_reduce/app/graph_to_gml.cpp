#include <iostream>

#include "graph_access.h"
#include "graph_io.h"
#include "tools/m2s_log.h"

int main(int argc, char **argv) {

        if (argc < 3) {
                std::cout << "Usage: ./graph_to_gml <graph> <graph_gml>";
                exit(1);
        }
        std::string graph_filepath(argv[1]);
        std::string graph_gml_filepath(argv[2]);
        std::cout << "Translating: " << graph_filepath << std::endl;
        std::cout << "Asserting graph is undirected!" << std::endl;
        std::cout << "Output: " << graph_gml_filepath << std::endl;

        // read graph
        graph_access graph;
        graph_io::readGraphWeighted(graph, graph_filepath);

        // write gml graph
        std::ofstream f(graph_gml_filepath.c_str());

        f << "graph" << std::endl;
        f << "[" << std::endl;

        for(NodeID node = 0; node < graph.number_of_nodes(); node++) {
                f << "\tnode [" << std::endl;
                f << "\t\tid " << node << std::endl;
                f << "\t\tlabel \"" << node << "\"" << std::endl;
                f << "\t]" << std::endl;
        }

        for(NodeID node = 0; node < graph.number_of_nodes(); node++) {
                forall_out_edges(graph, e, node) {
                        auto target = graph.getEdgeTarget(e);
                        if (node < target) {
                                f << "\tedge [" << std::endl;
                                f << "\t\tsource " << node << std::endl;
                                f << "\t\ttarget " << target << std::endl;
                                f << "\t]" << std::endl;
                        }
                }
                endfor
        }

        f << "]" << std::endl;
        f.close();
        std::cout << "Written " << graph_gml_filepath << std::endl;
        return 0;
}