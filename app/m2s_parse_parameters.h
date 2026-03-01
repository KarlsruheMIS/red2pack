/**
 * m2s_parse_parameters.h
 * Purpose: Parse command line parameters.
 *
 *****************************************************************************/

#pragma once

#include <red2pack/m2s_config.h>

#include <argtable3.h>
#include <stdio.h>
#include <string.h>

#include <iostream>
#include <stdexcept>

namespace red2pack::cli {

#ifndef NDEBUG
#define USE_EXTENDED_PARAMS
#define EXTENDED_BASE_PARAMS 0
#define EXTENDED_UNWEIGHTED_PARAMS 6
#define EXTENDED_WEIGHTED_PARAMS 16
#else
#define EXTENDED_BASE_PARAMS 0
#define EXTENDED_UNWEIGHTED_PARAMS 0
#define EXTENDED_WEIGHTED_PARAMS 0
#endif

struct base_cli {
        static constexpr int ARGTABLE_SLOTS = 6 + EXTENDED_BASE_PARAMS;

        // base parameters
        struct arg_lit *help;
        struct arg_str *filename;
        struct arg_int *user_seed;
        struct arg_dbl *time_limit;
        struct arg_lit *silent;
        struct arg_str *rnt_graph;

#ifdef USE_EXTENDED_PARAMS

#endif

        struct arg_end *end;
        int end_pos = 0;

        void set_end(void *argtable[], int end_pos) {
                end=arg_end(100);
                this->end_pos=end_pos;
                argtable[end_pos]= end;
        }


        virtual void init_cli(void *argtable[]) {
                // init parameters
                help = arg_lit0(NULL, "help", "Print help.");
                filename = arg_strn(NULL, NULL, "FILE", 1, 1, "Path to graph file.");
                user_seed = arg_int0(NULL, "seed", NULL, "Seed to use for the PRNG.");
                time_limit = arg_dbl0(NULL, "time_limit", NULL, "Time limit in s. Default: 1000s.");
                silent = arg_lit0(NULL, "silent", "Suppress program output.");
                rnt_graph = arg_str0(NULL, "output_rnt_graph", NULL, "Path to store resulting reduced-and-transformed graph.");

                // set parameters in argtable
                int table_pos = 0;
                argtable[table_pos++] = help;
                argtable[table_pos++] = filename;
                argtable[table_pos++] = user_seed;
                argtable[table_pos++] = time_limit;
                argtable[table_pos++] = silent;
                argtable[table_pos++] = rnt_graph;
        }

        virtual int parse_cli(int argc, char** argv, M2SConfig &m2s_config, std::string &graph_filepath, const std::string &progname,
                              void *argtable[]) {

                int nerrors = arg_parse(argc, argv, argtable);

                // parse parameters
                if (help->count > 0) {
                        printf("Usage: %s", progname.c_str());
                        arg_print_syntax(stdout, argtable, "\n");
                        arg_print_glossary(stdout, argtable, "  %-40s %s\n");
                        return 1;
                }

               if (nerrors > 0) {
                        arg_print_errors(stderr, end, progname.c_str());
                        printf("Try '%s --help' for more information.\n", progname.c_str());
                        return 1;
                }

                if (filename->count > 0) {
                        graph_filepath = filename->sval[0];
                        m2s_config.graph_filename = graph_filepath.substr(graph_filepath.find_last_of('/') + 1);
                }

                if (user_seed->count > 0) {
                        m2s_config.seed = user_seed->ival[0];
                }

                if (time_limit->count > 0) {
                        m2s_config.time_limit = time_limit->dval[0];
                }

                if (silent->count > 0) {
                        m2s_config.silent = true;
                }

                if (rnt_graph->count > 0) {
                        m2s_config.transformed_graph_filename = rnt_graph->sval[0];
                        m2s_config.write_transformed = true;
                } else {
                        m2s_config.write_transformed = false;
                }

                return 0;
        }
};

struct unweighted_cli : public base_cli {
        static constexpr int ARGTABLE_SLOTS = 2 + base_cli::ARGTABLE_SLOTS + EXTENDED_UNWEIGHTED_PARAMS;

        // unweighted parameters
        struct arg_str *reduction_style;
        struct arg_str *output;


#ifdef USE_EXTENDED_PARAMS
        struct arg_lit *disable_deg_one;
        struct arg_lit *disable_clique;
        struct arg_lit *disable_cycle;
        struct arg_lit *disable_twin;
        struct arg_lit *disable_domination;
        struct arg_lit *disable_fast_domination;
#endif

