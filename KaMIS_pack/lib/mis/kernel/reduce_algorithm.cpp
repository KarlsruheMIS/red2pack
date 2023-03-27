/***********************************************************************************
 * reduce_algorithm.cpp
 *
 **********************************************************************************/


#include "reduce_algorithm.h"

#include<algorithm>
#include<chrono>
#include<numeric>
#include<queue>

struct deg_node {
	public: 
		size_t v; 
		size_t deg; 

		bool operator<(const deg_node& rhs) const {
			return deg < rhs.deg; 
		}
};

reduce_algorithm::reduce_algorithm(graph_access& G, bool called_from_fold) 
	: global_status(G), set_1(global_status.n), set_2(global_status.n), double_set(global_status.n*2), buffers(2, sized_vector<NodeID>(global_status.n)) {
		// I have no configurations, yet...
		global_status.reductions = make_reduction_vector<clique_reduction, domination_reduction>(global_status.n);
		global_reduction_map.resize(REDUCTION_NUM); 
		for(size_t i = 0; i < global_status.reductions.size(); i++) {
			global_reduction_map[global_status.reductions[i]->get_reduction_type()] = i; // Hm....
		}

		set_local_reductions = [this, called_from_fold]() {
			status.reductions = make_reduction_vector<domination_reduction, clique_reduction>(status.n);
			local_reduction_map.resize(REDUCTION_NUM);
			for(size_t i = 0; i < status.reductions.size(); i++) {
				local_reduction_map[status.reductions[i]->get_reduction_type()] = i; 
			}
		}; 
	}

// set status of a node. If 'included': exclude neighbors, set 2-neighbors to 'unsafe'
void reduce_algorithm::set_imprecise(NodeID node, pack_status mpack_status) {
	if(mpack_status == pack_status::included) {
		status.node_status[node] = mpack_status; 
		status.remaining_nodes--;
		status.pack_weight += status.weights[node]; 
		status.graph.hide_node_imprecise(node); 

		for(auto neighbor : status.graph[node]) {
			status.node_status[neighbor] = pack_status::excluded; 
			status.remaining_nodes--; 
			status.graph.hide_node_imprecise(neighbor); 
		}

		for(auto neighbor : status.graph.get2neighbor_list(node)) {
			status.node_status[neighbor] = pack_status::unsafe; 	
		}
	} else {
		status.node_status[node] = mpack_status; 
		status.remaining_nodes--; 
		status.graph.hide_node_imprecise(node); 
	}
}

size_t reduce_algorithm::deg(NodeID node) const {
	return status.graph[node].size(); 
}

size_t reduce_algorithm::two_deg(NodeID node) {
	return status.graph.get2neighbor_list(node).size();
}

void reduce_algorithm::add_next_level_node(NodeID node) {
	// mark node for next round of status.reductions...
	for(auto& reduction : status.reductions) {
		if(reduction->has_run) {
			reduction->marker.add(node); // mark: Probably because something has changed? 
		}
	}
}

void reduce_algorithm::add_next_level_neighborhood(NodeID node) {
	// node has been excluded in M2PS -> neighbouring vertices are interesting for the next round of reduction. 
	for(auto neighbor : status.graph[node]) {
		add_next_level_node(neighbor); 
	}

	// same for the neighbors with distance two...
	for(auto neighbor2 : status.graph.get2neighbor_list(node)) {
		add_next_level_node(neighbor2); 
	}
}

// For this function no adjustement is needed.
void reduce_algorithm::add_next_level_neighborhood(const std::vector<NodeID>& nodes) {
	for(auto node : nodes) {
		add_next_level_neighborhood(node); 
	}
}

void reduce_algorithm::init_reduction_step() {
	if(!status.reductions[active_reduction_index]->has_run) {
		status.reductions[active_reduction_index]->marker.fill_current_ascending(status.n);
		status.reductions[active_reduction_index]->marker.clear_next();	
		status.reductions[active_reduction_index]->has_run = true;
	}
	else {
		status.reductions[active_reduction_index]->marker.get_next(); 
	}
}

void reduce_algorithm::initial_reduce() {
	std::swap(global_reduction_map, local_reduction_map); 
	status = std::move(global_status); 
	reduce_graph_internal(); 

	global_status = std::move(status);
	std::swap(global_reduction_map, local_reduction_map); 
}

void reduce_algorithm::reduce_graph_internal() {
	bool progress; 

	do {
		progress = false; 

		for(auto& reduction : status.reductions) {
			active_reduction_index = local_reduction_map[reduction->get_reduction_type()];  

			init_reduction_step(); 
			progress = reduction->reduce(this); 

			if(progress) break; 

			active_reduction_index++; 
		}
	} while(progress); 

	// brauche hier kein branching_token weil ich ja nicht branche...
}

void reduce_algorithm::run_reductions() {
	std::cout << "Start reducing the graph..." << std::endl; 
	initial_reduce(); 
}

// gives the actual 2-packing set so far 
std::vector<NodeID> reduce_algorithm::get_status() {

	std::cout << "Weight of the two-packing set after reductions: " << global_status.pack_weight << std::endl;

	std::cout << std::endl; 

	std::vector<NodeID> set_so_far(global_status.n,0); 
	for(size_t i = 0; i < set_so_far.size(); i++) {
		if(global_status.node_status[i] == 1) {
			set_so_far[i] = 1; 
		}
	}

	return set_so_far;

}

