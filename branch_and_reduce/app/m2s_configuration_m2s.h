/**
 * configuration.h
 * Purpose: Contains preset configurations for the evolutionary algorithms.
 *
 *****************************************************************************/
#ifndef INC_2_PACKING_SET_M2S_CONFIGURATION_M2S_H
#define INC_2_PACKING_SET_M2S_CONFIGURATION_M2S_H

#include "definitions.h"
#include "m2s_config.h"

/* #ifdef OM2S */
const int FAST = 0;
const int ECO = 1;
const int STRONG = 2;
const int FASTSOCIAL = 3;
const int ECOSOCIAL = 4;
const int STRONGSOCIAL = 5;
/* #endif */

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
         * Set the standard configuration.
         * Use local search for combine operations.
         * Use ILS for improving offsprings.
         *
         * @param config Config to be initialized.
         */
        void standard(two_packing_set::M2SConfig &config);

        /**
         * Set the configuration for social network graphs.
         * Use local search for combine operations.
         * Use ILS for improving offsprings.
         *
         * @param config Config to be initialized.
         */
        void social(two_packing_set::M2SConfig &config);

        /**
         * Set the configuration for the experimental evaluation.
         * Use local search for combine operations.
         * Use ILS for improving offsprings.
         *
         * @param config Config to be initialized.
         */
        void full_standard(two_packing_set::M2SConfig &config);

        /**
         * Set the configuration for the experimental evaluation
         * for social network graphs.
         * Use local search for combine operations.
         * Use ILS for improving offsprings.
         *
         * @param config Config to be initialized.
         */
        void full_social(two_packing_set::M2SConfig &config);
};

inline void m2s_configuration_m2s::standard(two_packing_set::M2SConfig &m2s_config) {
        // Basic
        m2s_config.population_size = 50;
        m2s_config.repetitions = 50;
        m2s_config.time_limit = 1000.0;
        // KaHIP
        m2s_config.kahip_mode = FAST;
        // Randomization
        m2s_config.seed = 0;
        m2s_config.diversify = true;
        m2s_config.imbalance = 0.03;
        m2s_config.randomize_imbalance = true;
        // Selection
        m2s_config.enable_tournament_selection = true;
        m2s_config.tournament_size = 2;
        // Mutation
        m2s_config.flip_coin = 1;
        // Combination
        m2s_config.use_hopcroft = false;
        m2s_config.optimize_candidates = true;
        // Multiway
        m2s_config.use_multiway_vc = false;
        m2s_config.multiway_blocks = 64;
        // Thresholds
        m2s_config.insert_threshold = 150;
        m2s_config.pool_threshold = 250;
        m2s_config.pool_renewal_factor = 10.0;
        // Separator pool
        m2s_config.number_of_separators = 10;
        m2s_config.number_of_partitions = 10;
        m2s_config.number_of_k_separators = 10;
        m2s_config.number_of_k_partitions = 10;
        // Output
        m2s_config.print_repetition = true;
        m2s_config.print_population = false;
        m2s_config.console_log = false;
        m2s_config.check_sorted = true;
        // ILS
        m2s_config.ils_iterations = 15000;
        m2s_config.force_k = 1;
        m2s_config.force_cand = 4;
        // Reductions
        m2s_config.all_reductions = true;
        m2s_config.reduction_style2 = two_packing_set::M2SConfig::Reduction_Style2::initial;
        // Convergence
        m2s_config.reduction_threshold = 350;
        m2s_config.remove_fraction = 0.10;
        m2s_config.extract_best_nodes = true;
        // Initial solution
        m2s_config.start_greedy_adaptive = false;
        m2s_config.disable_twin = false;
        /* m2s_config.disable_twin_e = false; */
        m2s_config.disable_neighborhood = false;
        /* m2s_config.disable_neighborhood_e = false; */
        m2s_config.disable_deg_two = false;
        /* m2s_config.disable_cycle_e = false; */
        m2s_config.disable_domination = false;
        /* m2s_config.disable_domination_e = false; */
        m2s_config.disable_fast_domination = false;
        /* m2s_config.disable_fast_domination_e = false; */
        m2s_config.disable_deg_one = false;
        /* m2s_config.disable_deg_one_e = false; */
        m2s_config.disable_clique = false;
        /* m2s_config.disable_clique_e = false; */
}

inline void m2s_configuration_m2s::social(two_packing_set::M2SConfig &m2s_config) {
        standard(m2s_config);
        m2s_config.kahip_mode = FASTSOCIAL;
}

inline void m2s_configuration_m2s::full_standard(two_packing_set::M2SConfig &m2s_config) {
        standard(m2s_config);
        m2s_config.population_size = 250;
        m2s_config.time_limit = 36000.0;
        m2s_config.number_of_separators = 30;
        m2s_config.number_of_partitions = 30;
        m2s_config.number_of_k_separators = 30;
        m2s_config.number_of_k_partitions = 30;
        m2s_config.flip_coin = 10;
        m2s_config.pool_threshold = 200;
        m2s_config.reduction_threshold = 1000;
}

inline void m2s_configuration_m2s::full_social(two_packing_set::M2SConfig &m2s_config) {
        full_standard(m2s_config);
        m2s_config.kahip_mode = FASTSOCIAL;
}

#endif  // INC_2_PACKING_SET_M2S_CONFIGURATION_M2S_H