        void init_cli(void *argtable[]) override {
                base_cli::init_cli(argtable);

                // init parameters
                reduction_style = arg_str0(NULL, "reduction_style", NULL, "Choose between elaborated and core.");
                output = arg_str0(NULL, "output", NULL, "Path to store resulting 2-packing-set.");

#ifdef USE_EXTENDED_PARAMS
                disable_deg_one = arg_lit0(NULL, "disable_deg_one", "disable reduction deg_one");
                disable_clique = arg_lit0(NULL, "disable_clique", "disable reduction clique");
                disable_cycle = arg_lit0(NULL, "disable_deg_two", "disable reduction deg_two");
                disable_twin = arg_lit0(NULL, "disable_twin", "disable reduction twin");
                disable_domination = arg_lit0(NULL, "disable_domination", "disable reduction domination");
                disable_fast_domination =
                    arg_lit0(NULL, "disable_fast_domination", "disable reduction fast_domination");
#endif

                // set parameters in argtable
                int table_pos = base_cli::ARGTABLE_SLOTS;
                argtable[table_pos++] = reduction_style;
                argtable[table_pos++] = output;

#ifdef USE_EXTENDED_PARAMS
                argtable[table_pos++] = disable_deg_one;
                argtable[table_pos++] = disable_clique;
                argtable[table_pos++] = disable_cycle;
                argtable[table_pos++] = disable_twin;
                argtable[table_pos++] = disable_domination;
                argtable[table_pos++] = disable_fast_domination;
#endif

                return;
        }

        int parse_cli(int argc, char** argv, red2pack::M2SConfig &m2s_config, std::string &graph_filepath, const std::string &progname, void *argtable[]) override {
                int error = base_cli::parse_cli(argc, argv, m2s_config, graph_filepath, progname, argtable);
                if (error > 0) {
                        return error;
                }

                if (reduction_style->count > 0) {
                  try {
                        m2s_config.setUnweightedReductionStyle(reduction_style->sval[0]);
                  } catch ( const std::invalid_argument& e) {
                    std::cerr << "Error: " << e.what() << std::endl;
                    return 1;
                  }
                }

                if (output->count > 0) {
                        m2s_config.output_filename = output->sval[0];
                        m2s_config.write_result = true;
                } else {
                        m2s_config.write_result = false;
                }

#ifdef USE_EXTENDED_PARAMS
                if (disable_deg_one->count > 0) m2s_config.disable_deg_one = true;
                if (disable_clique->count > 0) m2s_config.disable_clique = true;
                if (disable_cycle->count > 0) m2s_config.disable_deg_two = true;
                if (disable_twin->count > 0) m2s_config.disable_twin = true;
                if (disable_domination->count > 0) m2s_config.disable_domination = true;
                if (disable_fast_domination->count > 0) m2s_config.disable_fast_domination = true;
#endif

                return 0;
        }
};

struct weighted_cli : public base_cli {
        static constexpr int ARGTABLE_SLOTS = 1 + base_cli::ARGTABLE_SLOTS + EXTENDED_WEIGHTED_PARAMS;

        // weighted parameters
        struct arg_str *reduction_style;

#ifdef USE_EXTENDED_PARAMS
        struct arg_lit *disable_fast_neighborhood_removal;
        struct arg_lit *disable_fast_complete_degree_one_removal;
        struct arg_lit *disable_fast_degree_two_removal;
        struct arg_lit *disable_neighborhood_removal;
        struct arg_lit *disable_split_neighbor_removal;
        struct arg_lit *disable_wdomination;
        struct arg_lit *disable_fast_wdomination;
        struct arg_lit *disable_split_intersection_removal;
        struct arg_lit *disable_weight_transfer;
        struct arg_lit *enable_old_weight_transfer;
        struct arg_lit *disable_direct_neighbor_removal;
        struct arg_lit *disable_two_neighbor_removal;
        struct arg_lit *disable_single_fast_domination;
        struct arg_lit *disable_neighborhood_folding;
        struct arg_lit *disable_fold2;
        struct arg_lit *disable_all;
#endif

