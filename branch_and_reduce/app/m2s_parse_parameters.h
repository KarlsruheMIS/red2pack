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
int parse_parameters(int argn, char **argv, two_packing_set::M2SConfig &m2s_config, MISConfig &mis_config, std::string &graph_filepath) {
        const char *progname = argv[0];

        // Setup the argtable structs
        struct arg_lit *help = arg_lit0(NULL, "help", "Print help.");
        struct arg_int *user_seed = arg_int0(NULL, "seed", NULL, "Seed to use for the PRNG.");
        struct arg_str *user_conf = arg_str0(
            NULL, "config", NULL,
            "Configuration to use. ([standard|social|full_standard|full_social]). Standard/social use different modes "
            "of the graph partitioning tool. Full configurations use more time consuming parameters.");

        struct arg_int *kahip_mode = arg_int0(NULL, "kahip_mode", NULL, "KaHIP mode to use.");

        struct arg_str *kernelization_mode = arg_str0(
            NULL, "kernelization", NULL,
            "Kernelization to use. ([FastKer|full]). Full is slower but produces smaller kernels (default: FastKer)");
        struct arg_str *reduction_style2 =
            arg_str0(NULL, "reduction_style2", NULL, "Choose between initial, compact and extended.");
        struct arg_str *reduction_style =
            arg_str0(NULL, "reduction_style", NULL, "Choose between normal/sparce (default), dense/osm.");
        struct arg_str *kernel_file_name = arg_str0(NULL, "kernel_file_name", NULL, "filename for kernel");
        struct arg_lit *disable_deg_one = arg_lit0(NULL, "disable_deg_one", "disable reduction deg_one");
        /* struct arg_lit *disable_deg_one_e            = arg_lit0(NULL, "disable_deg_one_e","disable reduction
         * deg_one_e"); */
        struct arg_lit *disable_clique = arg_lit0(NULL, "disable_clique", "disable reduction clique");
        /* struct arg_lit *disable_clique_e             = arg_lit0(NULL, "disable_clique_e", "disable reduction
         * clique_e"); */
        struct arg_lit *disable_cycle = arg_lit0(NULL, "disable_deg_two", "disable reduction deg_two");
        /* struct arg_lit *disable_cycle_e              = arg_lit0(NULL, "disable_cycle_e", "disable reduction
         * cycle_e"); */
        struct arg_lit *disable_neighborhood = arg_lit0(NULL, "disable_neighborhood", "disable reduction neighborhood");
        /* struct arg_lit *disable_neighborhood_e       = arg_lit0(NULL, "disable_neighborhood_e", "disable reduction
         * neighborhood_e"); */
        struct arg_lit *disable_twin = arg_lit0(NULL, "disable_twin", "disable reduction twin");
        /* struct arg_lit *disable_twin_e               = arg_lit0(NULL, "disable_twin_e", "disable reduction twin_e");
         */
        struct arg_lit *disable_domination = arg_lit0(NULL, "disable_domination", "disable reduction domination");
        /* struct arg_lit *disable_domination_e         = arg_lit0(NULL, "disable_domination_e", "disable reduction
         * domination_e"); */
        struct arg_lit *disable_fast_domination =
            arg_lit0(NULL, "disable_fast_domination", "disable reduction fast_domination");
        /* struct arg_lit *disable_fast_domination_e    = arg_lit0(NULL, "disable_fast_domination_e", "disable reduction
         * fast_domination_e"); */

        /* able(argtable, s */

        // struct arg_int *repetitions         = arg_int0(NULL, "repetitions", NULL, "Number of repetitions per
        // round.");

        // struct arg_lit *use_multiway_ns     = arg_lit0(NULL, "use_multiway_ns", "Use the multiway combine operator
        // with node separators."); struct arg_lit *use_multiway_vc     = arg_lit0(NULL, "use_multiway_vc", "Use the
        // multiway combine operator with vertex covers."); struct arg_lit *use_hopcroft        = arg_lit0(NULL,
        // "use_hopcroft", "Use Hopcroft-Karp to fix vertex cover candidates.");

        struct arg_str *filename = arg_strn(NULL, NULL, "FILE", 1, 1, "Path to graph file.");
        struct arg_str *output = arg_str0(NULL, "output", NULL, "Path to store resulting independent set.");
        struct arg_dbl *time_limit = arg_dbl0(NULL, "time_limit", NULL, "Time limit in s. Default 1000s.");
        struct arg_lit *console_log = arg_lit0(NULL, "console_log", "Stream the log into the console");
        struct arg_lit *disable_checks = arg_lit0(NULL, "disable_checks", "Disable sortedness check during I/O.");

        // Reduction
        // struct arg_dbl *best_degree_frac    = arg_dbl0(NULL, "best_degree_frac", NULL, "Fraction of degree nodes to
        // remove before reduction.");
        struct arg_int *red_thres =
            arg_int0(NULL, "red_thres", NULL, "Number of unsuccessful iterations before reduction.");

        struct arg_end *end = arg_end(100);

        // Setup the argtable
        void *argtable[] = {help, filename, output, user_seed, user_conf, kahip_mode, kernelization_mode,
                            disable_deg_one,
                            /* disable_deg_one_e, */
                            disable_clique,
                            /* disable_clique_e, */
                            disable_cycle,
                            /* disable_cycle_e, */
                            disable_neighborhood,
                            /* disable_neighborhood_e, */
                            disable_twin,
                            /* disable_twin_e, */
                            disable_domination,
                            /* disable_domination_e, */
                            disable_fast_domination,
                            /* disable_fast_domination_e, */
                            // use_hopcroft,
                            // use_multiway_ns,
                            // use_multiway_vc,
                            // repetitions,
                            reduction_style2, reduction_style, kernel_file_name, red_thres, time_limit, console_log,
                            // best_degree_frac,
                            disable_checks, end};

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

        if (user_conf->count > 0) {
                if (strcmp(user_conf->sval[0], "standard") == 0)
                        cfg2.standard(m2s_config);
                else if (strcmp(user_conf->sval[0], "social") == 0)
                        cfg2.social(m2s_config);
                else if (strcmp(user_conf->sval[0], "full_standard") == 0)
                        cfg2.full_standard(m2s_config);
                else if (strcmp(user_conf->sval[0], "full_social") == 0)
                        cfg2.full_social(m2s_config);
        }

        m2s_config.fullKernelization = false;
        if (kernelization_mode->count > 0) {
                if (strcmp(kernelization_mode->sval[0], "FastKer") == 0)
                        m2s_config.fullKernelization = false;
                else if (strcmp(kernelization_mode->sval[0], "full") == 0)
                        m2s_config.fullKernelization = true;
        }

        if (filename->count > 0) {
                graph_filepath = filename->sval[0];
                m2s_config.graph_filename = graph_filepath.substr(graph_filepath.find_last_of('/') + 1);
        }

        if (kahip_mode->count > 0) {
                m2s_config.kahip_mode = kahip_mode->ival[0];
        }

        if (user_seed->count > 0) {
                m2s_config.seed = user_seed->ival[0];
                mis_config.seed = user_seed->ival[0];
        }

        // if (use_multiway_ns->count > 0) {
        //     m2s_config.use_multiway_vc = false;
        // }

        // if (use_multiway_vc->count > 0) {
        //     m2s_config.use_multiway_vc = true;
        // }

        // if (use_hopcroft->count > 0) {
        //     m2s_config.use_hopcroft = true;
        // }

        // if (repetitions->count > 0) {
        //     m2s_config.repetitions = repetitions->ival[0];
        // }

        if (reduction_style2->count > 0) {
                m2s_config.set2ReductionStyle(reduction_style2->sval[0]);
        }

        if (reduction_style->count > 0) {
                mis_config.setReductionStyle(reduction_style->sval[0]);
        }

        if (kernel_file_name->count > 0) {
                m2s_config.kernel_file_name = kernel_file_name->sval[0];
        }

        if (red_thres->count > 0) {
                m2s_config.reduction_threshold = red_thres->ival[0];
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

        if (disable_checks->count > 0) {
                m2s_config.check_sorted = false;
        }

        if (disable_deg_one->count > 0) m2s_config.disable_deg_one = true;
        /* if (disable_deg_one_e->count > 0)  m2s_config.disable_deg_one_e= true; */
        if (disable_clique->count > 0) m2s_config.disable_clique = true;
        /* if (disable_clique_e->count > 0)  m2s_config.disable_clique_e = true; */
        if (disable_cycle->count > 0) m2s_config.disable_deg_two = true;
        /* if (disable_cycle_e->count > 0)  m2s_config.disable_cycle_e = true; */
        if (disable_twin->count > 0) m2s_config.disable_twin = true;
        /* if (disable_twin_e->count > 0)  m2s_config.disable_twin_e = true; */
        if (disable_domination->count > 0) m2s_config.disable_domination = true;
        /* if (disable_domination_e->count > 0)  m2s_config.disable_domination_e = true; */
        if (disable_fast_domination->count > 0) m2s_config.disable_fast_domination = true;
        /* if (disable_fast_domination_e->count > 0)  m2s_config.disable_fast_domination_e  = true; */
        if (disable_neighborhood->count > 0) m2s_config.disable_neighborhood = true;
        /* if (disable_neighborhood_e->count > 0)  m2s_config.disable_neighborhood_e = true; */

        if (output->count > 0) {
                m2s_config.output_filename = output->sval[0];
                m2s_config.write_graph = true;
        } else {
                m2s_config.write_graph = false;
        }
        arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));

        return 0;
}

#endif
