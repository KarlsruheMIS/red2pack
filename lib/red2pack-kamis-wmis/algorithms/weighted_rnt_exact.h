//
// Created by Jannick Borowitz on 19.10.23.
//

#ifndef RED2PACK_WEIGHTED_RNT_EXACT_H
#define RED2PACK_WEIGHTED_RNT_EXACT_H

#include <red2pack/algorithms/rnt_solver_scheme.h>
#include <mis_config.h>

namespace red2pack {

class weighted_rnt_exact : public rnt_solver_scheme {
       public:
        weighted_rnt_exact(std::unique_ptr<m2s_graph_access> G, M2SConfig m2s_cfg, MISConfig mis_cfg)
            : rnt_solver_scheme(std::move(G), std::move(m2s_cfg)), mis_cfg(std::move(mis_cfg)) {}
        void attach(std::unique_ptr<m2s_graph_access> G, M2SConfig m2s_cfg, MISConfig mis_cfg);

       protected:
        void init_reducer() override;
        bool use_reducer() override;
        bool solve_mis(double mis_solve_time_limit, m2s_graph_access& reduced_graph) override;

       private:
        MISConfig mis_cfg;
};

}  // namespace red2pack

#endif  // RED2PACK_WEIGHTED_RNT_EXACT_H
