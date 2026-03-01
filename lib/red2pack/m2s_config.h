/**
 * m2s_config.h
 * Purpose: Configuration used for the evolutionary maximum independent set algorithms.
 *
 *****************************************************************************/

#ifndef _M2S_CONFIG_H_
#define _M2S_CONFIG_H_

#include <stdexcept>
#include <string>

#include "red2pack_def.h"

namespace red2pack {
// Configuration for the calculation of the M(W)2S
struct M2SConfig {
        enum Reduction_Style2 { strong, full, fast, heuristic, main };
        enum Core_Solver { none, exact, chils };
        // Name of the graph file.
        std::string graph_filename;
        // Name of the output file.
        std::string output_filename;
        // Name of the output reduced-and-transformed graph file.
        std::string transformed_graph_filename;

        bool console_log;
        bool silent;
        bool write_result;
        bool write_transformed;
        int seed;
        double time_limit;

        // whether mis solver should perform independent set reduction at the beginning
        bool perform_IS_reductions;

        // turn of single reductions
        bool disable_domination;
        bool disable_fast_domination;
        bool disable_deg_one;
        bool disable_clique;
        bool disable_deg_two;
        bool disable_twin;

        // turn of single weighted reductions
        bool disable_fast_neighborhood_removal;
        bool disable_fast_degree_one_removal;
        bool disable_fast_degree_two_removal;
        bool disable_fast_complete_degree_one_removal;
        bool disable_fast_direct_neighbor_removal;
        bool disable_neighborhood_removal;
        bool disable_split_neighbor_removal;
        bool disable_wdomination;
        bool disable_fast_wdomination;
        bool disable_split_intersection_removal;
        bool disable_weight_transfer;
        bool disable_old_weight_transfer;
        bool disable_direct_neighbor_removal;
        bool disable_two_neighbor_removal;
        bool disable_single_fast_domination;
        bool disable_neighborhood_folding;
        bool disable_fold2;
        bool disable_all;


        // Initialize 2-neighborhoods first when they are needed
        bool on_demand_two_neighborhood;
        // Maintain Two neighborhood once it was initialized; otherwise it re-computed each time.
        bool maintain_two_neigh;


        // DRP
        double max_chils;
        double max_core_time_limit;
        double similarity_threshold;
        double increase_sim_thres;
        double decrease_sim_thres;
        unsigned max_max_num_candidates;
        Core_Solver core_solver;

        // DRP and RnP
        unsigned max_num_candidates;
        bool adaptive_rating;
        double perturb_prob;
        bool shuffle_redu_order;

        bool rnp_warm_start_for_kernel = false;  // restore heuristic 2ps reduction decision, set heuristic solution for kernel

        // include best/exclude worst candidates w.r.t. rating
        two_pack_status heuristic_decision_style;
        mw2ps_heuristic_ratings heuristic_rating;

        Reduction_Style2 reduction_style;

        bool uses_heuristic_reduce_style() const {
                return reduction_style == Reduction_Style2::heuristic;
        }

        void setUnweightedReductionStyle(const std::string& redu_style) {
                if (strCompare(redu_style, "main")) {
                        reduction_style = Reduction_Style2::main;
                } else if (strCompare(redu_style, "fast")) {
                        reduction_style = Reduction_Style2::fast;
                } else {
                        throw std::invalid_argument("received invalid reduction style. Choose between main and fast.");
                }
        }
        void setWeightedReductionStyle(const std::string& redu_style) {
                if (strCompare(redu_style, "main")) {
                        reduction_style = Reduction_Style2::main;
                } else if (strCompare(redu_style, "full")) {
                        reduction_style = Reduction_Style2::full;
                } else if (strCompare(redu_style, "strong")) {
                        reduction_style = Reduction_Style2::strong;
                } else if (strCompare(redu_style, "fast")) {
                        reduction_style = Reduction_Style2::fast;
                } else if (strCompare(redu_style, "heuristic")) {
                        reduction_style = Reduction_Style2::heuristic;
                } else {
                        throw std::invalid_argument("received invalid reduction style. Choose between core, full, strong, and heuristic (rnp).");
                }
        }

        void setCoreSolver(const std::string& c_solver) {
                if(strCompare(c_solver, "none")){
                        core_solver = Core_Solver::none;
                }else if(strCompare(c_solver, "exact")) {
                        core_solver = Core_Solver::exact;
                }else if(strCompare(c_solver, "chils")) {
                        core_solver = Core_Solver::chils;
                }else {
                        core_solver = Core_Solver::chils;
                }
        }

        void setHeuristicDecisionStyle(const std::string& decision_style) {
                if (strCompare(decision_style, "included")) {
                        heuristic_decision_style = two_pack_status::included;
                } else if (strCompare(decision_style, "excluded")) {
                        heuristic_decision_style = two_pack_status::excluded;
                } else {
                        heuristic_decision_style = two_pack_status::excluded;
                }
        }

        void setHeuristicRating(const std::string& heuristic_style) {
                if (strCompare(heuristic_style, "weight")) {
                        heuristic_rating = mw2ps_heuristic_ratings::weight;
                } else if (strCompare(heuristic_style, "deg")) {
                        heuristic_rating = mw2ps_heuristic_ratings::deg;
                } else if (strCompare(heuristic_style, "weight-diff")) {
                        heuristic_rating = mw2ps_heuristic_ratings::weight_diff;
                } else {
                        heuristic_rating = mw2ps_heuristic_ratings::weight_diff;
                }
        }

        bool use_weighted_reductions() const {
                return uses_heuristic_reduce_style() ||
                       (!disable_all &&
                        !(disable_fast_neighborhood_removal && disable_fast_degree_one_removal &&
                          disable_fast_complete_degree_one_removal && disable_fast_degree_two_removal &&
                          disable_fast_direct_neighbor_removal && disable_neighborhood_removal &&
                          disable_split_neighbor_removal && disable_wdomination && disable_split_intersection_removal &&
                          disable_weight_transfer && disable_direct_neighbor_removal && disable_two_neighbor_removal &&
                          disable_single_fast_domination && disable_neighborhood_folding && disable_fold2));
        }

       private:
        bool strCompare(const std::string& str1, const std::string& str2) {
                return str1.size() == str2.size() &&
                       std::equal(str1.begin(), str1.end(), str2.begin(), [](unsigned char c1, unsigned char c2) {
                               return std::toupper(c1) == std::toupper(c2);
                       });
        }
};
}  // namespace red2pack

#endif
