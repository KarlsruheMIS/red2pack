#include <red2pack-scip-ilp/algorithms/weighted_rnt_scip_ilp.h>

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
        auto ret_code = cli::parse_parameters<cli::weighted_solver_cli>(argc, argv, m2s_config, graph_filepath);
        if (ret_code) return 0;

        // supress output
        std::streambuf* backup = std::cout.rdbuf();
        std::ofstream ofs;
        ofs.open("/dev/null");
        if (m2s_config.silent) {
                std::cout.rdbuf(ofs.rdbuf());
        }

        // benchmark
        ret_code = benchmark_red2pack_solver<weighted_rnt_scip_ilp>(graph_filepath, m2s_config);

        if (m2s_config.silent) {
                std::cout.rdbuf(backup);
        }

        return ret_code;
}
