#ifdef RED2PACK_ENABLE_MALLOC
#include <malloc_count.h>
#endif
#include <red2pack/algorithms/kernel/weighted_reduce_algorithm.h>

#include <fstream>
#include <iostream>

#include "../benchmark_red2pack_solver.h"
#include "../m2s_configuration_m2s.h"
#include "../m2s_parse_parameters.h"

int main(int argc, char** argv) {
        using namespace red2pack;
        M2SConfig m2s_config;
        std::string graph_filepath;

        // read arguments
        m2s_configuration_m2s configurator_m2s;
        configurator_m2s.standard_weighted(m2s_config);
        auto ret_code = cli::parse_parameters<cli::weighted_reducer_cli>(argc, argv, m2s_config, graph_filepath);
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
        const std::size_t space_before = malloc_count_current();
#endif
        auto graph = std::make_unique<m2s_graph_access>();
        m2s_graph_io::readGraphWeighted(*graph, graph_filepath);
        m2s_log::instance()->set_graph(*graph);

        m2s_log::instance()->print_graph();
        m2s_log::instance()->print_config();

        m2s_graph_access reduced_graph;
        std::vector<NodeID> reduced_node_id;
        std::vector<NodeID> former_node_id;

        /*===============================BENCHMARK START===============================*/
        /*
         * The benchmark consist for three pars
         * 1. initializing the reduce algorithm (allocations and so forth)
         * 2. running the reductions
         * 3. obtaining the exact kernel
         */
        m2s_log::instance()->restart_timer();
        // COMPUTATION START

        if (!m2s_config.on_demand_two_neighborhood) {
#ifdef RED2PACK_ENABLE_MALLOC
                const std::size_t space_middle = malloc_count_current();
#endif
                graph->construct_2neighborhood();
#ifdef RED2PACK_ENABLE_MALLOC
                const std::size_t space_after = malloc_count_current();

                std::cout << "space consumption transformed graph [Bytes]: "
                          << space_after - space_middle + space_before << std::endl;
#endif
                m2s_log::instance()->set_best_size(0);  // if the reducer is not used
        }

        // init branch and reduce solver
        weighted_reduce_algorithm reducer(*graph, m2s_config);

        if (m2s_config.use_weighted_reductions()) {
                reducer.run_reductions();
        }

        reducer.get_exact_kernel(reduced_graph, reduced_node_id, former_node_id);
        m2s_log::instance()->print_reduction(reducer.get_solution_weight(), reduced_graph.number_of_nodes(),
                                             reduced_graph.number_of_edges() / 2, reduced_graph.number_of_links() / 2);

        m2s_log::instance()->set_best_size(reducer.get_solution_weight());
        m2s_log::instance()->finish_solving();
        /*===============================BENCHMARK END===============================*/

        // PRINT RESULTS
        m2s_log::instance()->print_results();
        if (m2s_config.use_weighted_reductions()) {
                m2s_log::instance()->print_reduced_nodes_mw2ps();
        }
        if (m2s_config.write_transformed) {
                std::cout << "Printing reduced-and-transformed graph" << std::endl;
                m2s_graph_io::writeGraphWeighted(reduced_graph, m2s_config.transformed_graph_filename,
                                                 " reduced-and-transformed graph of " + m2s_config.graph_filename +
                                                     ", offset: " + std::to_string(reducer.get_solution_weight()));
        }
        if (static_cast<std::size_t>(reduced_graph.number_of_edges()) +
                static_cast<std::size_t>(reduced_graph.number_of_links()) >
            std::numeric_limits<red2pack::EdgeID>::max()) {
                std::cout << "WARNING: Cannot represent all edges of reduced graph with EdgeID!" << std::endl;
        }

        if (m2s_config.silent) {
                std::cout.rdbuf(backup);
        }

        return 0;
}