        void init_cli(void *argtable[]) override {
                base_cli::init_cli(argtable);

                // init parameters
                reduction_style =
                    arg_str0(NULL, "reduction_style", NULL, "Choose between full, strong, core, fast, and heuristic.");

                // set parameters in argtable
                int table_pos = base_cli::ARGTABLE_SLOTS;
                argtable[table_pos++] = reduction_style;

#ifdef USE_EXTENDED_PARAMS
                disable_fast_neighborhood_removal =
                    arg_lit0(NULL, "disable_fast_neighborhood_removal", "disables this reduction");
                disable_fast_complete_degree_one_removal =
                    arg_lit0(NULL, "disable_fast_complete_degree_one_removal", "disables this reduction");
                disable_fast_degree_two_removal =
                    arg_lit0(NULL, "disable_fast_degree_two_removal", "disables this reduction");
                disable_neighborhood_removal =
                    arg_lit0(NULL, "disable_neighborhood_removal", "disables this reduction");
                disable_split_neighbor_removal =
                    arg_lit0(NULL, "disable_split_neighbor_removal", "disables this reduction");
                disable_wdomination = arg_lit0(NULL, "disable_wdomination", "disables this reduction");
                disable_fast_wdomination = arg_lit0(NULL, "disable_fast_wdomination", "disables this reduction");
                disable_split_intersection_removal =
                    arg_lit0(NULL, "disable_split_intersection_removal", "disables this reduction");
                disable_weight_transfer = arg_lit0(NULL, "disable_weight_transfer", "disables this reduction");
                enable_old_weight_transfer = arg_lit0(NULL, "enable_old_weight_transfer", "enables this reduction");
                disable_direct_neighbor_removal =
                    arg_lit0(NULL, "disable_direct_neighbor_removal", "disables this reduction");
                disable_two_neighbor_removal =
                    arg_lit0(NULL, "disable_two_neighbor_removal", "disables this reduction");
                disable_single_fast_domination =
                    arg_lit0(NULL, "disable_single_fast_domination", "disables this reduction");
                disable_neighborhood_folding =
                    arg_lit0(NULL, "disable_neighborhood_folding", "disables this reduction");
                disable_fold2 = arg_lit0(NULL, "disable_fold2", "disables this reduction");
                disable_all = arg_lit0(
                    NULL, "disable_all",
                    "disables all weighted 2-packing set reductions for algorithms based on IS transformations");

                argtable[table_pos++] = disable_fast_neighborhood_removal;
                argtable[table_pos++] = disable_fast_complete_degree_one_removal;
                argtable[table_pos++] = disable_fast_degree_two_removal;
                argtable[table_pos++] = disable_neighborhood_removal;
                argtable[table_pos++] = disable_split_neighbor_removal;
                argtable[table_pos++] = disable_wdomination;
                argtable[table_pos++] = disable_fast_wdomination;
                argtable[table_pos++] = disable_split_intersection_removal;
                argtable[table_pos++] = disable_weight_transfer;
                argtable[table_pos++] = enable_old_weight_transfer;
                argtable[table_pos++] = disable_direct_neighbor_removal;
                argtable[table_pos++] = disable_two_neighbor_removal;
                argtable[table_pos++] = disable_single_fast_domination;
                argtable[table_pos++] = disable_neighborhood_folding;
                argtable[table_pos++] = disable_fold2;
                argtable[table_pos++] = disable_all;
#endif

                return;
        }

