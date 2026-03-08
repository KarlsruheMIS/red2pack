#ifdef RED2PACK_ENABLE_MALLOC
#include <malloc_count.h>
#endif
#include <red2pack/algorithms/kernel/weighted_reduce_algorithm.h>
#include <red2pack/data_structure/fast_set.h>
#include <red2pack/data_structure/m2s_graph_access.h>
#include <red2pack/m2s_config.h>
#include <red2pack/red2pack_def.h>
#include <red2pack/tools/m2s_graph_io.h>
#include <red2pack/tools/m2s_log.h>

#include <fstream>
#include <iostream>

#include "../m2s_configuration_m2s.h"
#include "../m2s_parse_parameters.h"

int main(int argc, char** argv) {
        using namespace red2pack;
        M2SConfig m2s_config;
        std::string graph_filepath;

        // read arguments
        m2s_configuration_m2s configurator_m2s;
        configurator_m2s.standard_weighted(m2s_config);
        auto ret_code = cli::parse_parameters<cli::base_cli>(argc, argv, m2s_config, graph_filepath);
        if (ret_code) return 0;

        // supress output
        std::streambuf* backup = std::cout.rdbuf();
        std::ofstream ofs;
        ofs.open("/dev/null");
        if (m2s_config.silent) {
                std::cout.rdbuf(ofs.rdbuf());
        }
        // benchmark
        // SETUP
        m2s_log::instance()->restart_total_timer();

        m2s_log::instance()->set_config(m2s_config);

        // read graph
#ifdef RED2PACK_ENABLE_MALLOC
        malloc_count_reset_peak();
#endif
        m2s_graph_access graph;
        m2s_graph_io::readGraphWeighted(graph, graph_filepath);
        m2s_log::instance()->set_graph(graph);

        m2s_log::instance()->print_graph();
        m2s_log::instance()->print_config();
        /*===============================BENCHMARK START===============================*/
        /*
         * The benchmark consists of only building the m2s_graph_access' 2-neighborhood
         */
        m2s_log::instance()->restart_timer();
        // COMPUTATION START

        graph.construct_2neighborhood();
        m2s_log::instance()->print_reduction(0, graph.number_of_nodes(), graph.number_of_edges() / 2,
                                             graph.number_of_links() / 2);

        m2s_log::instance()->set_best_size(0);  // if the reducer is not used
        m2s_log::instance()->finish_solving();
        /*===============================BENCHMARK END===============================*/
        // PRINT RESULTS
        m2s_log::instance()->print_results();
        if (m2s_config.write_transformed) {
                std::cout << "Printing reduced-and-transformed graph" << std::endl;
                m2s_graph_io::writeGraphWeighted(
                    graph, m2s_config.transformed_graph_filename,
                    " reduced-and-transformed graph of " + m2s_config.graph_filename + ", offset: 0");
        }
        if (static_cast<std::size_t>(graph.number_of_edges()) + static_cast<std::size_t>(graph.number_of_links()) >
            std::numeric_limits<red2pack::EdgeID>::max()) {
                std::cout << "WARNING: Cannot represent all edges of reduced graph with EdgeID!" << std::endl;
        }

        if (m2s_config.silent) {
                std::cout.rdbuf(backup);
        }

        return 0;
}
