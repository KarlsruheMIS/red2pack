#include <red2pack/data_structure/m2s_graph_access.h>
#include <red2pack/m2s_config.h>
#include <red2pack/red2pack_def.h>
#include <scip/scip.h>
#include <scip/scipdefplugins.h>

#include <exception>
#include <fstream>
#include <iostream>
#include <tuple>
#include <vector>

#include "../benchmark_red2pack_solver.h"
#include "../m2s_configuration_m2s.h"
#include "../m2s_parse_parameters.h"

using namespace red2pack;

std::tuple<bool, NodeWeight, std::vector<bool>> solve_w2pack_with_ilp(m2s_graph_access &graph, const double timelimit,
                                                                      const int seed) {
        NodeWeight solution_weight = 0;
        std::vector<bool> in_solution;
        // 1) Umgebung & Modell
        SCIP *scip = nullptr;
        if (SCIPcreate(&scip) != SCIP_OKAY || SCIPincludeDefaultPlugins(scip) != SCIP_OKAY)
                return {false, solution_weight, std::move(in_solution)};
        SCIPsetMessagehdlrQuiet(scip, TRUE);  // silence solver output; set FALSE for logs
        SCIPcreateProbBasic(scip, "mw2ps");
        SCIPsetRealParam(scip, "limits/time", timelimit);
        SCIPsetRealParam(scip, "limits/gap", 0.0);
        SCIPsetIntParam(scip, "lp/threads", 1);
        SCIPsetIntParam(scip, "parallel/maxnthreads", 1);
        SCIPsetIntParam(scip, "randomization/randomseedshift", seed);
        std::vector<SCIP_VAR *> node_vars(graph.number_of_nodes());

        // Binary Variables the solution status of each node
        for (NodeID u = 0; u < graph.number_of_nodes(); ++u) {
                SCIPcreateVarBasic(scip, &node_vars[u], "", 0.0, 1.0, /*obj*/ graph.getNodeWeight(u),
                                   SCIP_VARTYPE_BINARY);
                SCIPaddVar(scip, node_vars[u]);
        }

        // Add closed Neighborhood constraints
        for (NodeID u = 0; u < graph.number_of_nodes(); ++u) {
                SCIP_CONS *closed_neighborhood_constr = nullptr;
                SCIPcreateConsBasicLinear(scip, &closed_neighborhood_constr, "",
                                          /*nvars=*/0, /*vars=*/nullptr, /*vals=*/nullptr,
                                          /*lhs=*/0.0, /*rhs=*/1.0);
                SCIPaddCoefLinear(scip, closed_neighborhood_constr, node_vars[u], 1.0);
                forall_out_edges (graph, e, u) {
                        const NodeID target = graph.getEdgeTarget(e);
                        SCIPaddCoefLinear(scip, closed_neighborhood_constr, node_vars[target], 1.0);
                }
                endfor
                SCIPaddCons(scip, closed_neighborhood_constr);
                SCIPreleaseCons(scip, &closed_neighborhood_constr);
        }

        // Set Objective
        SCIPsetObjsense(scip, SCIP_OBJSENSE_MAXIMIZE);

        // Solve
        SCIPsolve(scip);
        SCIP_STATUS status = SCIPgetStatus(scip);
        std::cout << "ILP status: " << static_cast<int>(status) << std::endl;

        // Obtain solution
        SCIP_SOL *sol = SCIPgetBestSol(scip);
        if (status == SCIP_STATUS_OPTIMAL) {
                solution_weight = static_cast<NodeWeight>(SCIPgetSolOrigObj(scip, sol));
                in_solution.resize(graph.number_of_nodes());
                for (NodeID u = 0; u < graph.number_of_nodes(); ++u) {
                        const bool in = SCIPisFeasEQ(scip, SCIPgetSolVal(scip, sol, node_vars[u]), 1.0);
                        in_solution[u] = in;
                }
                return {true, solution_weight, std::move(in_solution)};
        } else {
                std::cout << "Failed to find an optimal solution\n";
        }

        for (NodeID u = 0; u < graph.number_of_nodes(); ++u) {
                SCIPreleaseVar(scip, &node_vars[u]);
        }
        SCIPfree(&scip);
        return {false, solution_weight, std::move(in_solution)};
}

int main(int argc, char **argv) {
        using namespace red2pack;
        M2SConfig m2s_config;
        std::string graph_filepath;

        // read arguments
        m2s_configuration_m2s configurator_m2s;
        configurator_m2s.standard_weighted(m2s_config);
        auto ret_code = cli::parse_parameters<cli::weighted_solver_cli>(argc, argv, m2s_config, graph_filepath);
        if (ret_code) return 0;

        // supress output
        std::streambuf *backup = std::cout.rdbuf();
        std::ofstream ofs;
        ofs.open("/dev/null");
        if (m2s_config.silent) {
                std::cout.rdbuf(ofs.rdbuf());
        }

        // benchmark
        m2s_log::instance()->restart_total_timer();

        m2s_log::instance()->set_config(m2s_config);

        // read graph
        m2s_graph_access graph;
        m2s_graph_io::readGraphWeighted(graph, graph_filepath);
        m2s_log::instance()->set_graph(graph);

        m2s_log::instance()->print_graph();
        m2s_log::instance()->print_config();

        /*===============================BENCHMARK START===============================*/
#ifdef RED2PACK_ENABLE_MALLOC
        malloc_count_reset_peak();
#endif
        m2s_log::instance()->restart_timer();
        // COMPUTATION START
        // init branch and reduce solver
        auto [found_opt, solution_weight, solution] =
            solve_w2pack_with_ilp(graph, m2s_config.time_limit, m2s_config.seed);

        // COMPUTATION END
        m2s_log::instance()->set_best_size(solution_weight);
        m2s_log::instance()->finish_solving();
        // detach graph from solver
#ifdef RED2PACK_ENABLE_MALLOC
        malloc_count_print_status();
#endif
        /*===============================BENCHMARK END===============================*/

        // VERIFY
        if (found_opt) {
                std::cout << "Solved optimally without timeout" << std::endl;
        } else {
                std::cout << "Failed to solve ILP" << std::endl;
        }

        // PRINT RESULTS
        m2s_log::instance()->print_results();

        auto valid =
            is_maximal_2ps(graph, solution, solution_weight) && solution_weight == m2s_log::instance()->get_best_size();
        if (!valid) {
                return 1;
        }

        if (m2s_config.write_result) {
                m2s_graph_io::writeTwoPackingSet(solution, m2s_config.output_filename);
        }

        if (m2s_config.silent) {
                std::cout.rdbuf(backup);
        }

        return 0;
}
