//
// Created by Jannick Borowitz on 21.08.23.
//

#include <cstdint>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>


#include "../app/m2s_configuration_mis.h"
#include "algorithms/branch_and_reduce.h"
#include "algorithms/heuristic.h"

struct BnR {
        using Solver=red2pack::branch_and_reduce;
        using MISConfig=MISConfig;
        using configuration_mis=m2s_configuration_mis;
};

struct Heuristic {
        using Solver=red2pack::heuristic;
        using MISConfig=onlinemis::MISConfig;
        using configuration_mis=m2s_configuration_mis;
};

using Solver_Types = std::tuple<BnR, Heuristic>;

TEMPLATE_LIST_TEST_CASE("A Template product test case", "[template][m2s]", Solver_Types) {
        using namespace red2pack;

        std::vector<std::string> instances = {"../../graphs/lesmis.graph"};


        for(auto instance : instances) {
                typename TestType::MISConfig misConfig;
                M2SConfig m2sConfig;
                m2s_graph_access graph;
                typename TestType::Solver solver(graph, m2sConfig, misConfig);
        }


        REQUIRE(true);
}