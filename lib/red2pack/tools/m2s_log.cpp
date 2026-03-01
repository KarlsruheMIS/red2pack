/******************************************************************************
 * m2s_log.cpp
 *
 * based on mis_log.h from KaMIS
 * Copyright (C) 2015-2017 Sebastian Lamm <lamm@ira.uka.de>
 *
 *****************************************************************************/

#include "m2s_log.h"

#include <fstream>
#include <iomanip>
#include <iostream>

namespace red2pack {

m2s_log::m2s_log() {
        number_of_nodes = 0;
        number_of_edges = 0;
        best_solution_size = 0;
        time_taken_best = 0.0;

}

m2s_log::~m2s_log() {}

void m2s_log::set_config(M2SConfig &config) {
        log_config = config;
        if (config.use_weighted_reductions()) {
                reduced_nodes_mw2ps = std::vector<NodeID>( MW2PS_REDUCTION_NUM, 0);
                reduced_nodes_mw2ps_t = std::vector(MW2PS_REDUCTION_NUM, 0.0);
        }
}

void m2s_log::set_graph(m2s_graph_access &G) {
        number_of_nodes = G.number_of_nodes();
        number_of_edges = G.number_of_edges();
}

void m2s_log::print_graph() {
        std::cout << "\t\tGraph" << std::endl;
        std::cout << "==========================================" << std::endl;
        std::cout << "IO time:\t\t\t" << total_timer.elapsed() << std::endl;
        std::cout << "Filename:\t\t\t" << log_config.graph_filename << std::endl;
        std::cout << "|-Nodes:\t\t\t" << number_of_nodes << std::endl;
        std::cout << "|-Edges:\t\t\t" << number_of_edges / 2 << std::endl;
        std::cout << std::endl;
}

void m2s_log::print_config() {
        std::cout << "\t\tConfiguration" << std::endl;
        std::cout << "==========================================" << std::endl;
        std::cout << "Time limit:\t\t\t" << log_config.time_limit << std::endl;
        std::cout << "Reduction Style:\t\t" << log_config.reduction_style << std::endl;
        std::cout << std::endl;
}
void m2s_log::print_transformed_graph(unsigned int n, unsigned int m, double time) {
        std::cout << "\t\tTransformation (Building graph for M(W)IS solver)" << std::endl;
        std::cout << "==========================================" << std::endl;
        std::cout << "|-Transformed G Nodes:\t\t" << n << std::endl;
        std::cout << "|-Transformed G Edges:\t\t" << m << std::endl;
        std::cout << "|-Transformation time:\t\t" << time << "\n" << std::endl;
}

void m2s_log::print_reduction(unsigned int extracted_nodes, unsigned int kernel_size, unsigned int kernel_size_m,
                              unsigned int kernel_size_m2) {
        std::cout << "\t\tReduction (Includes building link-neighborhoods)" << std::endl;
        std::cout << "==========================================" << std::endl;
        std::cout << "|-Offset:\t\t\t\t" << extracted_nodes << std::endl;
        std::cout << "|-Kernel Nodes:\t\t\t" << kernel_size << std::endl;
        std::cout << "|-Kernel Edges:\t\t\t" << kernel_size_m << std::endl;
        std::cout << "|-Kernel Links:\t\t" << kernel_size_m2 << std::endl;
}

void m2s_log::print_results() {
        std::cout << std::endl;
        std::cout << "\t\tResult" << std::endl;
        std::cout << "==========================================" << std::endl;
        std::cout << "Size:\t\t\t\t" << best_solution_size << std::endl;
        std::cout << "Time best:\t\t" << time_taken_best << std::endl;
        std::cout << "Time solve:\t\t" << time_solve << std::endl;
        std::cout << "Time all:\t\t" << total_time << std::endl;
        std::cout << std::endl;
}




void m2s_log::restart_total_timer() { total_timer.restart(); }

void m2s_log::restart_timer() { t.restart(); }

double m2s_log::get_timer() { return t.elapsed(); }

void m2s_log::set_best_size(unsigned int size) {
        if (size > best_solution_size) {
                best_solution_size = size;
                time_taken_best = t.elapsed();
                std::cout << "red2pack best-sol: " << best_solution_size << " [" <<  time_taken_best << "]" << std::endl;
        }
}
void m2s_log::set_best_size(unsigned int size, double best_time) {
        if (size > best_solution_size) {
                best_solution_size = size;
                time_taken_best = best_time;
                std::cout << "red2pack best-sol: " << best_solution_size << " [" <<  time_taken_best << "]" << std::endl;
        }
}
NodeWeight m2s_log::get_best_size() const {
        return best_solution_size;
}

void m2s_log::add_reduced_nodes_mw2ps(mw2ps_reduction_type reduction_type, NodeID reduced_nodes, double duration) {
        reduced_nodes_mw2ps[static_cast<size_t>(reduction_type)] += reduced_nodes;
        reduced_nodes_mw2ps_t[static_cast<size_t>(reduction_type)] += duration;
}
void m2s_log::print_reduced_nodes_mw2ps() {
        using redt = mw2ps_reduction_type;
        std::vector<redt> reduction_types;
        reduction_types.push_back(redt::fast_neighborhood_removal);
        reduction_types.push_back(redt::fast_complete_degree_one_removal);
        reduction_types.push_back(redt::fast_degree_two_removal);
        reduction_types.push_back(redt::neighborhood_removal);
        reduction_types.push_back(redt::fast_domination);
        reduction_types.push_back(redt::domination);
        reduction_types.push_back(redt::split_neighbor_removal);
        reduction_types.push_back(redt::single_fast_domination);
        reduction_types.push_back(redt::direct_neighbor_removal);
        reduction_types.push_back(redt::two_neighbor_removal);
        reduction_types.push_back(redt::split_intersection_removal);
        reduction_types.push_back(redt::weight_transfer);
        reduction_types.push_back(redt::old_weight_transfer);
        reduction_types.push_back(redt::neighborhood_folding);
        reduction_types.push_back(redt::fold2);
        reduction_types.push_back(redt::fast_direct_neighbor_removal);
        reduction_types.push_back(redt::heuristic);


        std::ostringstream ss;
        std::cout << "\t\tReduced Nodes By Reduction" << std::endl;
        std::cout << "==========================================" << std::endl;
        for(auto &red : reduction_types) {
                auto &reduced_nodes = reduced_nodes_mw2ps[static_cast<size_t>(red)];
                auto &duration = reduced_nodes_mw2ps_t[static_cast<size_t>(red)];

                ss << "|-" << mw2ps_reduction_to_string(red) << ":";
                std::cout << std::left << std::setw(45) << ss.str();
                ss.str("");
                ss << reduced_nodes;
                std::cout << std::left << std::setw(14) << ss.str();
                ss.str("");
                ss << duration;
                std::cout << std::left << std::setw(14) << ss.str() << std::endl;
                ss.str("");
        }
}
const char *m2s_log::mw2ps_reduction_to_string(mw2ps_reduction_type reduction_type) noexcept {
        using redt=mw2ps_reduction_type;
        switch (reduction_type) {
                case redt::fast_neighborhood_removal:
                        return "Fast Neighborhood Removal";
                case redt::fast_complete_degree_one_removal:
                        return "Fast Complete Degree-1 Removal";
		            case redt::fast_degree_two_removal:
                        return "Fast Degree-2 Removal";
                case redt::neighborhood_removal:
                        return "Neighborhood Removal";
                case redt::fast_domination:
                        return "Fast-Domination";
                case redt::domination:
                        return "Domination";
                case redt::single_fast_domination:
                        return "Single Fast Domination";
                case redt::fast_direct_neighbor_removal:
                        return "Fast Direct Neighbor Removal";
                case redt::direct_neighbor_removal:
                        return "Direct Neighbor Removal";
                case redt::two_neighbor_removal:
                        return "Two Neighbor Removal";
                case redt::split_neighbor_removal:
                        return "Split Neighbor Removal";
                case redt::split_intersection_removal:
                        return "Split Intersection Removal";
                case redt::weight_transfer:
                        return "Weight Transfer";
                case redt::old_weight_transfer:
                        return "Old Weight Transfer";
                case redt::neighborhood_folding:
                        return "Neighborhood Folding";
                case redt::fold2:
                        return "Fold 2";
                case redt::heuristic:
                        return "Heuristic";
                default:
                        throw std::invalid_argument("Unimplemented item");
        }
}
void m2s_log::finish_solving() {
        time_solve = t.elapsed();
        total_time = total_timer.elapsed();
}


}  // namespace red2pack