        int parse_cli(int argc, char** argv, red2pack::M2SConfig &m2s_config, std::string &graph_filepath, const std::string &progname,
                      void *argtable[]) override {
                int error = base_cli::parse_cli(argc, argv, m2s_config, graph_filepath, progname, argtable);
                if (error > 0) {
                        return error;
                }

                if (reduction_style->count > 0) {
                  try {
                        m2s_config.setWeightedReductionStyle(reduction_style->sval[0]);
                  } catch ( const std::invalid_argument& e) {
                    std::cerr << "Error: " << e.what() << std::endl;
                    return 1;
                  }
                }

#ifdef USE_EXTENDED_PARAMS
                if (disable_fast_neighborhood_removal->count > 0) m2s_config.disable_fast_neighborhood_removal = true;
                if (disable_fast_complete_degree_one_removal->count > 0)
                        m2s_config.disable_fast_complete_degree_one_removal = true;
                if (disable_fast_degree_two_removal->count > 0) m2s_config.disable_fast_degree_two_removal = true;
                if (disable_neighborhood_removal->count > 0) m2s_config.disable_neighborhood_removal = true;
                if (disable_split_neighbor_removal->count > 0) m2s_config.disable_split_neighbor_removal = true;
                if (disable_wdomination->count > 0) m2s_config.disable_wdomination = true;
                if (disable_fast_wdomination->count > 0) m2s_config.disable_fast_wdomination = true;
                if (disable_split_intersection_removal->count > 0) m2s_config.disable_split_intersection_removal = true;
                if (disable_weight_transfer->count > 0) m2s_config.disable_weight_transfer = true;
                if (enable_old_weight_transfer->count > 0) m2s_config.disable_old_weight_transfer = false;
                if (disable_direct_neighbor_removal->count > 0) m2s_config.disable_direct_neighbor_removal = true;
                if (disable_two_neighbor_removal->count > 0) m2s_config.disable_two_neighbor_removal = true;
                if (disable_single_fast_domination->count > 0) m2s_config.disable_single_fast_domination = true;
                if (disable_neighborhood_folding->count > 0) m2s_config.disable_neighborhood_folding = true;
                if (disable_fold2->count > 0) m2s_config.disable_fold2 = true;
                if (disable_all->count > 0) m2s_config.disable_all = true;
#endif

                return 0;
        }
};

struct weighted_reducer_cli : public weighted_cli {
        static constexpr int ARGTABLE_SLOTS = 2 + weighted_cli::ARGTABLE_SLOTS;

        // weighted parameters
        struct arg_str *heuristic_decision_style;
        struct arg_str *heuristic_rating;


        void init_cli(void *argtable[]) override {
                weighted_cli::init_cli(argtable);

                heuristic_decision_style =
                    arg_str0(NULL, "rnp_decision_style", NULL,
                             "[RnP redW2pack]: Choose between included and excluded. Default: exclude");
                heuristic_rating =
                    arg_str0(NULL, "rnp_rating", NULL,
                             "[RnP redW2pack]: Choose between weight, deg, weight-diff. Default: weight-diff");

                // set parameters in argtable
                int table_pos = weighted_cli::ARGTABLE_SLOTS;
                argtable[table_pos++] = heuristic_decision_style;
                argtable[table_pos++] = heuristic_rating;
        }

        int parse_cli(int argc, char** argv, red2pack::M2SConfig &m2s_config, std::string &graph_filepath, const std::string &progname,
                      void *argtable[]) override {
                int error = weighted_cli::parse_cli(argc, argv, m2s_config, graph_filepath, progname, argtable);
                if (error > 0) {
                        return error;
                }

                // RnP settings
                if (heuristic_rating->count > 0) {
                        m2s_config.setHeuristicRating(heuristic_rating->sval[0]);
                        if (m2s_config.reduction_style != M2SConfig::Reduction_Style2::heuristic) {
                                std::cerr
                                    << "WARNING: heurstic rating ignored since reduction style is not set to heuristic."
                                    << std::endl;
                        }
                }
                if (heuristic_decision_style->count > 0) {
                        m2s_config.setHeuristicDecisionStyle(heuristic_decision_style->sval[0]);
                        if (m2s_config.reduction_style != M2SConfig::Reduction_Style2::heuristic) {
                                std::cerr
                                    << "WARNING: heurstic rating ignored since reduction style is not set to heuristic."
                                    << std::endl;
                        }
                }

                return 0;
        }
};

struct weighted_solver_cli : public weighted_cli {
        static constexpr int ARGTABLE_SLOTS = 1 + weighted_cli::ARGTABLE_SLOTS;
        struct arg_str *output;

        void init_cli(void *argtable[]) override {
                weighted_cli::init_cli(argtable);
                output = arg_str0(NULL, "output", NULL, "Path to store resulting 2-packing set.");

                int table_pos = weighted_cli::ARGTABLE_SLOTS;
                argtable[table_pos++] = output;
        }

        int parse_cli(int argc, char** argv, M2SConfig& m2s_config, std::string& graph_filepath, const std::string& progname, void* argtable[]) override {
                int error = weighted_cli::parse_cli(argc, argv, m2s_config, graph_filepath, progname, argtable);
                if (error > 0) {
                        return error;
                }

                if (output->count > 0) {
                        m2s_config.output_filename = output->sval[0];
                        m2s_config.write_result = true;
                } else {
                        m2s_config.write_result = false;
                }

                return 0;
        };
};

struct drp_cli : public weighted_solver_cli {
        static constexpr int ARGTABLE_SLOTS = 7 + weighted_solver_cli::ARGTABLE_SLOTS;

