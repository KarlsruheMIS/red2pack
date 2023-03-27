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

#include "../extern/KaHIP/interface/kaHIP_interface.h"
#include "../extern/KaHIP/lib/data_structure/graph_access.h"
#include "../extern/KaHIP/lib/io/graph_io.h"
#include "../extern/KaHIP/lib/definitions.h"
#include "../lib/mis/kernel/reduce_algorithm.h"
#include "../lib/data_structure/dynamic_graph_pack.h"
#include "../lib/mis/kernel/reductions_pack.h"
#include "timer.h"
#include "ils/ils.h"
#include "ils/local_search.h"
#include "mis_log.h"
#include "graph_io.h"
#include "reduction_evolution.h"
#include "mis_config.h"
#include "greedy_mis.h"
#include "parse_parameters.h"
#include "data_structure/graph_access.h"
#include "data_structure/mis_permutation.h"
#include "mis/kernel/ParFastKer/fast_reductions/src/full_reductions.h"

typedef reduce_algorithm::pack_status pack_status; 

template<class reducer>
int run(MISConfig &mis_config, graph_access &G, std::vector<int>& old_ids, graph_access& G_2, std::vector<NodeID>& sol_vector) {

    // Perform the evolutionary algorithm
    std::vector<bool> independent_set(G.number_of_nodes(), false);
    reduction_evolution<reducer> evo;
    std::vector<NodeID> best_nodes;
    evo.perform_mis_search(mis_config, G, independent_set, best_nodes);

    mis_log::instance()->print_results();
    if (mis_config.print_log) mis_log::instance()->write_log();
    if (mis_config.write_graph) graph_io::writeIndependentSet(G, mis_config.output_filename);

    std::cout <<  "checking solution ..."  << std::endl;
    int counter = 0;
    forall_nodes(G, node) {
            if( independent_set[node] ) {
		    //std::cout << node << std::endl; 
                    counter++;
                    forall_out_edges(G, e, node) {
                            NodeID target = G.getEdgeTarget(e);
                            if(independent_set[target]) {
                                std::cout <<  "not an independent set!"  << std::endl;
                                exit(1);
                            }
                    } endfor
            }
    } endfor

    // add nodes to already existing solution
    for(int j = 0; j < G.number_of_nodes(); j++) {
    	if(independent_set[j] == 1) {
		sol_vector[old_ids[j]] = 1; 
	}
    }

    std::cout <<  "done ..."  << std::endl;
    std::cout << "Independent set has size " << counter << std::endl;

    // check solution 
	bool is_set_2 = true; 
	int counter_3 = 0; 
	forall_nodes(G_2,n) {
		if(sol_vector[n] == 1) {
			counter_3++; 
			forall_out_edges(G_2,e,n) {
				int k = G_2.getEdgeTarget(e); 
				if(sol_vector[k] == 1) {
					is_set_2 = false; 
				}
			}endfor
			for(size_t i = 0; i < G_2.two_neighbors[n].size(); i++) {
				if(sol_vector[G_2.two_neighbors[n][i]] == 1) {
					is_set_2 = false; 
				}
			}
		}
	}endfor

	if(is_set_2) {
		std::cout << "Is really two-packing set. The size is: " << counter_3 << std::endl; 
	} else {
		std::cout << "error" << std::endl; 
	}
    return 0;
}
		

int main(int argn, char **argv) {
    mis_log::instance()->restart_total_timer();
    mis_log::instance()->print_title();
    
    MISConfig mis_config;
    std::string graph_filepath;

    // Parse the command line parameters;
    int ret_code = parse_parameters(argn, argv, mis_config, graph_filepath);
    if (ret_code) {
        return 0;
    }
    mis_config.graph_filename = graph_filepath.substr(graph_filepath.find_last_of( '/' ) +1);
    mis_log::instance()->set_config(mis_config);
	
	graph_access G;
	std::string filename(argv[1]); 

	graph_io::readGraphWeighted(G,filename); 
	
	std::cout << "Graph: " << filename << ". Nodes: " << G.number_of_nodes() << ". Edges: " << G.number_of_edges() << std::endl;
	auto start_point = std::chrono::high_resolution_clock::now();
	G.construct_2neighborhood(); 
	reduce_algorithm reducer(G);  
	// run first reductions
	reducer.run_reductions();
       	// solution so far	
	auto sol_vector = reducer.get_status();
	// compute reduced densed graph
		       	
	auto& status = reducer.global_status; 	
	int n = status.graph.size(); 
	std::vector<int> new_ids; 
	std::vector<int> old_ids; 
       	new_ids.resize(n); 	
	int n_new = 0; // new number of nodes. 
	int m_new = 0; // new number of edges.

	for(size_t i = 0; i < status.graph.size(); i++) {
		if(status.node_status[i] != pack_status::excluded && status.node_status[i] != pack_status::included && status.node_status[i] != pack_status::unsafe) {
			new_ids[i] = n_new;
		       	old_ids.push_back(i); 	
			n_new++;	
		} else {
			new_ids[i] = -1; // i is not in the new graph.
		}
	}	

	std::vector<std::vector<int>> adj_two_list; 
	adj_two_list.resize(n_new);
     	std::vector<int> new_degrees;	
       	new_degrees.resize(n_new); 	
	int m_total = 0; 
	for(size_t i = 0; i < status.graph.size(); i++) {
		if(status.node_status[i] != pack_status::excluded && status.node_status[i] != pack_status::included && status.node_status[i] != pack_status::unsafe) {
			new_degrees[new_ids[i]] = 0; 
			for(size_t j = 0; j < status.graph[i].size(); j++) { 
				if(status.node_status[status.graph[i][j]] != pack_status::excluded && status.node_status[i] != pack_status::included && status.node_status[status.graph[i][j]] != pack_status::unsafe) {
					adj_two_list[new_ids[i]].push_back(new_ids[status.graph[i][j]]); 
					m_new++;
					new_degrees[new_ids[i]]++; 	
					m_total++;
				}
			}
	
			for(size_t j = 0; j < status.graph.get2neighbor_list(i).size(); j++) {
				if(status.node_status[status.graph.get2neighbor_list(i)[j]] != pack_status::excluded && status.node_status[status.graph.get2neighbor_list(i)[j]] != pack_status::included && status.node_status[status.graph.get2neighbor_list(i)[j]] != pack_status::unsafe) {
					adj_two_list[new_ids[i]].push_back(new_ids[status.graph.get2neighbor_list(i)[j]]); 
					new_degrees[new_ids[i]]++; 
					m_total++;
				}
			}
			std::sort(adj_two_list[new_ids[i]].begin(), adj_two_list[new_ids[i]].end()); 
		}
	}

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

        auto stop_point = std::chrono::high_resolution_clock::now();
	auto time_now = std::chrono::duration_cast<std::chrono::milliseconds>(stop_point-start_point);
	 
	std::cout << "Time for the first part: " << time_now.count() << " milliseconds." << std::endl;
	std::cout << "\n";

	
    //graph_access G;
    //graph_io::readGraphWeighted(G, graph_filepath);
    mis_log::instance()->set_graph(G_cond);
    
    // Print setup information
    mis_log::instance()->print_graph();
    mis_log::instance()->print_config();
	 

    if(mis_config.fullKernelization) {
    	return run<branch_and_reduce_algorithm>(mis_config, G_cond, old_ids, G, sol_vector);
    } 
    else {
	// Might be a bit confusingly named, but this is FastKer
    	return run<full_reductions>(mis_config, G_cond, old_ids, G, sol_vector);
    } 

}
