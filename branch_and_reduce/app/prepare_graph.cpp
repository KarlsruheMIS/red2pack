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

int main(int argn, char **argv) {

	graph_access G;
	std::string filename(argv[1]); 

	graph_io::readGraphWeighted(G,filename); 
	
	std::cout << "%" << "Graph: " << filename << ". Nodes: " << G.number_of_nodes() << ". Edges: " << G.number_of_edges() << std::endl; 
	// Start timer
	auto start_point = std::chrono::high_resolution_clock::now();
	G.construct_2neighborhood(); 
	// run first reductions
	reduce_algorithm reducer(G);  
	reducer.run_reductions(); 
	auto sol_vector = reducer.get_status();


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
		if(status.node_status[i] != pack_status::excluded && status.node_status[i] != pack_status::included && status.node_status[i] != pack_status::unsafe) {
			new_ids[i] = n_new;
		       	old_ids.push_back(i); 	
			n_new++;	
		} else {
			new_ids[i] = -1; // i is not in the new graph.
		}
	}	

	// compute new graph
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

        auto stop_point = std::chrono::high_resolution_clock::now();
	auto time_now = std::chrono::duration_cast<std::chrono::milliseconds>(stop_point-start_point);
	
	int new_counter = 0; 
	for(int i = 0; i < sol_vector.size(); i++) {
		if(sol_vector[i] == 1) {
			new_counter++; 
		}
	}
	 
	std::cout << "% " << "Time for the first part: " << time_now.count() << " milliseconds. " << "Weight: " << new_counter << std::endl;
	
	// print new graph_access to use branch and reduce algorithm afterwards
	std::cout << n_new << " " << m_total/2 << std::endl;
	forall_nodes(G_cond,n) {
		forall_out_edges(G_cond,e,n) {
			NodeID k = G_cond.getEdgeTarget(e); 
			std::cout << k+1 << " ";
		}endfor
		std::cout << "\n"; 
	}endfor
}