        // weighted parameters
        struct arg_dbl *sim_thres;
        struct arg_dbl *max_core_time_limit;
        struct arg_dbl *increase_sim_thres;
        struct arg_dbl *decrease_sim_thres;
        struct arg_int *max_max_num_candidates;
        struct arg_int *max_chils;
        struct arg_str *core_solver;

        void init_cli(void *argtable[]) override {
                weighted_solver_cli::init_cli(argtable);

                // init parameters
                sim_thres = arg_dbl0(NULL, "sim_thres", NULL, "[DRP]: similarity threshold (Default: 0.6)");
                max_core_time_limit =
                    arg_dbl0(NULL, "max_core_time_limit", NULL, "[DRP]: max_core_time_limit (Default: 80)");
                increase_sim_thres =
                    arg_dbl0(NULL, "increase_sim_thres", NULL, "[DRP]: increase similarity threshold (Default: 1.0)");
                decrease_sim_thres =
                    arg_dbl0(NULL, "decrease_sim_thres", NULL, "[DRP]: decrease similarity threshold (Default: 1.0)");
                max_max_num_candidates = arg_int0(NULL, "max_max_num_candidates", NULL,
                                                  "[DRP]: Maximum number of peeling candidates from which an adative "
                                                  "strategy chooses randomly (Default: 50)");
                max_chils = arg_int0(NULL, "max_chils", NULL,
                                     "[DRP]: Choose number of chils solvers for core graph (Default: 1)");
                core_solver =
                    arg_str0(NULL, "core_solver", NULL,
                             "[DRP]: Choose core graph solver between none, exact, and chils (Default: chils)");

                // set parameters in argtable
                int table_pos = weighted_solver_cli::ARGTABLE_SLOTS;
                argtable[table_pos++] = sim_thres;
                argtable[table_pos++] = max_core_time_limit;
                argtable[table_pos++] = increase_sim_thres;
                argtable[table_pos++] = decrease_sim_thres;
                argtable[table_pos++] = max_max_num_candidates;
                argtable[table_pos++] = max_chils;
                argtable[table_pos++] = core_solver;
        }

        int parse_cli(int argc, char** argv, M2SConfig& m2s_config, std::string& graph_filepath, const std::string& progname, void* argtable[]) override {
                int error = weighted_solver_cli::parse_cli(argc, argv, m2s_config, graph_filepath, progname, argtable);
                if (error > 0) {
                        return error;
                }

                if (core_solver->count > 0) {
                        m2s_config.setCoreSolver(core_solver->sval[0]);
                }

                if (max_core_time_limit->count > 0) {
                        m2s_config.max_core_time_limit = max_core_time_limit->dval[0];
                }

                if (max_chils->count > 0) {
                        m2s_config.max_chils = max_chils->ival[0];
                }

                if (sim_thres->count > 0) {
                        m2s_config.similarity_threshold = sim_thres->dval[0];
                }
                if (increase_sim_thres->count > 0) {
                        m2s_config.increase_sim_thres = increase_sim_thres->dval[0];
                }
                if (decrease_sim_thres->count > 0) {
                        m2s_config.decrease_sim_thres = decrease_sim_thres->dval[0];
                }
                if (max_max_num_candidates->count > 0) {
                        m2s_config.max_max_num_candidates = max_max_num_candidates->ival[0];
                }

                return 0;
        }
};


// @brief Helper to parse any CLI with base clase base_cli
template <typename CLIType, typename... Configs>
int parse_parameters(int argc, char **argv, Configs &...configs) {
        const char *progname = argv[0];

        CLIType cli;
        void *argtable[CLIType::ARGTABLE_SLOTS+1] = {nullptr};
        cli.init_cli(argtable);
        cli.set_end(argtable, CLIType::ARGTABLE_SLOTS);
        int nerrors = cli.parse_cli(argc, argv, configs..., progname, argtable);
        arg_freetable(argtable, CLIType::ARGTABLE_SLOTS+1);
        return nerrors;
}

}  // namespace red2pack::cli
