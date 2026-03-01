#ifndef RED2PACK_HEURISTIC_H
#define RED2PACK_HEURISTIC_H

#include <onlinemis/mis_config.h>
#include <red2pack/algorithms/rnt_solver_scheme.h>

namespace red2pack {
class heuristic : public rnt_solver_scheme {
       public:
        heuristic(std::unique_ptr<m2s_graph_access> G, M2SConfig m2s_cfg, onlinemis::MISConfig mis_cfg)
            : rnt_solver_scheme(std::move(G), std::move(m2s_cfg)), mis_cfg(std::move(mis_cfg)) {}
        bool solve_mis(double mis_solve_time_limit, m2s_graph_access& reduced_graph) override;
        void attach(std::unique_ptr<m2s_graph_access> G, M2SConfig m2s_cfg, onlinemis::MISConfig mis_cfg);

       protected:


       protected:
        onlinemis::MISConfig mis_cfg;
};
}


#endif  // RED2PACK_HEURISTIC_H
