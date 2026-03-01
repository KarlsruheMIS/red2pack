//
// Created by Jannick Borowitz on 12.01.24.
//

#ifndef RED2PACK_WEIGHTED_RNT_MMWIS_H
#define RED2PACK_WEIGHTED_RNT_MMWIS_H

#include <mmwis_config.h>
#include <graph_access.h>
#include <red2pack/algorithms/rnt_solver_scheme.h>
#include <memory>
#include <utility>



namespace red2pack {

class weighted_rnt_mmwis : public rnt_solver_scheme {
       public:
        weighted_rnt_mmwis(std::unique_ptr<m2s_graph_access> G, M2SConfig m2s_cfg, ::mmwis::MISConfig mmwis_cfg)
            : rnt_solver_scheme(std::move(G), std::move(m2s_cfg)),
              mmwis_config(std::move(mmwis_cfg)){
	      eps_time = 10; // works better with mmwis
	      };

        void attach(std::unique_ptr<m2s_graph_access> G, M2SConfig m2s_cfg, ::mmwis::MISConfig mmwis_cfg);

       protected:
        void init_reducer() override;
        bool use_reducer() override;
        bool solve_mis(double mis_solve_time_limit, m2s_graph_access& reduced_graph) override;
       private:
        ::mmwis::MISConfig mmwis_config;

        static bool is_IS(graph_access& G) {
                forall_nodes(G, node) {
                        if (G.getPartitionIndex(node) == 1) {
                                forall_out_edges(G, edge, node) {
                                        NodeID neighbor = G.getEdgeTarget(edge);
                                        if (G.getPartitionIndex(neighbor) == 1) {
                                                return false;
                                        }
                                } endfor
                        }
                } endfor

                return true;
        }

};

}  // namespace red2pack

#endif  // RED2PACK_WEIGHTED_RNT_MMWIS_H
