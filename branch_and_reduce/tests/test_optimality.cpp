//
// Created by Jannick Borowitz on 27.06.23.
//
//  Let Catch provide main():
#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>

#include "algorithms/branch_and_reduce.h"
#include "m2s_config.h"
#include "m2s_configuration_m2s.h"
#include "m2s_configuration_mis.h"
#include "mis_config.h"
#include "tools/graph_io.h"
#include "tools/m2s_log.h"

void check_is_maximal_2ps(two_packing_set::m2s_graph_access& graph, const std::vector<bool>& sol, const NodeID sol_size) {

        NodeID check_sol_size = 0;

        for (NodeID node = 0; node < graph.number_of_nodes(); node ++){
                if (sol[node]) {
                        check_sol_size++;

                        forall_out_edges (graph, e, node) {
                                REQUIRE(!sol[graph.getEdgeTarget(e)]);
                        }
                        endfor

                        forall_out_edges2(graph, e, node) {
                                REQUIRE(!sol[graph.getEdgeTarget2(e)]);
                        }
                        endfor
                }else {
                        // maximal ?

                        bool maximal = false;
                        forall_out_edges (graph, e, node) {
                                if (sol[graph.getEdgeTarget(e)]) {
                                        maximal = true;
                                }
                        }
                        endfor

                        if (!maximal) {
                                forall_out_edges2(graph, e, node) {
                                        if (sol[graph.getEdgeTarget2(e)]) {
                                                maximal = true;
                                        }
                                }
                                endfor
                        }

                        REQUIRE(maximal);

                }
        }

        REQUIRE(check_sol_size == sol_size);
}

NodeID m2s_bnr(const std::string& graph_filepath, MISConfig& mis_config, two_packing_set::M2SConfig& m2s_config) {
        using namespace two_packing_set;
        // SETUP
        m2s_log::instance()->restart_total_timer();
        m2s_log::instance()->print_title();

        m2s_log::instance()->set_config(m2s_config);

        // read graph
        m2s_graph_access graph;
        graph_io::readGraphWeighted(graph, graph_filepath);
        m2s_log::instance()->set_graph(graph);

        m2s_log::instance()->print_graph();
        m2s_log::instance()->print_config();


        /*===============================BENCHMARK START===============================*/
        m2s_log::instance()->restart_timer();
        // COMPUTATION START
        // init branch and reduce solver
        branch_and_reduce solver(graph, m2s_config, mis_config);

        // solve graph
        bool found = solver.run();
        // COMPUTATION END
        m2s_log::instance()->set_best_size(m2s_config, solver.get_solution_size());
        /*===============================BENCHMARK END===============================*/

        // VERIFY
        if (!found) {
                std::cerr << "Timeout" << std::endl;
                return 0;
        }

        graph.construct_2neighborhood();

        // PRINT RESULTS
        m2s_log::instance()->print_results();

        check_is_maximal_2ps(graph, solver.get_solution(), solver.get_solution_size());
        return solver.get_solution_size();
}


