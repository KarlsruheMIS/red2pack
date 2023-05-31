/**
 * m2s_config.h
 * Purpose: Configuration used for the evolutionary maximum independent set algorithms.
 *
 *****************************************************************************/

#ifndef _M2S_CONFIG_H_
#define _M2S_CONFIG_H_

#include <string>

#include "definitions.h"

// Configuration for the calculation of the MIS
struct M2SConfig {
    enum Reduction_Style2 {initial, extended, compact};
    // Name of the graph file.
    std::string graph_filename;
    // Name of the output file.
    std::string output_filename;
    // Name of the output kernel file.
    std::string kernel_file_name;
    // Seed for the RNG.
    int seed;
    // Size of the population used.
    unsigned int population_size;
    // Imbalance. Used for the KaHIP-interface calls.
    double imbalance;
    // Mode for the KaHIP-framework.
    unsigned int kahip_mode;
    // Use full kernelization (true) or FastKer (false)
    bool fullKernelization;
    // Time limit for the evolutionary algorithm
    double time_limit;
    // Number of repetitions in each round.
    unsigned int repetitions;
    // Insert a solution if no new solution has been inserted
    // for this amount of operations.
    unsigned int insert_threshold;
    // Update the pool of node separators.
    unsigned int pool_threshold;
    // Factor for timing based renewal of the separator.
    // Time after building > Factor * Time taken for building triggers renewal.
    double pool_renewal_factor;
    // Should the imbalance be randomized?
    bool randomize_imbalance;
    // Diversify the initial solution?
    bool diversify;
    // Tournament or random selection of individuals?
    bool enable_tournament_selection;
    // Use the vertex cover approach for the multiway combine operator.
    bool use_multiway_vc;
    // Number of blocks used in multiway operators.
    unsigned int multiway_blocks;
    // Number of individuals in a tournament
    unsigned int tournament_size;
    // Percentage for the mutation of a solution.
    int flip_coin;
    // Force parameter for the mutation.
    unsigned int force_k;
    // Number of candidates for forced insertion.
    unsigned int force_cand;
    // Number of initial node separators to be constructed.
    unsigned int number_of_separators;
    // Number of initial partitions to be constructed.
    unsigned int number_of_partitions;
    // Number of initial k-separators to be constructed.
    unsigned int number_of_k_separators;
    // Number of initial k-partitions to be constructed.
    unsigned int number_of_k_partitions;
    // Print result of each repetition.
    bool print_repetition;
    // Print the population after each round.
    bool print_population;
    // Write the log into a file
    bool print_log;
    // Write the inpendent set into a file
    bool write_graph;
    // Write the log into the console
    bool console_log;
    // Number of iterations for the ILS.
    unsigned int ils_iterations;
    // Use the Hopcroft-Karp algorithm to fix vertex cover candidates
    bool use_hopcroft;
    // Lower bound for the change rate between best individuals
    double best_limit;
    // Fraction of independent set nodes that should be removed before reduction
    double remove_fraction;
    // Optimize candidates for ILS.
    bool optimize_candidates;
    // Remove IS nodes from best individual before recursive reduction
    bool extract_best_nodes;
    // Apply all reductions to reduce the graph size
    bool all_reductions;
    // Threshold for reducing the graph
    unsigned int reduction_threshold;
    // Check graph sortedness
    bool check_sorted;
    // Use adaptive greedy starting solution
    bool start_greedy_adaptive;


    //turn of single reductions 
    bool disable_domination;
    /* bool disable_domination_e; */
    bool disable_fast_domination;
    /* bool disable_fast_domination_e; */
    bool disable_deg_one;
    /* bool disable_deg_one_e; */
    bool disable_clique;
    /* bool disable_clique_e; */
    bool disable_cycle;
    /* bool disable_cycle_e; */
    bool disable_neighborhood;
    /* bool disable_neighborhood_e; */
    bool disable_twin;
    /* bool disable_twin_e; */

    Reduction_Style2 reduction_style2;

    void set2ReductionStyle(const std::string& redu_style) {
        if (strCompare(redu_style, "initial")) {
            reduction_style2 = Reduction_Style2::initial;
        } else if (strCompare(redu_style, "compact")) { 
            reduction_style2 = Reduction_Style2::compact;
        } else if (strCompare(redu_style, "extended")) { 
            reduction_style2 = Reduction_Style2::extended;
        } else {
            reduction_style2 = Reduction_Style2::initial;
        }

    }

    private:

    bool strCompare(const std::string & str1, const std::string & str2) {
        return str1.size() == str2.size() && std::equal(str1.begin(), str1.end(), str2.begin(), [](unsigned char c1, unsigned char c2){return std::toupper(c1) == std::toupper(c2); });
    }
};

#endif
