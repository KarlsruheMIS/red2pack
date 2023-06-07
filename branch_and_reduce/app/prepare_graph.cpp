/** 
 * reduction_evomis.cpp
 * Purpose: Main program for the evolutionary algorithm.
 *
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <argtable3.h>
#include <algorithm>
#include <chrono>
#include <ctime>

#include "../extern/KaHIP/lib/data_structure/graph_access.h"
#include "../KaMIS/wmis/extern/KaHIP/lib/data_structure/graph_access.h"
#include "../extern/KaMIS/wmis/lib/mis/kernel/branch_and_reduce_algorithm.h"
#include "../lib/mis/kernel/reduce_algorithm.h"
#include "../lib/data_structure/dynamic_graph_pack.h"
#include "timer.h"
#include "m2pack_log.h"
#include "graph_io.h"
#include "m2s_config.h"
#include "mis_config.h"
#include "parse_parameters.h"

typedef reduce_algorithm::pack_status pack_status; 


bool is_2PS(M2S_GRAPH::graph_access & G, std::vector<bool> & sol) {
    bool valid = true;
    forall_nodes(G,n) {
        if (sol[n]) {
            forall_out_edges(G, e, n) {
                NodeID target = G.getEdgeTarget(e);
                if (sol[target]) return false;
                forall_out_edges(G, e2, target) {
                    NodeID target2 = G.getEdgeTarget(e2);
                    if (target2 == n) continue;
                    if (sol[target2]) return false;
                } endfor
            } endfor
        }
    } endfor
    return true;
}

int main(int argn, char **argv) {
    m2pack_log::instance()->restart_total_timer();
    m2pack_log::instance()->print_title();

    M2SConfig m2s_config;
    MISConfig mis_config;
	std::string graph_filepath;

    int ret_code = parse_parameters(argn, argv, m2s_config,mis_config, graph_filepath);
    if (ret_code) return 0;


    m2s_config.graph_filename = graph_filepath.substr(graph_filepath.find_last_of( '/')  +1);
    m2pack_log::instance()->set_config(m2s_config);

    M2S_GRAPH::graph_access G;

	graph_io::readGraphWeighted(G, graph_filepath); 
    m2pack_log::instance()->set_graph(G);

    // Print setum information
    m2pack_log::instance()->print_graph();
    m2pack_log::instance()->print_config();
    m2pack_log::instance()->restart_timer();
   

	
	// Start timer
    auto start_point = std::chrono::system_clock::now();
	G.construct_2neighborhood(); 
	// run first reductions
    reduce_algorithm reducer(G, m2s_config);//, false);  
	reducer.run_reductions(); 
    auto stop = std::chrono::system_clock::now();
    std::chrono::duration<double> time = stop - start_point;
    double reduction_time = time.count();

    std::vector<bool> solution(G.number_of_nodes(), 0);
    reducer.get_solution(solution);
    int sol_counter =0;
    for (size_t i =0; i< solution.size(); i++){
        if (solution[i]) sol_counter++;
    }


	// compute new graph access 
	auto& status = reducer.global_status; 	
	int n = status.graph.size(); 
	std::vector<int> new_ids; 
	std::vector<int> old_ids; 
    new_ids.resize(n); 	
	int n_new = 0; // new number of nodes. 
	int m_new = 0; // new number of edges.

	// compute new_ids and old_ids
	for(size_t i = 0; i < status.graph.size(); i++) {
		if(status.node_status[i] == pack_status::not_set) {
			new_ids[i] = n_new;
		    old_ids.push_back(i); 	
			n_new++;	
		} else {
			new_ids[i] = -1; // i is not in the new graph.
		}
	}	

    m2pack_log::instance()->print_reduction(sol_counter, n_new,m_new, reduction_time);

    if (n_new == 0) {
        m2pack_log::instance()->set_best_size(m2s_config, sol_counter);
        m2pack_log::instance()->print_results();
        return 0;
    }


	// compute new graph
    auto start_cond = std::chrono::system_clock::now();
	std::vector<std::vector<int>> adj_two_list; 
	adj_two_list.resize(n_new);
	std::vector<int> new_degrees;	
	new_degrees.resize(n_new); 	
	int m_total = 0; 
	for(size_t i = 0; i < status.graph.size(); i++) {
		if(status.node_status[i] == pack_status::not_set) {
			new_degrees[new_ids[i]] = 0; 
			for(size_t j = 0; j < status.graph[i].size(); j++) { 
				if(status.node_status[status.graph[i][j]] == pack_status::not_set) {
					adj_two_list[new_ids[i]].push_back(new_ids[status.graph[i][j]]);
					m_new++;
					new_degrees[new_ids[i]]++; 	
					m_total++;
				}
			}
			for(size_t j = 0; j < status.graph.get2neighbor_list(i).size(); j++) {
				if(status.node_status[status.graph.get2neighbor_list(i)[j]] == pack_status::not_set) {
					adj_two_list[new_ids[i]].push_back(new_ids[status.graph.get2neighbor_list(i)[j]]); 
					new_degrees[new_ids[i]]++; 
					m_total++;
				}
			}
			std::sort(adj_two_list[new_ids[i]].begin(), adj_two_list[new_ids[i]].end()); 
		}
	}

	// We use build_from_metis
	int vertex = 0; 
	int counter = 0; 
	int* xadj = new int[n_new+1]; 
	int* adjncy = new int[m_total]; 
	for(size_t i = 0; i < n_new; i++) {
		xadj[vertex] = counter; 
		for(size_t j = 0; j < new_degrees[i]; j++) {
			adjncy[counter] = adj_two_list[i][j];	
			counter++; 
		}

		vertex++;
		if(vertex == n_new) {
			xadj[vertex] = counter; 
		}
	}

	graph_access G_cond; 
	G_cond.build_from_metis(n_new, xadj, adjncy); 
    auto stop_cond = std::chrono::system_clock::now();
    std::chrono::duration<double> time_cond = stop_cond - start_cond;
    double condensed_time = time_cond.count();

    m2pack_log::instance()->print_condensed_graph(n_new,m_new, condensed_time);

	
	// print new graph_access to use branch and reduce algorithm afterwards
    stop = std::chrono::system_clock::now();
    std::chrono::duration<double> spendt_time = stop - start_point;
    mis_config.time_limit -= spendt_time.count();
    if (mis_config.time_limit > 0 ) {
        std::cout << "\t\tMIS Solver" << std::endl;
        std::cout << "==========================================" << std::endl;
        std::cout << "Remaining time: " << mis_config.time_limit << std::endl;
        cout_handler::disable_cout();
        branch_and_reduce_algorithm mis_reducer(G_cond, mis_config);
        mis_reducer.run_branch_reduce();
        cout_handler::enable_cout();
        mis_reducer.apply_branch_reduce_solution(G_cond);
    } else { std::cout << "\%timeout" << std::endl;}

    if (true) {
        forall_nodes(G, node) {
            int red_node = new_ids[node];
            if (red_node == -1) continue;
            if (G_cond.getPartitionIndex(red_node)) {
                solution[node] = true;
                sol_counter++;
            }
        } endfor
    }
    m2pack_log::instance()->set_best_size(m2s_config, sol_counter);
    m2pack_log::instance()->print_results();
    if (is_2PS(G, solution)) std::cout << "Valid solution to 2-packing set problem." << std::endl;
    else std::cout << "ERROR: no valid solution!" << std::endl;

    if (m2s_config.kernel_file_name.size()>0) {
	    graph_io::writeGraph(G_cond, m2s_config.kernel_file_name); 
    }


    delete[] xadj;
    delete[] adjncy;
}
