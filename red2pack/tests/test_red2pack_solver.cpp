//
// Created by Jannick Borowitz on 21.08.23.
//

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cstdint>

#include "../app/m2s_configuration_m2s.h"
#include "../app/m2s_configuration_mis.h"
#include "../extern/onlinemis/configuration_mis.h"
#include "../extern/onlinemis/mis_config.h"
#include "algorithms/branch_and_reduce.h"
#include "algorithms/heuristic.h"
#include "tools/m2s_graph_io.h"

bool is_maximal_2ps(red2pack::m2s_graph_access& graph, const std::vector<bool>& sol, const NodeID sol_size) {
        NodeID check_sol_size = 0;
        fast_set touched(graph.number_of_nodes());

        for (NodeID node = 0; node < graph.number_of_nodes(); node++) {
                if (sol[node]) {
                        check_sol_size++;

                        touched.clear();
                        forall_out_edges (graph, e, node) {
                                if (sol[graph.getEdgeTarget(e)]) {
                                        INFO("Direct Neighbors are part of the solution.");
                                        return false;
                                }
                                touched.add(graph.getEdgeTarget(e));
                        }
                        endfor

                        touched.add(node);

                        forall_out_edges (graph, e, node) {
                                forall_out_edges (graph, e2, graph.getEdgeTarget(e)) {
                                        if (!touched.get(graph.getEdgeTarget(e2)) && sol[graph.getEdgeTarget(e2)]) {
                                                INFO("Two vertices with a common neighbor are part of the solution.");
                                                return false;
                                        }
                                        touched.add(graph.getEdgeTarget(e2));
                                }
                                endfor
                        }
                        endfor
                } else {
                        // maximal ?

                        bool maximal = false;
                        touched.clear();
                        forall_out_edges (graph, e, node) {
                                if (sol[graph.getEdgeTarget(e)]) {
                                        maximal = true;
                                }
                                touched.add(graph.getEdgeTarget(e));
                        }
                        endfor

                        if (!maximal) {
                                forall_out_edges (graph, e, node) {
                                        if (sol[graph.getEdgeTarget(e)]) {
                                                maximal = true;
                                        }
                                }
                                endfor
                                forall_out_edges (graph, e, node) {
                                        forall_out_edges (graph, e2, graph.getEdgeTarget(e)) {
                                                if (!touched.get(graph.getEdgeTarget(e2)) &&
                                                    sol[graph.getEdgeTarget(e2)]) {
                                                        maximal = true;
                                                }
                                                touched.add(graph.getEdgeTarget(e2));
                                        }
                                        endfor
                                }
                                endfor
                        }

                        if (!maximal) {
                                INFO("Solution is not maximal");
                                return false;
                        }
                }
        }

        if (check_sol_size != sol_size) {
                INFO("Solution Size is wrong. It should be " << check_sol_size << " and not " << sol_size << "!");
                return false;
        }

        return true;
}

struct BnR {
        using Solver = red2pack::branch_and_reduce;
        using MIS_Config = MISConfig;
        using configuration_mis = m2s_configuration_mis;
        std::string info = "BnR";
};

struct Heuristic {
        using Solver = red2pack::heuristic;
        using MIS_Config = onlinemis::MISConfig;
        using configuration_mis = onlinemis::configuration_mis;
        std::string info = "Heuristic";
};

using Solver_Types = std::tuple<BnR, Heuristic>;

TEMPLATE_LIST_TEST_CASE("Testing M2S Solvers", "[template][m2s]", Solver_Types) {
        using namespace red2pack;

        std::vector<std::string> instances = {"aGraphErdos40-8.graph", "lesmis.graph", "Outerplanar500_1.graph",
                                              "cac100.graph", "cond-mat-2005.graph"};
        double time_limit = 10;

        for (const auto& instance : instances) {
                M2SConfig m2sConfig;
                m2s_configuration_m2s configurator_m2s;
                configurator_m2s.standard(m2sConfig);
                m2sConfig.time_limit = time_limit;

                m2s_graph_access graph;
                m2s_graph_io::readGraphWeighted(graph, std::string(INSTANCES_PATH) + std::string("/") + instance);

                typename TestType::MIS_Config misConfig;
                typename TestType::configuration_mis configurator;
                configurator.standard(misConfig);
                misConfig.seed = m2sConfig.seed;
                misConfig.console_log = false;
                misConfig.print_log = false;
                misConfig.check_sorted = false;
                misConfig.write_graph = false;
                misConfig.time_limit = time_limit;

                typename TestType::Solver solver(graph, m2sConfig, misConfig);
                solver.solve();
                auto res = is_maximal_2ps(graph, solver.get_solution(), solver.get_solution_size());
                CHECK(res == true);
        }
}
