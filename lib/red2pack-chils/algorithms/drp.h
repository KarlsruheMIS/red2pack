//
// Created by Jannick Borowitz on 15.10.24.
//

#pragma once

#include <red2pack/algorithms/rnt_solver_scheme.h>
#include <red2pack/data_structure/sized_vector.h>

namespace red2pack {
// inherits from rnt_solver_scheme as it handles the initial reduce phase
class drp : public rnt_solver_scheme {

       public:
        drp(std::unique_ptr<m2s_graph_access> G, M2SConfig m2s_cfg)
            : rnt_solver_scheme(std::move(G), std::move(m2s_cfg)), free_nodes(graph->number_of_nodes()){};

       protected:
        sized_vector<NodeID> free_nodes;
        void init_reducer() override;
        bool use_reducer() override;
        bool solve_mis(double mis_solve_time_limit, m2s_graph_access& reduced_graph) override;

        NodeWeight maximize_solution(m2s_graph_access& reduced_graph, std::vector<bool>& reduced_solution);
};

}

