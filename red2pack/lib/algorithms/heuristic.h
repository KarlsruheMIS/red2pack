#ifndef RED2PACK_HEURISTIC_H
#define RED2PACK_HEURISTIC_H
#include "../../extern/onlinemis/mis_config.h"
#include "solver_scheme.h"
#include "graph_access.h"

namespace red2pack {
class heuristic : public solver_scheme {
       public:
        heuristic(m2s_graph_access& G, M2SConfig m2s_cfg, onlinemis::MISConfig mis_cfg)
            : solver_scheme(G, std::move(m2s_cfg)), mis_cfg(std::move(mis_cfg)) {}
        bool solve_mis(graph_access& condensed_graph) override;

       protected:
        void build_condensed_graph_from_access(graph_access& condensed_graph, m2s_graph_access& graph, NodeID nodes,
                                               EdgeID edges) override;
        void build_condensed_graph_from_status(graph_access& condensed_graph, m2s_dynamic_graph& reduced_graph,
                                               std::vector<reduce_algorithm::two_pack_status>& reduced_node_status,
                                               NodeID nodes, EdgeID edges) override;

       protected:
        onlinemis::MISConfig mis_cfg;
};
}


#endif  // RED2PACK_HEURISTIC_H
