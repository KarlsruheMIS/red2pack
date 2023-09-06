#include <iostream>

#include "../benchmark_red2pack_solver.h"
#include "algorithms/heuristic.h"
#include "m2s_config.h"
#include "mis_config.h"
#include "../../extern/onlinemis/configuration_mis.h"

int main(int argc, char **argv) {
        using namespace red2pack;
        M2SConfig m2s_config;
        MISConfig mis_config;
        std::string graph_filepath;

        // read arguments
        auto ret_code = parse_parameters(argc, argv, m2s_config, mis_config, graph_filepath);
        if (ret_code) return 0;

        onlinemis::MISConfig mis_cfg_onlinemis;
        onlinemis::configuration_mis configurator_omis;
        configurator_omis.standard(mis_cfg_onlinemis);
        mis_cfg_onlinemis.seed = mis_config.seed;
        mis_cfg_onlinemis.console_log = false;
        mis_cfg_onlinemis.print_log = false;
        mis_cfg_onlinemis.check_sorted = false;
        mis_cfg_onlinemis.start_greedy_adaptive = false;
        mis_cfg_onlinemis.write_graph = false;

        // benchmark
        ret_code = benchmark_red2pack_solver<heuristic>(graph_filepath, m2s_config, mis_cfg_onlinemis);

        return ret_code;
}