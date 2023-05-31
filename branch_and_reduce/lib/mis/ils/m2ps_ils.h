/**
 * m2ps_ils.h
 * Purpose: Perform the iterated local search (m2ps_ils) as described by Andrade et al.
 *
 * The original code from Andrade et al. was kindly provided by Renato Werneck.
 *
 *****************************************************************************/

#ifndef _m2ps_ils_H_
#define _m2ps_ils_H_

#include <vector>

#include "timer.h"
#include "m2s_config.h"
#include "population_mis.h"
#include "m2ps_local_search.h"
#include "data_structure/graph_access.h"

class m2ps_ils {
    public:
        /**
         * Default Constructor.
         */
        m2ps_ils();

        /**
         * Default Destructor.
         */
        virtual ~m2ps_ils();

        /**
         * Main algorithm of the m2ps_ils.
         * More detailed information can be found in the original paper
         * of Andrade et al.
         *
         * @param config Config used for the m2ps_ils.
         * @param G Graph representation.
         * @param iteration_limit Maximum number of iterations.
         */
        void perform_m2ps_ils(M2SConfig & config, graph_access & G, unsigned int iteration_limit = 0);

        /** 
         * Reset the m2ps_ils.
         * Clears the force-list and best solution.
         */
        void reset();

    private:
        // Array for storing the last time a node was forced in the solution.
        std::vector<NodeID> last_forced;
        // Main solution data structure.
        mis_permutation *perm;
        // List of candidates for the local search.
        candidate_list *cand;
        // List of nodes that were forced in the solution.
        candidate_list *force_list;
        // List of onetight nodes.
        candidate_list *one;
        // Best solution found so far
        individuum_mis best_solution;
        // Population used to create the initial solution
        population_mis pop;
        // Local search algorithm
        m2ps_local_search local;
        // Timer for measuring the time taken for the m2ps_ils.
        timer t;

        // m2ps_ils config
        unsigned int plateau_down;
        unsigned int plateau_up;
        unsigned int plateau_best;
        unsigned int plateau_best_again;
        unsigned int pden_floor;
        unsigned int delta_penalty;
        bool limit_plateau;
        bool swap_on_failure;

        /**
         * Force the given node in the solution.
         *
         * @param config Config used for the m2ps_ils.
         * @param G Graph representation.
         * @param v Node to force into the solution.
         * @param force_list List storing the forced nodes.
         */
        void force(M2SConfig & config, graph_access & G, NodeID v, candidate_list *force_list = NULL);

        /**
         * Undo all operations currently stored in the operation log.
         *
         * @param G Graph representation
         */
        void unwind(graph_access & G);
};
#endif
