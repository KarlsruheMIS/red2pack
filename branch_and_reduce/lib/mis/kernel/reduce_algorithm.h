/***********************************************************************************
 * reduce_algorithm.h
 *
 **********************************************************************************/

#ifndef REDUCE_SOLVER_PACK_H
#define REDUCE_SOLVER_PACK_H

// local includes
#include "fast_set.h"
#include "../../../extern/KaHIP/lib/definitions.h"
#include "../../../extern/KaHIP/lib/data_structure/graph_access.h"
#include "../../data_structure/sized_vector.h"
#include "../../data_structure/dynamic_graph_pack.h"
#include "reductions_pack.h"
#include "m2s_config.h"
#include "timer.h"


// system includes
#include <vector>
#include <memory>
#include <array>
#include <string>
#include <functional>
#include <iostream>
#include <sstream>

class evo_graph; 

class reduce_algorithm {

public:
	enum pack_status {not_set, included, excluded, folded, unsafe}; 

private: 
	friend general_reduction_2pack; 
	friend deg_one_2reduction;
	friend cycle2_reduction; 
	friend twin2_reduction; 
	friend fast_domination2_reduction; 
	friend neighborhood2_reduction; 
	friend domination2_reduction; 
	friend clique2_reduction;
	friend deg_one_2reduction_e;
	friend cycle2_reduction_e; 
	friend twin2_reduction_e; 
	friend fast_domination2_reduction_e; 
	friend neighborhood2_reduction_e; 
	friend domination2_reduction_e; 
	friend clique2_reduction_e;

	friend evo_graph; 	 
	

	struct node_pos {
		NodeID node; 
		size_t pos; 
		
		// Konstruktor...
		node_pos(NodeID node = 0, size_t pos = 0) : node(node), pos(pos) {}
	}; 


	struct graph_status {
		friend evo_graph; 

		size_t n = 0; 
		size_t remaining_nodes = 0; 
		NodeWeight pack_weight = 0; 
		NodeWeight reduction_offset = 0; 
		dynamic_graph_pack graph; 
		std::vector<NodeWeight> weights;
		std::vector<pack_status> node_status; 
		std::vector<reduction2_ptr> reductions2;
	    std::vector<m2ps_reduction_type> folded_queue; 	
		std::vector<NodeID> modified_queue;
		

		graph_status(M2S_GRAPH::graph_access& G) : 
			n(G.number_of_nodes()), remaining_nodes(n), graph(G), weights(n,0), node_status(n, pack_status::not_set), 
			folded_queue(n) {
				forall_nodes(G, node) {
					weights[node] = G.getNodeWeight(node); 
				} endfor
			}

        graph_status() = default; 
	};
	
	/////////////////////////////////////////////////////DATA/////////////////////////////////////////////////////////////////
	size_t active_reduction_index;  

	//graph_status global_status; 
	std::vector<size_t> reduction_map; 


	graph_status status; 
    M2SConfig config;
    timer t;
	// Lets see whether we need all of them or not...
	fast_set set_1; 
	fast_set set_2; 
	fast_set double_set; 
	sized_vector<sized_vector<NodeID>> buffers;  
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	void set_imprecise(NodeID node, pack_status mpack_status); 
	void set(NodeID node, pack_status mpack_status); 
	size_t deg(NodeID node) const; 
	size_t two_deg(NodeID node);


    void add_next_level_node(NodeID node); 	
	void add_next_level_neighborhood(NodeID node); 
	void add_next_level_neighborhood(const std::vector<NodeID>& nodes); 
	void init_reduction_step(); 
	void reduce_graph_internal(); 
	void initial_reduce(); 

public: 
	reduce_algorithm(M2S_GRAPH::graph_access& G, const M2SConfig & mis_config);//, bool called_from_fold); 
    void get_solution (std::vector<bool> & solution_vec);
	void run_reductions(); 
/* 	std::vector<NodeID> get_status();  */
	graph_status global_status;
};
#endif // REDUCE_SOLVER_H
