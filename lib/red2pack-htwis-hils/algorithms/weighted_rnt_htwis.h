//
// Created by Jannick Borowitz on 12.01.24.
//

#ifndef RED2PACK_WEIGHTED_RNT_HTWIS_H
#define RED2PACK_WEIGHTED_RNT_HTWIS_H

#include <red2pack/algorithms/rnt_solver_scheme.h>

namespace red2pack {

class weighted_rnt_htwis : public rnt_solver_scheme {
       public:
        weighted_rnt_htwis(std::unique_ptr<m2s_graph_access> G, M2SConfig m2s_cfg)
            : rnt_solver_scheme(std::move(G), std::move(m2s_cfg)) {}

       protected:
        void init_reducer() override;
        bool use_reducer() override;
        bool solve_mis(double mis_solve_time_limit, m2s_graph_access& reduced_graph) override;
};
}  // namespace red2pack

#endif  // RED2PACK_WEIGHTED_RNT_HTWIS_H
