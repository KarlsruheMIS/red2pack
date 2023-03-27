/*****************************************************************************************
 * reductions_pack.cpp
 *
 ****************************************************************************************/

#include "reductions_pack.h"
#include "reduce_algorithm.h"


#include<utility>


typedef reduce_algorithm::pack_status pack_status; 
// As far as I understand it, the marker stores the nodes that we want to look at with this reduction...

bool neighborhood_reduction::reduce(reduce_algorithm* algo) {
	auto& status = algo->status; 
	size_t oldn = status.remaining_nodes; 

	for(size_t v_idx = 0; v_idx < marker.current_size(); v_idx++) {
		NodeID v = marker.current_vertex(v_idx); 

		if(status.node_status[v] == pack_status::not_set) {
			NodeWeight neighbor_weights = 0; 
			
			// sum the weights of the neighbors of degree one.
			for(NodeID u: status.graph[v]) {
				if(status.node_status[u] != pack_status::unsafe) {
					neighbor_weights += status.weights[u];
				}
			} 
			
			// sum the the weights of the neighbors of degree two. 
			for(NodeID w: status.graph.get2neighbor_list(v)) {
				if(status.node_status[w] != pack_status::unsafe) {	
					neighbor_weights += status.weights[w]; 
				}
			}
			
			if(status.weights[v] >= neighbor_weights) {
				std::cout << "Include node: " << v << std::endl; 
				algo->set_imprecise(v, pack_status::included); 
			}
		}
	}

	return oldn != status.remaining_nodes;
       	
}

bool domination_reduction::reduce(reduce_algorithm* algo) {
	auto& status = algo->status;  
	fast_set set_1(algo->status.n); 
	auto& neighbors = set_1; 
	size_t oldn = status.remaining_nodes; 
	
	for(size_t v_idx = 0; v_idx < marker.current_size(); v_idx++) {
		NodeID v = marker.current_vertex(v_idx); 

		if(status.node_status[v] == pack_status::not_set) {
			NodeWeight neighbors_weight = 0;
		       	size_t neighbors_count = 0; 
			neighbors.clear(); 

			for(NodeID neighbor : status.graph[v]) {
				if(status.node_status[neighbor] != pack_status::unsafe) {
					neighbors.add(neighbor); 
					neighbors_weight += status.weights[neighbor]; 
					neighbors_count++; 
				}
			}
			
			for(NodeID neighbor : status.graph.get2neighbor_list(v)) {
				if(status.node_status[neighbor] != pack_status::unsafe) {
					neighbors.add(neighbor); 
					neighbors_weight += status.weights[neighbor]; 
					neighbors_count++;
				}
			}
			
			if(status.weights[v] >= neighbors_weight) {
				algo->set_imprecise(v, pack_status::included); 
				continue; 
			}

			neighbors.add(v); 
			bool is_subset; 

			for(NodeID neighbor : status.graph[v]) {
				if(algo->deg(neighbor) + algo->two_deg(neighbor) > neighbors_count || status.node_status[neighbor] == pack_status::unsafe) {
					continue;
				}

				is_subset = true; 

				for(NodeID neighbor2 : status.graph[neighbor]) {
					if(status.node_status[neighbor2] != pack_status::unsafe) {
						if(!neighbors.get(neighbor2)) {
							is_subset = false;
							break; 
						}
					}
				}
				
				for(NodeID neighbor3 : status.graph.get2neighbor_list(neighbor)) {
					if(status.node_status[neighbor3] != pack_status::unsafe) {	
						if(!neighbors.get(neighbor3)) {
							is_subset = false; 
							break; 
						}
					}
				}
				
				if(is_subset && status.weights[neighbor] >= status.weights[v]) {
					algo->set_imprecise(v, pack_status::excluded);
				       	break; 	
				}
			}	
		}
	}
	return oldn != status.remaining_nodes; 
}

