#include "../benchmark_red2pack_solver.h"
#include "../m2s_parse_parameters.h"
#include "../m2s_configuration_m2s.h"

#include <red2pack-onlinemis/algorithms/heuristic.h>
#include <onlinemis/configuration_mis.h>

#include <iostream>
#include <fstream>

int main(int argc, char **argv) {
        using namespace red2pack;
        M2SConfig m2s_config;
        std::string graph_filepath;

        // read arguments
        m2s_configuration_m2s configurator_m2s;
        configurator_m2s.standard_unweighted(m2s_config);
        auto ret_code = cli::parse_parameters<cli::unweighted_cli>(argc, argv, m2s_config, graph_filepath);
        if (ret_code) return 0;

        // supress output
        std::streambuf *backup = std::cout.rdbuf();
        std::ofstream ofs;
        ofs.open("/dev/null");
        if (m2s_config.silent) {
                std::cout.rdbuf(ofs.rdbuf());
        }

        onlinemis::MISConfig mis_cfg_onlinemis;
        onlinemis::configuration_mis configurator_omis;
        configurator_omis.standard(mis_cfg_onlinemis);
        mis_cfg_onlinemis.seed = m2s_config.seed;
        mis_cfg_onlinemis.console_log = false;
        mis_cfg_onlinemis.print_log = false;
        mis_cfg_onlinemis.check_sorted = false;
        mis_cfg_onlinemis.start_greedy_adaptive = false;
        mis_cfg_onlinemis.write_graph = false;

        // benchmark
        ret_code = benchmark_red2pack_solver<heuristic>(graph_filepath, m2s_config, mis_cfg_onlinemis);

        if (m2s_config.silent) {
                std::cout.rdbuf(backup);
        }

        return ret_code;
}
