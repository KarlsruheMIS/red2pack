#ifndef RED2PACK_BENCHMARK_RED2PACK_SOLVER_H
#define RED2PACK_BENCHMARK_RED2PACK_SOLVER_H

#include <red2pack/red2pack_def.h>
#include <red2pack/data_structure/m2s_graph_access.h>
#include <red2pack/data_structure/fast_set.h>
#include <red2pack/m2s_config.h>
#include <red2pack/tools/m2s_graph_io.h>
#include <red2pack/tools/m2s_log.h>

#ifdef RED2PACK_ENABLE_MALLOC
#include <malloc_count.h>
#endif

#include <iostream>


bool is_maximal_2ps(red2pack::m2s_graph_access& graph, const std::vector<bool>& sol, const red2pack::NodeID sol_weight) {
        using namespace red2pack;

        NodeID check_sol_weight = 0;
	fast_set touched(graph.number_of_nodes());

        for (NodeID node = 0; node < graph.number_of_nodes(); node ++){
                if (sol[node]) {
                        check_sol_weight += graph.getNodeWeight(node);

                        touched.clear();
                        forall_out_edges (graph, e, node) {
                                if (sol[graph.getEdgeTarget(e)]) {
                                        std::cerr << "Direct Neighbors are part of the solution." << std::endl;
                                        return false;
                                }
                                touched.add(graph.getEdgeTarget(e));
                        }
                        endfor

                        touched.add(node);

                        forall_out_edges (graph, e, node) {
                                forall_out_edges (graph, e2, graph.getEdgeTarget(e)) {
                                        if (!touched.get(graph.getEdgeTarget(e2)) && sol[graph.getEdgeTarget(e2)]) {
                                                std::cerr << "Two vertices with a common neighbor are part of the solution." << std::endl;
                                                return false;
                                        }
                                        touched.add(graph.getEdgeTarget(e2));
                                }
                                endfor
                        }
                        endfor
                }else {
                        // maximal ?

                        bool maximal = false;
                        forall_out_edges (graph, e, node) {
                                if (sol[graph.getEdgeTarget(e)]) {
                                        maximal = true;
                                        break;
                                }
                        }
                        endfor

                        if (!maximal) {
                                forall_out_edges (graph, e, node) {
                                        auto target = graph.getEdgeTarget(e);
                                        forall_out_edges (graph, e2, target) {
                                                if (sol[graph.getEdgeTarget(e2)]) {
                                                        maximal = true;
                                                        break;
                                                }
                                        }
                                        endfor
                                        if(maximal) {
                                                break;
                                        }
                                }
                                endfor
                        }

                        if (!maximal) {
                                EdgeID edges = 0;
                                touched.clear();
                                touched.add(node);
                                std::cout << "node can be added to solution: " << node << std::endl;
                                forall_out_edges(graph, e, node) {
                                        edges++;
                                        touched.add(graph.getEdgeTarget(e));
                                        std::cout << graph.getEdgeTarget(e) << std::endl;
                                }
                                endfor
                                std::cout << edges << std::endl;
                                edges = 0;
                                forall_out_edges (graph, e, node) {
                                        auto target =  graph.getEdgeTarget(e);
                                        forall_out_edges (graph, e2, target) {
                                                if (touched.add(graph.getEdgeTarget(e2))) {
                                                        edges++;
                                                }
                                        }
                                        endfor
                                }
                                endfor
                                std::cout << edges << std::endl;
                                std::cerr << "Solution is not maximal" << std::endl;
                                return false;
                        }

                }
        }

        if (check_sol_weight != sol_weight) {
                std::cerr << "Solution Weight is wrong. It should be " << check_sol_weight << " and not " << sol_weight << "!" << std::endl;
                return false;
        }else{
                std::cout << "checked solution weight: " << check_sol_weight << std::endl;
        }

        return true;
}