TEST_CASE("Test Optimality", "[single-file]") {
        std::vector<std::pair<std::string, NodeID>> instances; // graph file and optimal solution
        /*instances.emplace_back("aGraphErdos20-0.graph", 7);
        instances.emplace_back("aGraphErdos40-0.graph", 10);
        instances.emplace_back("aGraphErdos40-1.graph", 7);
        instances.emplace_back("aGraphErdos40-2.graph", 9);
        instances.emplace_back("aGraphErdos40-3.graph", 8);
        instances.emplace_back("aGraphErdos40-4.graph", 9);
        instances.emplace_back("aGraphErdos40-5.graph", 8);
        instances.emplace_back("aGraphErdos40-6.graph", 8);
        instances.emplace_back("aGraphErdos40-7.graph", 11);
        instances.emplace_back("aGraphErdos40-8.graph", 8);
        instances.emplace_back("aGraphErdos40-9.graph", 7);
        instances.emplace_back("aGraphErdos40-10.graph", 10);
        instances.emplace_back("aGraphErdos40-11.graph", 9);
        instances.emplace_back("aGraphErdos40-12.graph", 9);
        instances.emplace_back("aGraphErdos40-13.graph", 9);
        instances.emplace_back("aGraphErdos40-14.graph", 8);
        instances.emplace_back("aGraphErdos40-15.graph", 9);
        instances.emplace_back("aGraphErdos40-16.graph", 7);
        instances.emplace_back("aGraphErdos40-17.graph", 8);
        instances.emplace_back("aGraphErdos40-18.graph", 8);
        instances.emplace_back("aGraphErdos40-19.graph", 9);
        instances.emplace_back("aGraphErdos40-20.graph", 9);
        instances.emplace_back("aGraphErdos21-26.graph", 6);
        instances.emplace_back("aGraphErdos22-22.graph", 7);
        instances.emplace_back("aGraphErdos23-23.graph", 6);
        instances.emplace_back("aGraphErdos24-24.graph", 9);*/

        // instances are read in using the following code https://www.geeksforgeeks.org/csv-file-management-using-c/
        std::fstream f_erdos_graphs;
        f_erdos_graphs.open("../../tests/erdos_graphs.txt");
        std::vector<std::string> row;
        std::string line, word, tmp;

        while(f_erdos_graphs >> tmp) {
                row.clear();
                //getline(f_erdos_graphs, line);
                std::cout << tmp << std::endl;
                std::stringstream s(tmp);
                while(getline(s, word, ',')) {
                        row.push_back(word);
                }
                instances.emplace_back(row[0], std::stoi(row[1]));
        }

        f_erdos_graphs.close();

        two_packing_set::M2SConfig m2s_config;
        m2s_configuration_m2s m2s_configurator;
        m2s_configurator.standard(m2s_config);

        MISConfig mis_config;
        m2s_configuration_mis mis_configurator;
        mis_configurator.standard(mis_config);

        SECTION("No reductions") {

                m2s_config.reduction_style2 = two_packing_set::M2SConfig::Reduction_Style2::extended;
                m2s_config.time_limit = 1000000;
                m2s_config.disable_deg_one = true;
                m2s_config.disable_clique = true;
                m2s_config.disable_deg_two = true;
                m2s_config.disable_domination = true;
                m2s_config.disable_twin = true;

                for(const auto& [inst, opt_sol_size] : instances) {
                        INFO("Testing " + inst);
                        m2s_config.graph_filename = inst;
                        NodeID sol_size = m2s_bnr( inst, mis_config, m2s_config);
                        REQUIRE(sol_size == opt_sol_size);
                }
        }


        SECTION("All extended reductions (except deg-2)") {
                m2s_config.reduction_style2 = two_packing_set::M2SConfig::Reduction_Style2::extended;
                m2s_config.time_limit = 1000000;
                m2s_config.disable_deg_two = true;

                for(const auto& [inst, opt_sol_size] : instances) {
                        INFO("Testing " + inst);
                        m2s_config.graph_filename = inst;
                        NodeID sol_size = m2s_bnr(inst, mis_config, m2s_config);
                        REQUIRE(sol_size == opt_sol_size);
                }
        }

        SECTION("Only Deg-1 reduction") {
                m2s_config.reduction_style2 = two_packing_set::M2SConfig::Reduction_Style2::extended;
                m2s_config.time_limit = 1000000;
                m2s_config.disable_clique = true;
                m2s_config.disable_deg_two = true;
                m2s_config.disable_domination = true;
                m2s_config.disable_twin = true;

                for(const auto& [inst, opt_sol_size] : instances) {
                        INFO("Testing " + inst);
                        m2s_config.graph_filename = inst;
                        NodeID sol_size = m2s_bnr(inst, mis_config, m2s_config);
                        REQUIRE(sol_size == opt_sol_size);
                }
        }

        SECTION("Only Clique reduction") {
                m2s_config.reduction_style2 = two_packing_set::M2SConfig::Reduction_Style2::extended;
                m2s_config.time_limit = 1000000;
                m2s_config.disable_deg_one = true;
                m2s_config.disable_deg_two = true;
                m2s_config.disable_domination = true;
                m2s_config.disable_twin = true;

                for(const auto& [inst, opt_sol_size] : instances) {
                        INFO("Testing " + inst);
                        m2s_config.graph_filename = inst;
                        NodeID sol_size = m2s_bnr(inst, mis_config, m2s_config);
                        REQUIRE(sol_size == opt_sol_size);
                }
        }

        SECTION("Only Domination reduction") {
                m2s_config.reduction_style2 = two_packing_set::M2SConfig::Reduction_Style2::extended;
                m2s_config.time_limit = 1000000;
                m2s_config.disable_clique = true;
                m2s_config.disable_deg_two = true;
                m2s_config.disable_deg_one = true;
                m2s_config.disable_twin = true;

                for(const auto& [inst, opt_sol_size] : instances) {
                        INFO("Testing " + inst);
                        m2s_config.graph_filename = inst;
                        NodeID sol_size = m2s_bnr( inst, mis_config, m2s_config);
                        REQUIRE(sol_size == opt_sol_size);
                }
        }

        /*
        SECTION("Only 2-deg reduction") {
                m2s_config.reduction_style2 = two_packing_set::M2SConfig::Reduction_Style2::extended;
                m2s_config.time_limit = 1000000;
                m2s_config.disable_clique = true;
                m2s_config.disable_domination = true;
                m2s_config.disable_deg_one = true;
                m2s_config.disable_twin = true;

                for(const auto& [inst, opt_sol_size] : instances) {
                        INFO("Testing " + inst);
                        m2s_config.graph_filename = inst;
                        NodeID sol_size = m2s_bnr(inst, mis_config, m2s_config);
                        REQUIRE(sol_size == opt_sol_size);
                }
        }
        */

        SECTION("Only twin reduction") {
                m2s_config.reduction_style2 = two_packing_set::M2SConfig::Reduction_Style2::extended;
                m2s_config.time_limit = 1000000;
                m2s_config.disable_clique = true;
                m2s_config.disable_domination = true;
                m2s_config.disable_deg_one = true;
                m2s_config.disable_deg_two = true;

                for(const auto& [inst, opt_sol_size] : instances) {
                        INFO("Testing " + inst);
                        m2s_config.graph_filename = inst;
                        NodeID sol_size = m2s_bnr(inst, mis_config, m2s_config);
                        REQUIRE(sol_size == opt_sol_size);
                }
        }

}