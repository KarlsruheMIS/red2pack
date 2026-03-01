//
// Created by Jannick Borowitz on 20.06.24.
//

#ifndef RED2PACK_HILS_UTILS_H
#define RED2PACK_HILS_UTILS_H

#include "red2pack-htwis-hils/hils_config.h"

#include <hils/src/Graph.h>
#include <hils/src/Solution.h>

#include <red2pack/tools/scoped_timer.h>

#include <graph_access.h>

#include <random>

namespace opt {
std::mt19937 generator;
};

namespace red2pack {

inline void convert(m2s_graph_access& input_graph, opt::Graph& graph) {
        RED2PACK_SCOPED_TIMER("Convert m2s_graph_access to opt::Graph");

        for (NodeID v = 0; v < input_graph.number_of_nodes(); ++v) {
                graph.weights_[v] = input_graph.getNodeWeight(v);
        }

        for (NodeID v = 0; v < input_graph.number_of_nodes(); ++v) {
                // direct edges
                forall_out_edges (input_graph, e, v) {
                        auto u = input_graph.getEdgeTarget(e);
                        if (v < u) {
                                graph.addEdge(u, v);
                        }
                }
                endfor
                // two-edges
                forall_out_links(input_graph, e2, v) {
                        auto u = input_graph.getLinkTarget(e2);
                        if (v < u) {
                                graph.addEdge(u, v);
                        }
                }
                endfor
        }
}

inline void convert(graph_access& input_graph, opt::Graph& graph) {
        RED2PACK_SCOPED_TIMER("Convert graph_access to opt::Graph");

        for (NodeID v = 0; v < input_graph.number_of_nodes(); ++v) {
                graph.weights_[v] = input_graph.getNodeWeight(v);
        }

        for (NodeID v = 0; v < input_graph.number_of_nodes(); ++v) {
                // direct edges
                forall_out_edges (input_graph, e, v) {
                        auto u = input_graph.getEdgeTarget(e);
                        if (v < u) {
                                graph.addEdge(u, v);
                        }
                }
                endfor
        }
}



inline opt::Solution perform_hils_impl(const opt::Graph& graph_instance, opt::Solution&& initial_solution, const HilsConfig &hils_cfg, double& target_time, timer& proc_timer, double time_limit) {
        using opt::Graph;
        using opt::Solution;

        // stats about the best solution
        double target_iterations = 0;  // hils iterations until best solution

        auto &s = initial_solution;

        do {
                while (!s.isMaximal()) {
                        s.addRandomVertex();
                }
        } while ((s.omegaImprovement() || s.twoImprovement()) &&
                 proc_timer.elapsed() < time_limit /*|| s.threeImprovement() */);

        Solution best_s(s);
        std::cout << "best weight: " << best_s.weight() << "\n";
	target_time=proc_timer.elapsed(); // added by Borowitz to ensure initial solution is associated with a running time
	if(target_time > time_limit) {
		return best_s;
	} 

        // run ILS iterations

        int k = 1;
        int local_best = s.weight();
        int iter;
        for (iter = 0; iter < hils_cfg.iterations; iter++) {
                Solution next_s(s);

                // shake
                next_s.force(hils_cfg.p[0]);

                assert(next_s.integrityCheck());

                do {
                        while (!next_s.isMaximal()) {
                                next_s.addRandomVertex();
                        }
                } while (next_s.omegaImprovement() || next_s.twoImprovement());

                assert(best_s.integrityCheck());

                if (next_s.weight() > s.weight()) {
                        k = 1;
                        s = next_s;

                        if (local_best < next_s.weight()) {
                                k -= s.size() / hils_cfg.p[1];
                                local_best = next_s.weight();
                        }

                        if (best_s.weight() < s.weight()) {
                                best_s = s;
                                k -= s.size() * hils_cfg.p[2];

                                target_iterations = iter;
                                target_time = proc_timer.elapsed();
                                // check time limit
                                if (target_time > time_limit) {
                                        break;
                                }

                                std::cout << "new best weight: " << best_s.weight() << " / iteration: " << iter
                                          << " / time (s): " << target_time << "\n";
                        }
                } else if (k <= s.size() / hils_cfg.p[1]) {
                        k++;
                } else {
                        local_best = s.weight();
                        s.force(hils_cfg.p[3]);
                        k = 1;
                }
                // check time limit here
                if (proc_timer.elapsed() > time_limit) {
                        break;
                }
        }
        assert(best_s.integrityCheck());

        double hils_time = proc_timer.elapsed();

        std::cout << best_s.weight() << " " << target_time << " " << hils_time << "\n";

        // set MIS solution
        return best_s;
}

inline opt::Solution perform_hils(const opt::Graph& graph_instance, const HilsConfig &hils_cfg, double &time_best, double mis_solve_time_limit) {
        RED2PACK_SCOPED_TIMER("HILS");
        using opt::Graph;
        using opt::Solution;
        timer proc_timer;
        proc_timer.restart();

        // stats about the best solution
        Solution s(&graph_instance);

        // randomly initialize a solution
        while (!s.isMaximal()) {
                s.addRandomVertex();
                assert(s.integrityCheck());
        }

        return perform_hils_impl(graph_instance, std::move(s), hils_cfg, time_best, proc_timer, mis_solve_time_limit-proc_timer.elapsed());
}

inline opt::Solution perform_hils(const opt::Graph& graph_instance, opt::Solution&& initial_solution, const HilsConfig &hils_cfg, double &time_best, double mis_solve_time_limit) {
        RED2PACK_SCOPED_TIMER("HILS");
        using opt::Graph;
        using opt::Solution;
        timer proc_timer;
        proc_timer.restart();

        return perform_hils_impl(graph_instance, std::move(initial_solution), hils_cfg, time_best, proc_timer, mis_solve_time_limit-proc_timer.elapsed());
}

}

#endif  // RED2PACK_HILS_UTILS_H
