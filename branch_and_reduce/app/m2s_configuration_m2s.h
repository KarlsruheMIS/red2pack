/**
 * configuration.h
 * Purpose: Contains preset configurations for the evolutionary algorithms.
 *
 *****************************************************************************/
#ifndef INC_2_PACKING_SET_M2S_CONFIGURATION_M2S_H
#define INC_2_PACKING_SET_M2S_CONFIGURATION_M2S_H

#include "definitions.h"
#include "m2s_config.h"

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
        m2s_config.time_limit = 1000.0;
        // Randomization
        m2s_config.seed = 0;
        // Output
        m2s_config.console_log = false;
        m2s_config.write_2ps = false;
        // Reductions
        m2s_config.reduction_style2 = two_packing_set::M2SConfig::Reduction_Style2::elaborated;
        m2s_config.disable_twin = false;
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
        m2s_config.on_demand_two_neighborhood = false;
}

#endif  // INC_2_PACKING_SET_M2S_CONFIGURATION_M2S_H
