#ifndef INC_2_PACKING_SET_M2S_LOG_H
#define INC_2_PACKING_SET_M2S_LOG_H

#include "red2pack/algorithms/kernel/weighted_reductions.h"
#include "red2pack/data_structure/m2s_graph_access.h"
#include "red2pack/m2s_config.h"
#include "red2pack/tools/timer.h"

#include <sstream>

namespace red2pack {
class m2s_log {
       public:
        /**
         * Get the singleton logger instance.
         *
         * @return Instance of the logger.
         */
        static m2s_log *instance() {
                static m2s_log inst;
                return &inst;
        };

        /**
         * Set config to set logging settings
         *
         * @param config Config for the evolutionary algorithm.
         */
        void set_config(M2SConfig &config);

        /**
         * Set the graph.
         *
         * @param G Graph representation.
         */
        void set_graph(m2s_graph_access &G);

        /**
         * Print information about the graph.
         */
        void print_graph();

        /**
         * Print the current config.
         */
        void print_config();

        /**
         * Print information about the reduction step.
         * Includes number of extracted nodes and resulting kernel size
         *
         * @param extracted_nodes Number of removed nodes.
         * @param kernel_size Number of remaining nodes.
         * @param kernel_size_m Number of edges in the kernel.
         * @param kernel_size_m2 Number of links/2-edges in the kernel.
         */
        void print_reduction(unsigned int extracted_nodes, unsigned int kernel_size, unsigned int kernel_size_m,
                             unsigned int kernel_size_m2);
        void print_transformed_graph(unsigned int n, unsigned int m, double time);

        /**
         * Print the final results.
         */
        void print_results();

        /**
         * Restart the timer for the total time including IO, etc.
         */
        void restart_total_timer();

        /**
         * Restart the timer for the algorithm.
         */
        void restart_timer();

        /**
         * Get the timer for the algorithm.
         */
        double get_timer();

        timer get_timer_copy() {
                return t;
        }

        /**
         * Update the size of the best solution.
         *
         * @param size Candidate to replace the best solution size.
         */
        void set_best_size(unsigned int size);
        void set_best_size(unsigned int size, double best_time);
        NodeWeight get_best_size() const;;

        /**
         * Update the size of the best solution.
         *
         * @param m2s_config Config for the logger.
         * @param size Candidate to replace the best solution size.
         */
        void finish_solving();

        void add_reduced_nodes_mw2ps(mw2ps_reduction_type reduction_type, NodeID reduced_nodes, double duration);
        static const char* mw2ps_reduction_to_string(mw2ps_reduction_type reductionType) noexcept;
        void print_reduced_nodes_mw2ps();

        static void log_time(unsigned level, std::string description, double time_in_sec) {
                std::cout << std::string(level, '-')  <<  description << ": " << time_in_sec << std::endl;
        }

       private:
        // General information
        timer total_timer; // starts at the very first beginning before I/Os start.
        timer t; // starts at the beginning of the benchmark
        M2SConfig log_config;

        // Graph informations
        NodeID number_of_nodes;
        EdgeID number_of_edges;

        // mw2ps reductions
        std::vector<NodeID> reduced_nodes_mw2ps;
        std::vector<double> reduced_nodes_mw2ps_t;

        unsigned int best_solution_size;

        double time_taken_best; // time to find best_solution_size
        double time_solve; // overall time of benchmark
        double total_time; // total time of running program

        /**
         * Default Constructor.
         */
        m2s_log();

        /**
         * Default Destructor.
         */
        virtual ~m2s_log();

};
}  // namespace two_packing_set

#endif  // INC_2_PACKING_SET_M2S_LOG_H
