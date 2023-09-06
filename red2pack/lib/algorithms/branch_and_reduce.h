#ifndef INC_2_PACKING_SET_RED2PACK_BNR_H
#define INC_2_PACKING_SET_RED2PACK_BNR_H

#include <utility>

#include "mis_config.h"
#include "solver_scheme.h"

namespace red2pack {

class branch_and_reduce : public solver_scheme {
       public:
        branch_and_reduce(std::unique_ptr<m2s_graph_access> G, M2SConfig m2s_cfg, MISConfig mis_cfg)
            : solver_scheme(std::move(G), std::move(m2s_cfg)), mis_cfg(std::move(mis_cfg)) {}
        bool solve_mis(graph_access& condensed_graph) override;
        void attach(std::unique_ptr<m2s_graph_access> G, M2SConfig m2s_cfg, MISConfig mis_cfg);

       private:
        MISConfig mis_cfg;
};

}  // namespace red2pack
#endif  // INC_2_PACKING_SET_RED2PACK_BNR_H
