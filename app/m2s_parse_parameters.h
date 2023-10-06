/**
 * parse_parameters.h
 * Purpose: Parse command line parameters.
 *
 *****************************************************************************/

#ifndef _PARSE_PARAMETERS2_H_
#define _PARSE_PARAMETERS2_H_

#include <omp.h>
#include <stdio.h>
#include <string.h>

#include "argtable3.h"
#include "m2s_configuration_m2s.h"
#include "m2s_configuration_mis.h"

/**
 * Parse the given parameters and apply them to the config.
 *
 * @param argn Number of parameters.
 * @param argv Values of the parameters.
 * @param m2s_config Config to store the values in.
 * @param graph_path String to store the filepath of the graph.
 *
 * @return -1 if there was an error. 0 otherwise.
 */
int parse_parameters(int argn, char **argv, red2pack::M2SConfig &m2s_config, MISConfig &mis_config, std::string &graph_filepath) {
        const char *progname = argv[0];

        // Setup the argtable structs
        struct arg_lit *help = arg_lit0(NULL, "help", "Print help.");
        struct arg_int *user_seed = arg_int0(NULL, "seed", NULL, "Seed to use for the PRNG.");
        struct arg_str *reduction_style =
            arg_str0(NULL, "reduction_style", NULL, "Choose between elaborated and core.");
        struct arg_str *kernel_file_name = arg_str0(NULL, "kernel_file_name", NULL, "filename for kernel");
        /* struct arg_lit *disable_deg_one = arg_lit0(NULL, "disable_deg_one", "disable reduction deg_one"); */
        /* struct arg_lit *disable_clique = arg_lit0(NULL, "disable_clique", "disable reduction clique"); */
        /* struct arg_lit *disable_cycle = arg_lit0(NULL, "disable_deg_two", "disable reduction deg_two"); */
        /* struct arg_lit *disable_twin = arg_lit0(NULL, "disable_twin", "disable reduction twin"); */
        /* struct arg_lit *disable_domination = arg_lit0(NULL, "disable_domination", "disable reduction domination"); */
        /* struct arg_lit *disable_fast_domination = arg_lit0(NULL, "disable_fast_domination", "disable reduction fast_domination"); */
        struct arg_lit *full_two_neighborhood = arg_lit0(NULL, "full_two_neighborhood", "disable on demand two neighborhood");

        struct arg_str *filename = arg_strn(NULL, NULL, "FILE", 1, 1, "Path to graph file.");
        struct arg_lit *console_log = arg_lit0(NULL, "console_log", "Stream the log into the console");
        struct arg_lit *silent = arg_lit0(NULL, "silent", "Run the program without intermediate output.");
        struct arg_str *output = arg_str0(NULL, "output", NULL, "Path to store resulting 2-packing-set.");
        struct arg_dbl *time_limit = arg_dbl0(NULL, "time_limit", NULL, "Time limit in s. Default 1000s.");

        struct arg_end *end = arg_end(100);

        // Setup the argtable
        void *argtable[] = {help, filename, output, user_seed,
                            /* disable_deg_one, */
                            /* disable_clique, */
                            /* disable_cycle, */
                            /* disable_twin, */
                            /* disable_domination, */
                            /* disable_fast_domination, */
                            full_two_neighborhood,
                            reduction_style, 
                            kernel_file_name, 
                            console_log, 
                            silent, 
                            time_limit,
                            end};

        // Choose standard configuration
        m2s_configuration_m2s cfg2;
        m2s_configuration_mis cfg;
        cfg2.standard(m2s_config);
        cfg.standard(mis_config);

        // Parse the arguments
        int nerrors = arg_parse(argn, argv, argtable);

        if (help->count > 0) {
                printf("Usage: %s", progname);
                arg_print_syntax(stdout, argtable, "\n");
                arg_print_glossary(stdout, argtable, "  %-40s %s\n");
                arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
                return 1;
        }

        if (nerrors > 0) {
                arg_print_errors(stderr, end, progname);
                printf("Try '%s --help' for more information.\n", progname);
                arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
                return 1;
        }


        if (filename->count > 0) {
                graph_filepath = filename->sval[0];
                m2s_config.graph_filename = graph_filepath.substr(graph_filepath.find_last_of('/') + 1);
        }

        if (user_seed->count > 0) {
                m2s_config.seed = user_seed->ival[0];
                mis_config.seed = user_seed->ival[0];
        }

        if (reduction_style->count > 0) {
                m2s_config.set2ReductionStyle(reduction_style->sval[0]);
        }

        if (kernel_file_name->count > 0) {
                m2s_config.kernel_file_name = kernel_file_name->sval[0];
        }

        if (time_limit->count > 0) {
                m2s_config.time_limit = time_limit->dval[0];
                mis_config.time_limit = time_limit->dval[0];
        }

        // if (best_degree_frac->count > 0) {
        // m2s_config.remove_fraction = best_degree_frac->dval[0];
        //}

        if (console_log->count > 0) {
                m2s_config.console_log = true;
                m2s_config.print_log = false;
        } else {
                m2s_config.print_log = true;
        }

        if (silent-> count > 0) {
            m2s_config.silent = true;
        }


        /* if (disable_deg_one->count > 0) m2s_config.disable_deg_one = true; */
        /* if (disable_clique->count > 0) m2s_config.disable_clique = true; */
        /* if (disable_cycle->count > 0) m2s_config.disable_deg_two = true; */
        /* if (disable_twin->count > 0) m2s_config.disable_twin = true; */
        /* if (disable_domination->count > 0) m2s_config.disable_domination = true; */
        /* if (disable_fast_domination->count > 0) m2s_config.disable_fast_domination = true; */
        if (full_two_neighborhood->count > 0) m2s_config.on_demand_two_neighborhood = false;

        if (output->count > 0) {
                m2s_config.output_filename = output->sval[0];
                m2s_config.write_2ps = true;
        } else {
                m2s_config.write_2ps = false;
        }
        arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));

        return 0;
}

#endif