bool is_2ps(red2pack::m2s_graph_access& graph, const std::vector<bool>& sol, const red2pack::NodeID sol_weight) {
        using namespace red2pack;

        NodeWeight check_sol_weight = 0;
        fast_set touched(graph.number_of_nodes());

        for (NodeID node = 0; node < graph.number_of_nodes(); node ++){
                if (sol[node]) {
                        check_sol_weight += graph.getNodeWeight(node);

                        touched.clear();
                        forall_out_edges (graph, e, node) {
                                if (sol[graph.getEdgeTarget(e)]) {
                                        std::cerr << "Direct Neighbors are part of the solution." << std::endl;
                                        return false;
                                }
                                touched.add(graph.getEdgeTarget(e));
                        }
                        endfor

                        touched.add(node);

                        forall_out_edges (graph, e, node) {
                                forall_out_edges (graph, e2, graph.getEdgeTarget(e)) {
                                        if (!touched.get(graph.getEdgeTarget(e2)) && sol[graph.getEdgeTarget(e2)]) {
                                                std::cerr << "Two vertices with a common neighbor are part of the solution." << std::endl;
                                                return false;
                                        }
                                        touched.add(graph.getEdgeTarget(e2));
                                }
                                endfor
                        }
                        endfor
                }
        }

        if (check_sol_weight != sol_weight) {
                std::cerr << "Solution Weight is wrong. It should be " << check_sol_weight << " and not " << sol_weight << "!" << std::endl;
                return false;
        } else{
                std::cout << "checked solution weight: " << check_sol_weight << std::endl;
        }


        return true;
}

template<typename Solver, typename... MIS_Config>
int benchmark_red2pack_solver(std::string graph_filepath, red2pack::M2SConfig m2s_config, MIS_Config... mis_configs) {
        using namespace red2pack;
        // SETUP
        m2s_log::instance()->restart_total_timer();

        m2s_log::instance()->set_config(m2s_config);

        // read graph
        auto graph = std::make_unique<m2s_graph_access>();
        m2s_graph_io::readGraphWeighted(*graph, graph_filepath);
        m2s_log::instance()->set_graph(*graph);

        m2s_log::instance()->print_graph();
        m2s_log::instance()->print_config();


        /*===============================BENCHMARK START===============================*/
#ifdef RED2PACK_ENABLE_MALLOC
        malloc_count_reset_peak();
#endif
        m2s_log::instance()->restart_timer();
        // COMPUTATION START
        // init branch and reduce solver
        Solver solver(std::move(graph), m2s_config, mis_configs...);

        // solve graph
        bool found_opt = solver.solve(m2s_log::instance()->get_timer_copy());
        // COMPUTATION END
        m2s_log::instance()->set_best_size(solver.get_solution_size());
        m2s_log::instance()->finish_solving();
        // detach graph from solver
        graph = std::move(solver.detach());
#ifdef RED2PACK_ENABLE_MALLOC
        malloc_count_print_status();
#endif
        /*===============================BENCHMARK END===============================*/

        // VERIFY
        if (found_opt) {
                std::cout << "Solved optimally without timeout" << std::endl;
        }else {
                std::cout << "Solved heuristically (due to timeout)" << std::endl;
        }

        // PRINT RESULTS
        m2s_log::instance()->print_results();
        if (m2s_config.use_weighted_reductions()) {
                m2s_log::instance()->print_reduced_nodes_mw2ps();
        }

        auto valid = is_maximal_2ps(*graph, solver.get_solution(), solver.get_solution_size()) &&
                        solver.get_solution_size() == m2s_log::instance()->get_best_size();
        if (!valid) {
                return 1;
        }

        if (m2s_config.write_result) {
                m2s_graph_io::writeTwoPackingSet(solver.get_solution(), m2s_config.output_filename);
        }

        return 0;
}

#endif  // RED2PACK_BENCHMARK_RED2PACK_SOLVER_H
