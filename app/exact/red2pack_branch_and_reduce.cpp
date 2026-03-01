#include "../benchmark_red2pack_solver.h"
#include "../m2s_parse_parameters.h"
#include "../m2s_configuration_mis.h"
#include "../m2s_configuration_m2s.h"

#include <mis_config.h>
#include <red2pack-kamis-wmis/algorithms/branch_and_reduce.h>

#include <fstream>
#include <iostream>

int main(int argc, char** argv) {
        using namespace red2pack;
        M2SConfig m2s_config;
        MISConfig mis_config;
        m2s_configuration_mis cfg;
        cfg.standard(mis_config);
        std::string graph_filepath;

        // read arguments
        m2s_configuration_m2s configurator_m2s;
        configurator_m2s.standard_unweighted(m2s_config);
        auto ret_code = cli::parse_parameters<cli::unweighted_cli>(argc, argv, m2s_config, graph_filepath);
        if (ret_code) return 0;

        mis_config.time_limit = m2s_config.time_limit;
        mis_config.seed = m2s_config.seed;

        // supress output
        std::streambuf* backup = std::cout.rdbuf();
        std::ofstream ofs;
        ofs.open("/dev/null");
        if (m2s_config.silent) {
                std::cout.rdbuf(ofs.rdbuf());
        }

        // benchmark
        ret_code = benchmark_red2pack_solver<branch_and_reduce>(graph_filepath, m2s_config, mis_config);

        if (m2s_config.silent) {
                std::cout.rdbuf(backup);
        }

        return ret_code;
}
