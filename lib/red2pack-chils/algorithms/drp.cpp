//
// Created by Jannick Borowitz on 15.10.24.
//

#include "chils_utils.h"
#include "red2pack-chils/algorithms/drp.h"


#include <branch_and_reduce_algorithm.h>
#include <macros_assertions.h>
#include <random_functions.h>
#include <red2pack/algorithms/kernel/weighted_reduce_algorithm.h>
#include <red2pack/m2s_config.h>
#include <red2pack/red2pack_def.h>
#include <red2pack/tools/m2s_log.h>
#include <limits>

namespace red2pack {

void drp::init_reducer() { reducer = std::make_unique<weighted_reduce_algorithm>(*graph, m2s_cfg); }
bool drp::use_reducer() { return m2s_cfg.use_weighted_reductions(); }

NodeWeight drp::maximize_solution(m2s_graph_access& reduced_graph,
                                                     std::vector<bool>& reduced_solution) {
        free_nodes.clear();
        NodeWeight added = 0;
        for (NodeID v = 0; v < reduced_graph.number_of_nodes(); ++v) {
                if (!reduced_solution[v]) {
                        // check for maximality
                        bool free_node = true;
                        forall_out_edges (reduced_graph, e, v) {
                                auto target = reduced_graph.getEdgeTarget(e);
                                if (reduced_solution[target]) {
                                        free_node = false;
                                        break;
                                }
                        }
                        endfor
                        if (free_node) {
                                forall_out_links(reduced_graph, e, v) {
                                        auto target = reduced_graph.getLinkTarget(e);
                                        if (reduced_solution[target]) {
                                                free_node = false;
                                                break;
                                        }
                                }
                                endfor
                        }
                        if (free_node) {
                                free_nodes.push_back(v);
                        }
                }
        }
        std::sort(free_nodes.begin(), free_nodes.end(), [&g = reduced_graph](NodeID first, NodeID second) {
                return g.getNodeWeight(first) > g.getNodeWeight(second);
        });
        for (NodeID node : free_nodes) {
                ASSERT_TRUE(!reduced_solution[node]);
                bool free_node = true;
                forall_out_edges (reduced_graph, e, node) {
                        auto target = reduced_graph.getEdgeTarget(e);
                        if (reduced_solution[target]) {
                                free_node = false;
                                break;
                        }
                }
                endfor
                if (free_node) {
                        forall_out_links(reduced_graph, e, node) {
                                auto target = reduced_graph.getLinkTarget(e);
                                if (reduced_solution[target]) {
                                        free_node = false;
                                        break;
                                }
                        }
                        endfor
                }

                if (free_node) {
                        reduced_solution[node] = true;
                        added += reduced_graph.getNodeWeight(node);
                }
        }
        return added;
}

bool drp::solve_mis(double mis_solve_time_limit, m2s_graph_access& reduced_graph) {
        // solve core (non-sim graph)?
        M2SConfig::Core_Solver core_solver = m2s_cfg.core_solver;
        const bool solve_non_sim_graph = core_solver != M2SConfig::Core_Solver::none;
        double max_core_time_limit = m2s_cfg.max_core_time_limit;
        // how many solution should CHILS maintain
        unsigned max_chils = m2s_cfg.max_chils;
        // similarity threshold
        double similarity_threshold = m2s_cfg.similarity_threshold;
        // increase similarity threshold if improvements were made
        const double increase_sim_thres = m2s_cfg.increase_sim_thres;
        // decrease similarity threshold if solution was optimal
        const double decrease_sim_thres = m2s_cfg.decrease_sim_thres;
        // non adaptive ratings perturb the rating order step-by-step with a probability of [0, max_perturb_prob]
        // (uniformily chosen at random for each rnp run)
        const double max_perturb_prob = 1;  // m2s_cfg.perturb_prob;
        // adaptive ratings choose in a heuristic step one of the m2s_csfg.max_num_candidates top candidates uniformily
        // at random we increase m2s_cfg.max_num_candidate step-wise to a maximum of max_max_num_candidates and re-start
        // with 1
        const unsigned max_max_num_candidates = m2s_cfg.max_max_num_candidates;

        constexpr bool PRINT = false;

        timer proc_timer;
        proc_timer.restart();

        std::cout << "solve non-sim. graph" << solve_non_sim_graph << std::endl;
        std::cout << "core_solver: " << static_cast<int>(core_solver) << std::endl;
        std::cout << "-init sim threshold: " << similarity_threshold << std::endl;
        std::cout << "-incr. sim threshold: " << increase_sim_thres << std::endl;
        std::cout << "-decr. sim threshold: " << decrease_sim_thres << std::endl;
        std::cout << "-max core time limit [s]: " << max_core_time_limit << std::endl;
        std::cout << "-max chils: " << max_chils << std::endl;
        std::cout << "-perturb prob: " << max_perturb_prob << std::endl;
        std::cout << "max max num candidates: " << max_max_num_candidates << std::endl;

        double eps_bnr_time_limit = 5;  // seconds
        if (!solve_non_sim_graph) {
                eps_bnr_time_limit = 0;
        }

        // stats
        std::size_t iterations = 0;

        // rnp algorithm
        auto rnp = weighted_reduce_algorithm(
            reduced_graph,
            {.seed = m2s_cfg.seed,
             .time_limit = mis_solve_time_limit - proc_timer.elapsed(),
             .disable_fast_neighborhood_removal = true,
             .on_demand_two_neighborhood = false,
             .maintain_two_neigh = true,
             .max_num_candidates = 1,  // pick candidate randomly from top max_num_candidates in adaptive rating
             .adaptive_rating = true,
             .perturb_prob = max_perturb_prob,
             .shuffle_redu_order = true,
             .heuristic_decision_style = two_pack_status::excluded,
             .heuristic_rating = mw2ps_heuristic_ratings::weight_diff,
             .reduction_style = M2SConfig::Reduction_Style2::heuristic});
        M2SConfig& rnp_config = rnp.config;

        auto configure_next_rnp_config = [&](std::size_t iteration) {
                auto configs = 5;
                if (solve_non_sim_graph) {
                        // if the core graph is solved, use also non adaptive rating
                        configs = 10;
                }

                auto config_idx = iteration % configs;
                rnp_config.seed = iteration / configs;
                if (config_idx == 0 || config_idx == 1 || config_idx == 5 || config_idx == 6) {
                        rnp_config.heuristic_rating = mw2ps_heuristic_ratings::weight_diff;
                } else if (config_idx == 2 || config_idx == 3 || config_idx == 7 || config_idx == 8) {
                        rnp_config.heuristic_rating = mw2ps_heuristic_ratings::weight;
                } else {
                        ASSERT_TRUE(config_idx == 4 || config_idx == 9);
                        rnp_config.heuristic_rating = mw2ps_heuristic_ratings::deg;
                }

                if (config_idx < 5) {
                        // adaptive rating
                        rnp_config.adaptive_rating = true;
                        rnp_config.max_num_candidates = 1 + ((iterations / configs) % max_max_num_candidates);
                } else {
                        // non-adaptive rating
                        rnp_config.adaptive_rating = false;
                        rnp_config.perturb_prob = random_functions::nextDouble(0.5, max_perturb_prob);
                }

                if (config_idx % 5 < 4  // only for the first two heuristic ratings
                    && config_idx % 2 == 1) {
                        // choose include strategy
                        rnp_config.heuristic_decision_style = two_pack_status::included;
                } else {
                        // chose exclude strategy
                        rnp_config.heuristic_decision_style = two_pack_status::excluded;
                }

                rnp_config.time_limit = mis_solve_time_limit - eps_bnr_time_limit - proc_timer.elapsed();

                return config_idx;
        };

        // initial rnp step
        std::vector<bool> best_solution(reduced_graph.number_of_nodes(), false);
        std::vector<bool> solution(reduced_graph.number_of_nodes(), false);
        std::vector<bool> similars(reduced_graph.number_of_nodes(), true);
        NodeID similar = similars.size();
        sized_vector<NodeID> reverse_mapping(reduced_graph.number_of_nodes());
        sized_vector<NodeID> mapping(reduced_graph.number_of_nodes());

        auto relative_similarity_to_best_solution = [&]() {
                return static_cast<double>(similar) / static_cast<double>(reduced_graph.number_of_nodes());
        };

        // some helper functions that print usefull information between the rnp runs
        auto print_current_rnp_config = [](unsigned config_idx, const M2SConfig& config) {
                if (PRINT)
                        std::cout << "RnP" << config_idx << ": seed=" << config.seed
                                  << ", perturb_prob=" << config.perturb_prob
                                  << ", max_num_candidates=" << config.max_num_candidates << std::endl;
        };
        auto print_rnp_solution = [&](NodeWeight rnp_solution_weight) {
                if (PRINT)
                        std::cout << "RnP solution_weight=" << rnp_solution_weight
                                  << ", best_solution=" << mis_solution_size << std::endl;
        };
        auto print_similarity = [&]() {
                if (PRINT)
                        std::cout << "similars: " << similar << " / " << reduced_graph.number_of_nodes() << " ( "
                                  << 100.0 * relative_similarity_to_best_solution() << " % ), threshold ( "
                                  << 100.0 * similarity_threshold << " )" << std::endl;
        };
        auto print_new_similarity_threshold = [&]() {
                if (PRINT) std::cout << "new similarity threshold: " << similarity_threshold << std::endl;
        };
        auto print_core_graph = [](NodeID nodes, EdgeID edges) {
                if (PRINT) std::cout << "core: nodes=" << nodes << ", edges=" << edges << std::endl;
        };

        {
                //RED2PACK_SCOPED_TIMER("RnP step");
                auto config_idx = configure_next_rnp_config(iterations);
                print_current_rnp_config(config_idx, rnp_config);
                auto &status = rnp.global_status;
                for (std::size_t i = 0; i < status.reductions.size() - 1; ++i) {
                                // we know that we need to peel first
                                status.reductions[i]->has_run = true;
                }
                rnp.run_reductions();  // @todo set option to start with a peeling step
                ++iterations;
                rnp.build_solution(best_solution);
                NodeWeight added = maximize_solution(reduced_graph, best_solution);
                mis_solution_size = rnp.global_status.sol_weight + added;
                print_rnp_solution(rnp.get_solution_weight() + added);
                m2s_log::instance()->set_best_size(get_solution_size());
                rnp.reset();
        }

        // further rnp steps
        while (proc_timer.elapsed() < mis_solve_time_limit - eps_bnr_time_limit) {
                std::size_t inner_iterations = 0;

                // obterin further rnp solution until they are overall too unsimilar
                while ((!solve_non_sim_graph || relative_similarity_to_best_solution() > similarity_threshold) &&
                       proc_timer.elapsed() < mis_solve_time_limit - eps_bnr_time_limit) {
                        //RED2PACK_SCOPED_TIMER("RnP step");

                        // prepare next rnp step
                        auto config_idx = configure_next_rnp_config(iterations);
                        print_current_rnp_config(config_idx, rnp_config);

                        auto& status = rnp.global_status;
                        for (std::size_t i = 0; i < status.reductions.size() - 1; ++i) {
                                // we know that we need to peel first
                                status.reductions[i]->has_run = true;
                        }
                        status.reductions.back()->has_run = false;

                        // run reducer
                        rnp.run_reductions();

                        // maximize solution
                        rnp.build_solution(solution);
                        NodeWeight added_sol_weight = maximize_solution(reduced_graph, solution);
                        print_rnp_solution(rnp.get_solution_weight() + added_sol_weight);

                        if (rnp.get_solution_weight() + added_sol_weight > mis_solution_size) {
                                using std::swap;
                                mis_solution_size = rnp.global_status.sol_weight + added_sol_weight;
                                swap(solution, best_solution);
                                m2s_log::instance()->set_best_size( get_solution_size());
                                similar = similars.size();
                                std::fill(similars.begin(), similars.end(), true);
                        } else if (solve_non_sim_graph) {
                                // update similarity
                                similar = 0;
                                for (NodeID i = 0; i < solution.size(); ++i) {
                                        similars[i] = similars[i] && solution[i] == best_solution[i];
                                        similar += similars[i];
                                }
                                print_similarity();
                        }

                        // reset rnp reducer
                        rnp.reset();

                        ++iterations;
                        ++inner_iterations;
                }

                if (inner_iterations == 0) {
                        break;  // no RnP solution was determined (timeout)
                }

                if (!solve_non_sim_graph) {
                        continue;
                }

                // preparing core graph
                NodeID nodes = 0;
                NodeWeight H_best_weight = 0;
                mapping.set_size(reduced_graph.number_of_nodes());
                reverse_mapping.clear();
                for (NodeID node = 0; node < reduced_graph.number_of_nodes(); ++node) {
                        if (!similars[node]) {
                                reverse_mapping.push_back(node);
                                mapping[node] = nodes++;
                                if (best_solution[node]) {
                                        H_best_weight += reduced_graph.getNodeWeight(node);
                                }
                        }
                }

                if (core_solver = M2SConfig::Core_Solver::exact) {
                        //RED2PACK_SCOPED_TIMER("solving core");
                        graph_access H;

                        H.start_construction(nodes,
                                             reduced_graph.number_of_edges() + reduced_graph.number_of_links());
                        for (NodeID node = 0; node < reduced_graph.number_of_nodes(); ++node) {
                                if (!similars[node]) {
                                        NodeID new_node = H.new_node();
                                        H.setNodeWeight(new_node, reduced_graph.getNodeWeight(node));
                                        forall_out_edges (reduced_graph, e, node) {
                                                auto target = reduced_graph.getEdgeTarget(e);
                                                if (!similars[target]) {
                                                        auto edge = H.new_edge(new_node, mapping[target]);
                                                        H.setEdgeWeight(edge, 1);
                                                }
                                        }
                                        endfor
                                        forall_out_links(reduced_graph, e2, node) {
                                                auto target = reduced_graph.getLinkTarget(e2);
                                                if (!similars[target]) {
                                                        auto edge = H.new_edge(new_node, mapping[target]);
                                                        H.setEdgeWeight(edge, 1);
                                                }
                                        }
                                        endfor
                                }
                        }
                        H.finish_construction();
                        print_core_graph(H.number_of_nodes(), H.number_of_edges());
                        MISConfig mis_config{
                            .seed = m2s_cfg.seed,
                            .time_limit = static_cast<double>(reverse_mapping.size()) /
                                          reduced_graph.number_of_nodes() *
                                          std::min(max_core_time_limit, mis_solve_time_limit - proc_timer.elapsed()),
                            .console_log = false,
                            .ils_iterations = 15000,
                            .force_cand = 4,
                            .check_sorted = true,
                            .sort_freenodes = true,
                            .perform_reductions = true,
                            .reduction_style = MISConfig::Reduction_Style::NORMAL};
                        branch_and_reduce_algorithm exact_solver(H, mis_config, true);
                        cout_handler::disable_cout();
                        bool optimal = exact_solver.run_branch_reduce();
                        cout_handler::enable_cout();

                        if (exact_solver.get_is_weight() > H_best_weight) {
                                exact_solver.apply_branch_reduce_solution(H);
                                forall_nodes (H, node) {
                                        if (H.getPartitionIndex(node) != best_solution[reverse_mapping[node]]) {
                                                if (H.getPartitionIndex(node) == 1) {
                                                        mis_solution_size += H.getNodeWeight(node);
                                                } else {
                                                        mis_solution_size -= H.getNodeWeight(node);
                                                }
                                                best_solution[reverse_mapping[node]] = H.getPartitionIndex(node);
                                        }
                                }
                                endfor
                                NodeWeight added = maximize_solution(reduced_graph, best_solution);
                                mis_solution_size += added;
                                m2s_log::instance()->set_best_size( get_solution_size());

                                std::fill(similars.begin(), similars.end(), true);
                                similar = similars.size();

                                if (increase_sim_thres * similarity_threshold < 1.0) {
                                        // increase similarity threshold again a little
                                        similarity_threshold *= increase_sim_thres;
                                        print_new_similarity_threshold();
                                }
                        } else if (optimal) {
                                // new similarity thresholds must be smaller the the current similarity_threshold
                                // since the core graph is already solved optimally
                                similarity_threshold = decrease_sim_thres * relative_similarity_to_best_solution();
                                print_new_similarity_threshold();
                        } else {
                                // problem was too hard (timeout) -> increase threshold
                                if (increase_sim_thres * similarity_threshold < 1.0) {
                                        // increase similarity threshold again a little
                                        similarity_threshold *= increase_sim_thres;
                                        print_new_similarity_threshold();
                                }
                                // forget recent rnp solutions until best solution
                                std::fill(similars.begin(), similars.end(), true);
                                similar = similars.size();
                        }
                } else {
                        //RED2PACK_SCOPED_TIMER("solving core");
                        // solve core with chils
                        double time_best = 0;
                        for (NodeID i = 0; i < similars.size(); ++i) {
                                similars[i] = !similars[i];
                        }
                        chils::graph* H =
                            chils::build_induced_subgraph(reduced_graph, similars, reverse_mapping, mapping);
                        print_core_graph(H->n, H->V[H->n]);
                        // set warm start solution
                        for (NodeID i = 0; i < reverse_mapping.size(); ++i) {
                                solution[i] = best_solution[reverse_mapping[i]];
                        }

                        chils::run_chils_impl(
                            H, solution, time_best,
                            chils::chils_config{
                                .verbose = true,
                                .warm_start = true,
                                .run_chils = static_cast<int>(max_chils),
                                .max_queue = 32,
                                .timeout = static_cast<double>(reverse_mapping.size()) /
                                           reduced_graph.number_of_nodes() *
                                           std::min(max_core_time_limit, mis_solve_time_limit - proc_timer.elapsed()),
                                .step = 1,
                                .il = std::numeric_limits<long long>::max(),
                                .path_offset = 0,
                                .path_end = 0,
                                .seed = static_cast<unsigned>(m2s_cfg.seed)
                            });
                        chils::graph_free(H);

                        NodeWeight new_H_weight = 0;
                        for (NodeID i = 0; i < reverse_mapping.size(); ++i) {
                                if (solution[i]) {
                                        new_H_weight += reduced_graph.getNodeWeight(reverse_mapping[i]);
                                }
                        }
                        if (new_H_weight > H_best_weight) {
                                for (NodeID i = 0; i < reverse_mapping.size(); ++i) {
                                        best_solution[reverse_mapping[i]] = solution[i];
                                }
                                mis_solution_size += new_H_weight - H_best_weight;
                                NodeWeight added = maximize_solution(reduced_graph, best_solution);
                                mis_solution_size += added;
                                m2s_log::instance()->set_best_size( get_solution_size());

                                std::fill(similars.begin(), similars.end(), true);
                                similar = similars.size();

                                if (increase_sim_thres * similarity_threshold < 1.0) {
                                        // increase similarity threshold again a little
                                        similarity_threshold *= increase_sim_thres;
                                        print_new_similarity_threshold();
                                }
                        } else {
                                // solution could not be improved; forget latest rnp solutions sind best solution
                                std::fill(similars.begin(), similars.end(), true);
                                similar = similars.size();
                        }
                }
        }

        // set best_solution as solution for reduced_graph
        for (NodeID v = 0; v < reduced_graph.number_of_nodes(); ++v) {
                solution_status[former_node_id[v]] = best_solution[v];
        }

        return false;
}

}  // namespace red2pack