bool clique_reduction::reduce(reduce_algorithm* algo) {
	
	auto& status = algo->status; 
	//auto& set_1 = algo->set_1; 
	//auto& neighbors = algo->buffers[0]; 
	//auto& isolated = algo->buffers[1];
	fast_set set_1(algo->status.n);
       	sized_vector<sized_vector<NodeID>> buffers(2, sized_vector<NodeID>(algo->status.n));
	auto& neighbors = buffers[0];
	auto& isolated = buffers[1]; 
	std::vector<NodeID> non_isolated; 

	size_t oldn = status.remaining_nodes; 
	
	for(size_t node_idx = 0; node_idx < marker.current_size(); node_idx++) {
		NodeID node = marker.current_vertex(node_idx); 

		if(status.node_status[node] == pack_status::not_set) {
		
			neighbors.clear(); 
			set_1.clear(); 
			set_1.add(node); 
		
			// find potential 2-clique
			for(NodeID neighbor : status.graph[node]) { // add neighbors with distance one.
				if(status.node_status[node] != pack_status::unsafe) {
					neighbors.push_back(neighbor); 
					set_1.add(neighbor);
				}
			}

			for(NodeID neighbor2 : status.graph.get2neighbor_list(node)) { // add neighbors with distance two.
				if(status.node_status[node] != pack_status::unsafe) {
					neighbors.push_back(neighbor2); 
					set_1.add(neighbor2); 
				}
			}
			
			
			// check if 2-clique 
			isolated.clear(); 
			isolated.push_back(node); 
			non_isolated.clear(); 
			
			size_t max_isolated_idx = 0; 
			weighted_node max_isolated{ node, status.weights[node] }; 
			weighted_node max_non_isolated{0, 0}; 

			bool is_clique = false; 

			for(auto neighbor : neighbors) {
				if(status.node_status[neighbor] != pack_status::unsafe) {
					size_t count = 0; 
					bool is_isolated = true; 

					for(NodeID neighbor_2nd : status.graph[neighbor]) {
						if(status.node_status[neighbor] != pack_status::unsafe) {
							if(set_1.get(neighbor_2nd)) count++; 
							else is_isolated = false;
						}
					}

					// same for the neighbors with distance two
					for(NodeID neighbor_2nd : status.graph.get2neighbor_list(neighbor)) {
						if(status.node_status[neighbor] != pack_status::unsafe) {
							if(set_1.get(neighbor_2nd)) count++; 
							else is_isolated = false; 
						}
					}
	
					if(is_isolated) { // dieses if und das nächste else sind zwar technisch klar aber ich sehe noch nicht ganz warum das gemacht wird... 
						isolated.push_back(neighbor); 
						if(status.weights[neighbor] > max_isolated.weight) {
							max_isolated = {neighbor, status.weights[neighbor] }; 
							max_isolated_idx = isolated.size() -1; 
						} 
					}
					else {
						non_isolated.push_back(neighbor); 
						if(status.weights[neighbor] > max_non_isolated.weight) {
							max_non_isolated = { neighbor, status.weights[neighbor] };
						}
					}
	
					is_clique = count == neighbors.size(); 
					if(!is_clique) break;
				}
			}
 
			if(!is_clique) continue;

			// one of "isolated" members has highest weight of 2-clique: Add to 2-Packing Set.
			if(max_isolated.weight >= max_non_isolated.weight) {
				algo->set_imprecise(max_isolated.node, pack_status::included); 
				continue; 
			}

			/* // Das wird im ungewichteten Fall irrelevant.
			// remove all nodes from the 2-clique which have a smaller or equal weight than "max_isolated" -> we can always pick "max_isolated" over them.
			isolated[max_isolated_idx] = isolated.back(); 
			isolated.pop_back(); 

			for(auto neighbor : isolated) {
				algo->new_set(neighbor, pack_status::excluded); 
			}

			for(size_t i = 0; i < non_isolated.size(); i++) {
				NodeID neighbor = non_isolated[i]; 
				if(status.weights[neighbor] <= max_isolated.weight) {
					algo->new_set(neighbor, pack_status::excluded); 
					non_isolated[i] = non_isolated.back(); 
					non_isolated.pop_back(); 
					i--; 	
				}
			}
			fold(algo, std::move(max_isolated), std::move(non_isolated));
			*/
		}
	}

	return oldn != status.remaining_nodes;
}




