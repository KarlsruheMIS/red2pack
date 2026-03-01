#include "../benchmark_red2pack_solver.h"
#include "../m2s_parse_parameters.h"
#include "../m2s_configuration_m2s.h"

#include <red2pack-mmwis/algorithms/weighted_rnt_mmwis.h>
#include <mmwis_config.h>
#include <configuration_mis.h>

#include <fstream>
#include <iostream>

int main(int argc, char **argv) {
        using namespace red2pack;
        M2SConfig m2s_config;
        mmwis::MISConfig mmwis_config;
        mmwis::configuration_mis cfg;
        cfg.mmwiss(mmwis_config);
        std::string graph_filepath;

        // read arguments
        m2s_configuration_m2s configurator_m2s;
        configurator_m2s.standard_weighted(m2s_config);
        auto ret_code = cli::parse_parameters<cli::weighted_solver_cli>(argc, argv, m2s_config, graph_filepath);
        if (ret_code) return 0;

        mmwis_config.time_limit = m2s_config.time_limit;
        mmwis_config.ils_time_limit = 0.01 * m2s_config.time_limit;
        mmwis_config.seed = m2s_config.seed;

        // supress output
        std::streambuf *backup = std::cout.rdbuf();
        std::ofstream ofs;
        ofs.open("/dev/null");
        if (m2s_config.silent) {
                std::cout.rdbuf(ofs.rdbuf());
        }

        // benchmark
        ret_code = benchmark_red2pack_solver<weighted_rnt_mmwis>(graph_filepath, m2s_config, mmwis_config);

        if (m2s_config.silent) {
                std::cout.rdbuf(backup);
        }

        return ret_code;
}
