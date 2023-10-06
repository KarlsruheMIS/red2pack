#include <iostream>

#include "../benchmark_red2pack_solver.h"
#include "algorithms/branch_and_reduce.h"
#include "m2s_config.h"
#include "mis_config.h"

int main(int argc, char **argv) {
        using namespace red2pack;
        M2SConfig m2s_config;
        MISConfig mis_config;
        std::string graph_filepath;

        // read arguments
        auto ret_code = parse_parameters(argc, argv, m2s_config, mis_config, graph_filepath);
        if (ret_code) return 0;

        // supress output
        std::streambuf* backup = std::cout.rdbuf();
        std::ofstream ofs;
        ofs.open("/dev/null");
        if(m2s_config.silent) {
                std::cout.rdbuf(ofs.rdbuf()); 
        }

        // benchmark
        ret_code = benchmark_red2pack_solver<branch_and_reduce>(graph_filepath, m2s_config, mis_config);

        return ret_code;
}
