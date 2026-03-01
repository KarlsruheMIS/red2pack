/**
 * configuration.h
 * Purpose: Contains preset configurations for the evolutionary algorithms.
 *
 *****************************************************************************/
#ifndef INC_2_PACKING_SET_M2S_CONFIGURATION_M2S_H
#define INC_2_PACKING_SET_M2S_CONFIGURATION_M2S_H

#include <red2pack/m2s_config.h>

class m2s_configuration_m2s {
       public:
        /**
         * Default Constructor.
         */
        m2s_configuration_m2s(){};

        /**
         * Default Destructor.
         */
        virtual ~m2s_configuration_m2s(){};

        /**
         * Set the standard configuration for unweighted graphs.
         *
         * @param config Config to be initialized.
         */
        void standard_unweighted(red2pack::M2SConfig &config);


        /**
         * Set the standard configuration for weighted graphs.
         *
         * @param config Config to be initialized.
         */
        void standard_weighted(red2pack::M2SConfig &config);
};

inline void m2s_configuration_m2s::standard_unweighted(red2pack::M2SConfig &m2s_config) {
        // Basic
        m2s_config.time_limit = 1000.0;
        // Randomization
        m2s_config.seed = 0;
        // Output
        m2s_config.silent = false;
        m2s_config.write_result = false;
        m2s_config.write_transformed = false;
        // Reductions
        m2s_config.reduction_style = red2pack::M2SConfig::Reduction_Style2::fast;
        m2s_config.disable_twin = false;
        m2s_config.disable_deg_two = false;
        m2s_config.disable_domination = false;
        m2s_config.disable_fast_domination = false;
        m2s_config.disable_deg_one = false;
        m2s_config.disable_clique = false;
 
        m2s_config.on_demand_two_neighborhood = true;
        m2s_config.maintain_two_neigh = true;

}

inline void m2s_configuration_m2s::standard_weighted(red2pack::M2SConfig &m2s_config) {
        // Basic
        m2s_config.time_limit = 1000.0;
        // Randomization
        m2s_config.seed = 0;
        // Output
        m2s_config.silent = false;
        m2s_config.write_result = false;
        m2s_config.write_transformed = false;
        // Weighted Reductions
        m2s_config.reduction_style = red2pack::M2SConfig::Reduction_Style2::strong;
        m2s_config.disable_fast_neighborhood_removal = false;
        m2s_config.disable_fast_degree_one_removal = true;
        m2s_config.disable_fast_degree_two_removal = false;
        m2s_config.disable_fast_complete_degree_one_removal = false;
        m2s_config.disable_neighborhood_removal = false;
        m2s_config.disable_split_neighbor_removal = false;
        m2s_config.disable_wdomination = false;
        m2s_config.disable_fast_wdomination = false;
        m2s_config.disable_split_intersection_removal = false;
        m2s_config.disable_weight_transfer = false;
        m2s_config.disable_old_weight_transfer = true;
        m2s_config.disable_direct_neighbor_removal = false;
        m2s_config.disable_two_neighbor_removal = false;
        m2s_config.disable_single_fast_domination = false;
        m2s_config.disable_neighborhood_folding = false;
        m2s_config.disable_fold2 = false;
        m2s_config.disable_all = false;

        m2s_config.on_demand_two_neighborhood = true;
        m2s_config.maintain_two_neigh = true;

        // used in DRP and RnP
        m2s_config.max_chils = 1;
        m2s_config.max_core_time_limit = 80;
        m2s_config.similarity_threshold = 0.6;
        m2s_config.increase_sim_thres = 1.0;
        m2s_config.decrease_sim_thres = 1.0;
        m2s_config.max_max_num_candidates = 50;
        m2s_config.perturb_prob = 0.5;
        m2s_config.max_num_candidates = 1;
        m2s_config.heuristic_decision_style = red2pack::two_pack_status::excluded;
        m2s_config.heuristic_rating = red2pack::mw2ps_heuristic_ratings::weight_diff;
        m2s_config.core_solver = red2pack::M2SConfig::Core_Solver::chils;

        // use initial IS in M(W)IS solver for transformed graph (not supported by all M(W)IS solvers)
        m2s_config.perform_IS_reductions = false;
}

#endif  // INC_2_PACKING_SET_M2S_CONFIGURATION_M2S_H
