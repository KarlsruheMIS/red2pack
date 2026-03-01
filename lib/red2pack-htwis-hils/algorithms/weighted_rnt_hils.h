//
// Created by Jannick Borowitz on 12.01.24.
//

#ifndef RED2PACK_WEIGHTED_RNT_HILS_H
#define RED2PACK_WEIGHTED_RNT_HILS_H


#include "red2pack-htwis-hils/hils_config.h"

#include <branch_and_reduce_algorithm.h>
#include <mis_config.h>

#include <red2pack/algorithms/rnt_solver_scheme.h>
#include <red2pack/tools/scoped_timer.h>

#include <memory>
#include <utility>

namespace red2pack {

class weighted_rnt_hils : public rnt_solver_scheme {
       public:
        weighted_rnt_hils(std::unique_ptr<m2s_graph_access> G, M2SConfig m2s_cfg, HilsConfig hils_cfg, MISConfig mis_cfg)
            : rnt_solver_scheme(std::move(G), std::move(m2s_cfg)),
              hils_cfg(std::move(hils_cfg)), mis_cfg(std::move(mis_cfg)){};

       protected:
        HilsConfig hils_cfg;
        MISConfig mis_cfg;

        std::vector<NodeID> reverse_mapping;
        void init_reducer() override;
        bool use_reducer() override;
        bool solve_mis(double mis_solve_time_limit, m2s_graph_access& reduced_graph) override;

	NodeWeight perform_reduction(std::unique_ptr<branch_and_reduce_algorithm>& wmis_reducer, graph_access& G, graph_access& rG, const MISConfig& config) {
                RED2PACK_SCOPED_TIMER("IS reductions");
                wmis_reducer = std::make_unique<branch_and_reduce_algorithm>(G, config);
                wmis_reducer->reduce_graph();

                // Retrieve reduced graph
                reverse_mapping = std::vector<NodeID>(G.number_of_nodes(), 0);
                wmis_reducer->build_graph_access(rG, reverse_mapping);

                NodeWeight is_weight = wmis_reducer->get_is_weight();

                return is_weight;
        }
};

}  // namespace red2pack

#endif  // RED2PACK_WEIGHTED_RNT_HILS_H
