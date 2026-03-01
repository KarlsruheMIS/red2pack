//
// Created by Jannick Borowitz on 23.10.24.
//
#include "red2pack-scip-ilp/algorithms/weighted_rnt_scip_ilp.h"

#include <red2pack/algorithms/kernel/weighted_reduce_algorithm.h>
#include <red2pack/tools/scoped_timer.h>
#include <scip/scip.h>
#include <scip/scipdefplugins.h>

#include <limits>

namespace {

void add_edge_constraint(SCIP *scip, SCIP_VAR *v, SCIP_VAR *u) {
        SCIP_CONS *edge_constr = nullptr;
        SCIPcreateConsBasicLinear(scip, &edge_constr, "",
                                  /*nvars=*/0, /*vars=*/nullptr, /*vals=*/nullptr,
                                  /*lhs=*/0.0, /*rhs=*/1.0);
        SCIPaddCoefLinear(scip, edge_constr, v, 1.0);
        SCIPaddCoefLinear(scip, edge_constr, u, 1.0);
        SCIPaddCons(scip, edge_constr);
        SCIPreleaseCons(scip, &edge_constr);
}

using namespace red2pack;
std::tuple<bool, NodeWeight, std::vector<bool>> solve_mwis_with_ilp(m2s_graph_access &graph, const double timelimit,
                                                                    const int seed) {
        NodeWeight solution_weight = 0;
        std::vector<bool> in_solution;
        // 1) Umgebung & Modell
        SCIP *scip = nullptr;
        if (SCIPcreate(&scip) != SCIP_OKAY || SCIPincludeDefaultPlugins(scip) != SCIP_OKAY)
                return {false, solution_weight, std::move(in_solution)};
        SCIPsetMessagehdlrQuiet(scip, TRUE);  // silence solver output; set FALSE for logs
        SCIPcreateProbBasic(scip, "MWIS");
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
                forall_out_edges (graph, e, u) {
                        const NodeID target = graph.getEdgeTarget(e);
                        if (u < target) {
                                add_edge_constraint(scip, node_vars[u], node_vars[target]);
                        }
                }
                endfor
                forall_out_links(graph, e, u) {
                        const NodeID target = graph.getLinkTarget(e);
                        if (u < target) {
                                add_edge_constraint(scip, node_vars[u], node_vars[target]);
                        }
                }
                endfor
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

}  // namespace

namespace red2pack {

void weighted_rnt_scip_ilp::init_reducer() { reducer = std::make_unique<weighted_reduce_algorithm>(*graph, m2s_cfg); }
bool weighted_rnt_scip_ilp::use_reducer() { return m2s_cfg.use_weighted_reductions(); }

bool weighted_rnt_scip_ilp::solve_mis(double mis_solve_time_limit, m2s_graph_access &reduced_graph) {
        RED2PACK_SCOPED_TIMER("Solve MWIS");

        timer total_solve_mis;
        total_solve_mis.restart();

        std::vector<bool> solution(reduced_graph.number_of_nodes(), false);
        // ATTENTION for experiment evaluation: time best cannot be obtained from ILP solver ...
        auto [solved, sol_weight_ilp, solution_ilp] =
            solve_mwis_with_ilp(reduced_graph, mis_solve_time_limit, m2s_cfg.seed);
        mis_solution_size += sol_weight_ilp;
        m2s_log::instance()->set_best_size(get_solution_size());
        if (!solved) {
                return false;
        }
        for (NodeID v = 0; v < reduced_graph.number_of_nodes(); ++v) {
                solution_status[former_node_id[v]] = solution_ilp[v];
        }
        return true;
}

}  // namespace red2pack
