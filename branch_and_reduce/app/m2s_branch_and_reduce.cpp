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

        two_packing_set::M2SConfig m2s_config;
        MISConfig mis_config;
        std::string graph_filepath;

        // read arguments
        auto ret_code = parse_parameters(argc, argv, m2s_config, mis_config, graph_filepath);
        if (ret_code) return 0;
        m2s_log::instance()->set_config(m2s_config);

        // read graph
        two_packing_set::m2s_graph_access graph;
        two_packing_set::graph_io::readGraphWeighted(graph, graph_filepath);
        m2s_log::instance()->set_graph(graph);

        m2s_log::instance()->print_graph();
        m2s_log::instance()->print_config();


        /*===============================BENCHMARK START===============================*/
        m2s_log::instance()->restart_timer();
        // COMPUTATION START
        // init branch and reduce solver
        two_packing_set::branch_and_reduce solver(graph, m2s_config, mis_config);

        // solve graph
        solver.run();
        // COMPUTATION END
        m2s_log::instance()->set_best_size(m2s_config, solver.get_solution_size());
        /*===============================BENCHMARK END===============================*/

        // PRINT RESULTS
        m2s_log::instance()->print_results();
        return 0;
}