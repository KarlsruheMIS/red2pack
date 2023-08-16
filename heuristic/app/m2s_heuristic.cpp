#include <chrono>
#include <iostream>

#include "algorithms/heuristic.h"
#include "data_structures/m2s_graph_access.h"
#include "m2s_config.h"
#include "m2s_parse_parameters.h"
#include "mis_config.h"
#include "mis_log.h"
#include "io/graph_io.h"
#include "tools/m2s_graph_io.h"
#include "tools/m2s_log.h"

bool is_maximal_2ps(two_packing_set::m2s_graph_access& graph, const std::vector<bool>& sol, const NodeID sol_size) {

        NodeID check_sol_size = 0;
        fast_set touched(graph.number_of_nodes());

        for (NodeID node = 0; node < graph.number_of_nodes(); node ++){
                if (sol[node]) {
                        check_sol_size++;

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
                        touched.clear();
                        forall_out_edges (graph, e, node) {
                                if (sol[graph.getEdgeTarget(e)]) {
                                        maximal = true;
                                }
                                touched.add(graph.getEdgeTarget(e));
                        }
                        endfor

                        if (!maximal) {
                                forall_out_edges(graph, e, node) {
                                        if (sol[graph.getEdgeTarget(e)]) {
                                                maximal = true;
                                        }
                                }
                                endfor
                                forall_out_edges (graph, e, node) {
                                        forall_out_edges (graph, e2, graph.getEdgeTarget(e)) {
                                                if (!touched.get(graph.getEdgeTarget(e2)) && sol[graph.getEdgeTarget(e2)]) {
                                                        maximal = true;
                                                }
                                                touched.add(graph.getEdgeTarget(e2));
                                        }
                                        endfor
                                }
                                endfor
                        }

                        if (!maximal) {
                                std::cerr << "Solution is not maximal" << std::endl;
                                return false;
                        }

                }
        }

        if (check_sol_size != sol_size) {
                std::cerr << "Solution Size is wrong. It should be " << check_sol_size << " and not " << sol_size << "!" << std::endl;
                return false;
        }

        return true;
}

int main(int argc, char **argv) {
        using namespace two_packing_set;
        // SETUP
        m2s_log::instance()->restart_total_timer();
        m2s_log::instance()->print_title();

        two_packing_set::M2SConfig m2s_config;
        MISConfig mis_config;
        std::string graph_filepath;

        // read arguments
        auto ret_code = parse_parameters(argc, argv, m2s_config, mis_config, graph_filepath);
        if (ret_code) return 0;
        m2s_log::instance()->set_config(m2s_config);

        // read graph
        two_packing_set::m2s_graph_access graph;
        two_packing_set::m2s_graph_io::readGraphWeighted(graph, graph_filepath);
        m2s_log::instance()->set_graph(graph);

        m2s_log::instance()->print_graph();
        m2s_log::instance()->print_config();


        /*===============================BENCHMARK START===============================*/
        m2s_log::instance()->restart_timer();
        // COMPUTATION START
        // init heuristic solver
        two_packing_set::heuristic solver(graph, m2s_config, mis_config);

        // solve graph
        bool found = solver.run();
        // COMPUTATION END
        m2s_log::instance()->set_best_size(m2s_config, solver.get_solution_size());
        /*===============================BENCHMARK END===============================*/

        // VERIFY
        if (!found) {
                std::cout << "Timeout" << std::endl;
        }

        // PRINT RESULTS
        m2s_log::instance()->print_results();

        auto valid = is_maximal_2ps(graph, solver.get_solution(), solver.get_solution_size());
        if (!valid) {
                return 1;
        }

        if (m2s_config.write_2ps) {
                m2s_graph_io::writeTwoPackingSet(solver.get_solution(), m2s_config.output_filename);
        }

        return 0;
}
