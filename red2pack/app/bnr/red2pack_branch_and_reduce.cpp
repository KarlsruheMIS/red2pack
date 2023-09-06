#include <chrono>
#include <iostream>

#include "../benchmark_red2pack_solver.h"
#include "../m2s_parse_parameters.h"
#include "algorithms/branch_and_reduce.h"
#include "data_structure/m2s_graph_access.h"
#include "io/graph_io.h"
#include "m2s_config.h"
#include "mis_config.h"
#include "tools/m2s_graph_io.h"
#include "tools/m2s_log.h"

int main(int argc, char **argv) {
        using namespace red2pack;
        M2SConfig m2s_config;
        MISConfig mis_config;
        std::string graph_filepath;

        // read arguments
        auto ret_code = parse_parameters(argc, argv, m2s_config, mis_config, graph_filepath);
        if (ret_code) return 0;

        // benchmark
        ret_code = benchmark_red2pack_solver<branch_and_reduce>(graph_filepath, m2s_config, mis_config);

        return ret_code;
}