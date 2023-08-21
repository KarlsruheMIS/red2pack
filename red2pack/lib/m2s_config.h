/**
 * m2s_config.h
 * Purpose: Configuration used for the evolutionary maximum independent set algorithms.
 *
 *****************************************************************************/

#ifndef _M2S_CONFIG_H_
#define _M2S_CONFIG_H_

#include <string>

#include "definitions.h"

namespace red2pack {
// Configuration for the calculation of the MIS
struct M2SConfig {
        enum Reduction_Style2 { core, elaborated };
        // Name of the graph file.
        std::string graph_filename;
        // Name of the output file.
        std::string output_filename;
        // Name of the output kernel file.
        std::string kernel_file_name;

        bool console_log;
        bool print_log;
        bool write_2ps;
        int seed;
        double time_limit;

        // turn of single reductions
        bool disable_domination;
        bool disable_fast_domination;
        bool disable_deg_one;
        bool disable_clique;
        bool disable_deg_two;
        bool disable_twin;
        bool on_demand_two_neighborhood;

        Reduction_Style2 reduction_style2;

        void set2ReductionStyle(const std::string& redu_style) {
                 if (strCompare(redu_style, "core")) {
                        reduction_style2 = Reduction_Style2::core;
                } else if (strCompare(redu_style, "elaborated")) {
                        reduction_style2 = Reduction_Style2::elaborated;
                } else {
                        reduction_style2 = Reduction_Style2::elaborated;
                }
        }

       private:
        bool strCompare(const std::string& str1, const std::string& str2) {
                return str1.size() == str2.size() &&
                       std::equal(str1.begin(), str1.end(), str2.begin(), [](unsigned char c1, unsigned char c2) {
                               return std::toupper(c1) == std::toupper(c2);
                       });
        }
};
}  // namespace two_packing_set

#endif
