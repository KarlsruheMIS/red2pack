//
// Created by Jannick Borowitz on 23.10.24.
//

#pragma once

#include <red2pack/algorithms/rnt_solver_scheme.h>

#include <memory>
#include <utility>

namespace red2pack {

class weighted_rnt_scip_ilp : public rnt_solver_scheme {
       public:
        weighted_rnt_scip_ilp(std::unique_ptr<m2s_graph_access> G, M2SConfig m2s_cfg)
            : rnt_solver_scheme(std::move(G), std::move(m2s_cfg)) {
                eps_time = 2;
        };

       protected:
        void init_reducer() override;
        bool use_reducer() override;
        bool solve_mis(double mis_solve_time_limit, m2s_graph_access& reduced_graph) override;
};

}  // namespace red2pack
