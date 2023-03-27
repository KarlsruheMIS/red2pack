#include <iostream>
#include <algorithm>
#include <fstream>
#include <random>
#include <sstream>
#include <set>
#include <queue>
#include <ctime>
#include <chrono>

#include "../../extern/KaHIP/interface/kaHIP_interface.h"
#include "dynamic_graph_pack.h"
#include "../../extern/KaHIP/lib/data_structure/graph_access.h"
#include "../mis/kernel/reduce_algorithm.h"
#include "../mis/kernel/reductions_pack.h"
 
typedef reduce_algorithm::pack_status pack_status;
// This is our graph data structure with functionality.

class evo_graph {
	public: 
// The next three classes are helpful during our computations. The first two for the initial population.
// The third for the tournament selection.

class two_deg_node {
	public: 
		NodeID v; 
		int deg; 

		bool operator<(const two_deg_node& r) const {
			return deg > r.deg; 
		}
};

class weighted_node {
	public: 
		NodeID v; 
		NodeWeight weight; 

		bool operator<(const weighted_node& rhs) const {
			return weight > rhs.weight; 
		}
};

class deg_node {
	public: 
		NodeID v; 
		int deg; 

		bool operator<(const deg_node& r) const {
			return deg > r.deg; 
		}
}; 

class tournament_cand {
	public: 
		int candidate; 
		int fitness; 

		bool operator<(const tournament_cand& r) const {
			return fitness > r.fitness; 
		}
};
		evo_graph() {}

		 
		int read_graph(const std::string& filename) {
			std::string filename1; 
			std::string line1; 
			std::ifstream in1(filename.c_str()); 
			if(!in1) {
				std::cerr << "Error opening " << filename << std::endl; 
				return 1; 
			}
			std::getline(in1, line1); 
			while(line1[0] == '%') {
				std::getline(in1, line1); 
			}
			int is_weighted = -1; 
			std::stringstream ss(line1); 
			ss >> n_org; 
			ss >> m_org;
		       	ss >> is_weighted; 	
			xadj_org = new int[n_org+1];
			adjncy_org = new int[2*m_org]; 
			int vertex = 0;
			int counter = 0; 

			while(std::getline(in1, line1)) {
				if(line1[0] == '%') {
					continue; 
				}
				xadj_org[vertex] = counter; 
				std::stringstream ss(line1); 
				int node; 
				if(is_weighted == 10) {
					int weight; 
					ss >> weight;
				       	vwgt_org[vertex] = weight; 	
				}
				while(ss >> node) {
					adjncy_org[counter] = node -1; 
					counter++; 
				}

				vertex++;

				if(vertex == n_org) {
					xadj_org[n_org] = counter; 
				}

				if(in1.eof()) {
					break; 
				}	
			}

			return 0; 
		}
		/*
		void init_graph_2(reduce_algorithm& algo) {
		       	auto& status = algo.global_status; 	
			n = status.graph.size();  
			nodes_to_ignore.resize(n);
			new_ids.resize(n); 
			int how_many = 0; 
			int new_id = 0; 
			for(int i = 0; i < n; i++) {
				if(status.graph.hided_nodes[i] == true || status.weights[i] == 0) {
					nodes_to_ignore[i] = true;
					new_ids[i] = -1; 
				} else {
					how_many++; 
					new_ids[i] = new_id; 
					new_id++; 
				}	
			}


			hidden_nodes = status.graph.hided_nodes; 

			// Build adj_two_list...
			int counter = 0; 
			int vertex = 0; 
			int scd_counter = 0; 
			adj_two_list.resize(how_many); 
			xadj = new int[how_many+1]; 
			for(NodeID v = 0; v < n; v++) {
				if(hidden_nodes[v] == false && status.weights[v] != 0) { 
					xadj[vertex] = scd_counter; 	
					for(long unsigned int i = 0; i < status.graph[v].size(); i++) {
						if(nodes_to_ignore[status.graph[v][i]] == false) {
							adj_two_list[counter].push_back(new_ids[status.graph[v][i]]); 
							help_adjncy.push_back(new_ids[status.graph[v][i]]); 
							scd_counter++; 
						}
					} 
					for(long unsigned int j = 0; j < status.graph.get2neighbor_list(v).size(); j++) {
						if(nodes_to_ignore[status.graph.get2neighbor_list(v)[j]] == false) {	
							adj_two_list[counter].push_back(new_ids[status.graph.get2neighbor_list(v)[j]]); 
						}
					}
					old_ids.push_back(v); 
					node_weights.push_back(status.weights[v]); 
					counter++;
					vertex++; 
					if(vertex == how_many) {
						xadj[vertex] = scd_counter; 
					}
				}
			}
			adjncy = new int[help_adjncy.size()]; 
			for(long unsigned int i = 0; i < help_adjncy.size(); i++) {
				adjncy[i] = help_adjncy[i]; 
			}
			*/
			/*
			// output the graph for test reasons..
			for(int i = 0; i < how_many; i++) {
				std::cout << i+1 << ": ";  
				for(int j = xadj[i]; j < xadj[i+1]; j++) {
					std::cout << adjncy[j]+1 << " "; 
				}
				std::cout << "\n";  
			}
			*/
			/*
			std::cout << "The graph as stored in the adj_two_list: " << std::endl; 
			// For test purposes output the graph...
			for(long unsigned int i = 0; i < G_dyn.size(); i++) {
				std::cout << i+1 << ": "; 
				if(hidden_nodes[i] == false) {
					for(long unsigned int j = 0; j < G_dyn[i].size(); j++) {
						std::cout << adj_two_list[i][j]+1 << " "; 
					}
				}
				std::cout << "\n"; 
			}
			*/
		       	/*	
			n = how_many; 
		} */
		/*
		void init_graph(reduce_algorithm& algo) {
		       	auto& status = algo.global_status; 	
			n = status.graph.size(); 
			adj_two_list.resize(n); 
			nodes_to_ignore.resize(n);
			for(int i = 0; i < n; i++) {
				nodes_to_ignore[i] = false; 
			}

			hidden_nodes = status.graph.hided_nodes; 

			// Build adj_two_list...
			for(NodeID v = 0; v < n; v++) {
				if(hidden_nodes[v] == false) {
					for(long unsigned int i = 0; i < status.graph[v].size(); i++) {
						adj_two_list[v].push_back(status.graph[v][i]); 
					} 
					for(long unsigned int j = 0; j < status.graph.get2neighbor_list(v).size(); j++) {
						adj_two_list[v].push_back(status.graph.get2neighbor_list(v)[j]); 
					}
				}
			}
			node_weights.resize(n); 
			for(long unsigned int i = 0; i < n; i++) {
				node_weights[i] = status.weights[i]; 
			}
			*/
			/*
			std::cout << "The graph as stored in the adj_two_list: " << std::endl; 
			// For test purposes output the graph...
			for(long unsigned int i = 0; i < G_dyn.size(); i++) {
				std::cout << i+1 << ": "; 
				if(hidden_nodes[i] == false) {
					for(long unsigned int j = 0; j < G_dyn[i].size(); j++) {
						std::cout << adj_two_list[i][j]+1 << " "; 
					}
				}
				std::cout << "\n"; 
			}
			*/
			/*
			for(int i = 0; i < n; i++) {
				if(adj_two_list[i].size() == 0 || node_weights[i] == 0) {
					nodes_to_ignore[i] = true;
				}
			}
		}*/

		void build_from_kernel(reduce_algorithm& algo) {
		       	auto& status = algo.global_status; 	
			n = status.graph.size(); 
			unsafe_nodes.resize(n);
		       	new_ids.resize(n); 	
			int n_new = 0; // new number of nodes. 
			int m_new = 0; // new number of edges.

			// We mark the unsafe_nodes. 
			for(int i = 0; i < n; i++) {
				if(status.node_status[i] == pack_status::unsafe) {
					unsafe_nodes[i] == true; 
				} else {
					unsafe_nodes[i] == false; 
				}
			}
			for(size_t i = 0; i < status.graph.size(); i++) {
				if(status.node_status[i] != pack_status::excluded && status.node_status[i] != pack_status::included) {
					new_ids[i] = n_new;
				       	old_ids.push_back(i); 	
					n_new++;	
				} else {
					new_ids[i] = -1; // i is not in the new graph.
				}
			}
			adj_two_list.resize(n_new);
			adj_list.resize(n_new); 
		       	new_degrees.resize(n_new); 	
			for(size_t i = 0; i < status.graph.size(); i++) {
				if(status.node_status[i] != pack_status::excluded && status.node_status[i] != pack_status::included) {
					new_degrees[new_ids[i]] = 0; 
					for(size_t j = 0; j < status.graph[i].size(); j++) { 
						if(status.node_status[status.graph[i][j]] != pack_status::excluded && status.node_status[i] != pack_status::included) {
							adj_two_list[new_ids[i]].push_back(new_ids[status.graph[i][j]]);
							adj_list[new_ids[i]].push_back(new_ids[status.graph[i][j]]); 
							m_new++;
							new_degrees[new_ids[i]]++; 	
						}
					}
					std::sort(adj_list[new_ids[i]].begin(), adj_list[new_ids[i]].end()); 
					for(size_t j = 0; j < status.graph.get2neighbor_list(i).size(); j++) {
						if(status.node_status[status.graph.get2neighbor_list(i)[j]] != pack_status::excluded && status.node_status[status.graph.get2neighbor_list(i)[j]] != pack_status::included) {
							adj_two_list[new_ids[i]].push_back(new_ids[status.graph.get2neighbor_list(i)[j]]); 
						}
					}
					std::sort(adj_two_list[new_ids[i]].begin(), adj_two_list[new_ids[i]].end()); 
				}
			}
			// NOTE: m_new counts the forward and the backward edge!

			// build xadj, adjncy for the node_separator on the kernel. 
			
			int vertex = 0; 
			int counter = 0; 
			xadj = new int[n_new+1]; 
			adjncy = new int[m_new]; 
			n_kernel = n_new;
			m_kernel = m_new/2; 
			for(size_t i = 0; i < n_new; i++) {
				xadj[vertex] = counter; 
				for(size_t j = 0; j < new_degrees[i]; j++) {
					adjncy[counter] = adj_list[i][j]; 
					counter++; 
				}
				vertex++; 
				if(vertex == n_new) {
					xadj[vertex] = counter; 
				}
			}
			
			/*
			for(int i = 0; i < n_kernel; i++) {
				std::cout << i << ": "; 
				for(int j = xadj[i]; j < xadj[i+1]; j++) {
					std::cout << adjncy[j] << " "; 
				}
				std::cout << "\n"; 
			}
			*/
			// store the node_weights.
			node_weights.resize(n_new); 
			for(size_t i = 0; i < n_new; i++) {
					if(status.node_status[old_ids[i]] == pack_status::unsafe) {
						node_weights[i] = 0;
					} else {
						node_weights[i] = status.weights[old_ids[i]]; 
					}
			}
			
			vwgt = new int[n_new]; 
			for(size_t i = 0; i < n_new; i++) {
				vwgt[i] = node_weights[i]; 
			}
			
		}                


		// This is a control function to check if the vector defines a two packing set.  
		int is_two_packing_set(std::vector<int>& sol, reduce_algorithm& algo) {
			int stop = 0; 
			for(int i = 0; i < n_kernel; i++) {
				if(sol[i] == 1) {
					for(long unsigned int j = 0; j < adj_two_list[i].size(); j++) {
						if(sol[adj_two_list[i][j]] == 1) {
							stop = 1; 
							std::cout << old_ids[i] << " " << algo.global_status.node_status[old_ids[i]] <<  ". " << old_ids[adj_two_list[i][j]] << " " << algo.global_status.node_status[adj_two_list[i][j]] << std::endl; 
						}
					}
				}
			}
			return stop; 
		}

		// Here we compute the 2-tightness of a node.
		void two_tightness(std::vector<int>& sol, std::vector<int>& tight_nodes) {
			for(int i = 0; i < n_kernel; i++) {
					tight_nodes[i] = 0;
			}
			for(int i = 0; i < n_kernel; i++) {
				if(sol[i] == 1) {
					tight_nodes[i] = -1; 
				}
			}
			for(int i = 0; i < n_kernel; i++) {
				if(sol[i] == 1) {
					for(long unsigned int j = 0; j < adj_two_list[i].size(); j++) {
						tight_nodes[adj_two_list[i][j]]++; 
					}
				}
			}
		}
		// Here we maximize the offspring, assuming it is a {0,1}-vector. We do the steps in a random order so diversity is guaranteed.
		void maximize_individual(std::vector<int>& sol_candidate, int random_seed, reduce_algorithm& algo, std::vector<NodeID>&  nodes) {
			int stop = 0; 
			std::shuffle(std::begin(nodes), std::end(nodes), std::default_random_engine(random_seed)); 
			for(long unsigned int i = 0; i < nodes.size(); i++) {
				if(sol_candidate[nodes[i]] == 0) {
					for(long unsigned int j = 0; j < adj_two_list[nodes[i]].size(); j++) {
						if(sol_candidate[adj_two_list[nodes[i]][j]] == 1) {
							stop = 1; 
						}
					}
					if(stop == 0) {
						if(algo.global_status.node_status[old_ids[nodes[i]]] != pack_status::unsafe) {	
							sol_candidate[nodes[i]] = 1; 
						}
					} else {
						stop = 0; 
					}
				}
			}
		}
		// Here we implement our mutation. Perturbation steps + local_search for some rounds. 
		void mutation_of_individual(std::vector<int>& offspring, int offspring_size, int random_seed, reduce_algorithm& algo, std::vector<NodeID>& nodes) {
			std::default_random_engine mutation_generator(random_seed); 
			std::uniform_int_distribution<int> mutation_distribution; 
			
			// how many vertices are in solution...
			int how_many = 0; 
			for(int i = 0; i < n_kernel; i++) {
				if(offspring[i] == 1) {
					how_many++; 
				}
			}
			double f = 0.007* how_many; 
			int amount_of_vertices = f; 
			
			std::vector<int> pertubation_vertices; 
			std::default_random_engine pertubation_generator(mutation_distribution(mutation_generator));
			std::uniform_int_distribution<int> pertubation_distribution(0,n_kernel-1); 
			for(int i = 0; i < amount_of_vertices; i++) {
				pertubation_vertices.push_back(pertubation_distribution(pertubation_generator)); 
			}

			// Now find randomly vertices that are not in the solution and put it into the solution.
			// Fix the solution afterwards.
			
			int counter = 0;	
			while(counter < f) {
				if(algo.global_status.node_status[old_ids[pertubation_vertices[counter]]] != pack_status::unsafe) {
					int set_node = pertubation_vertices[counter]; 
					if(offspring[set_node] == 0) {
						offspring[set_node] = 1; 
						for(long unsigned int j = 0; j < adj_two_list[set_node].size(); j++) {
							if(offspring[adj_two_list[set_node][j]] == 1) {
								offspring[adj_two_list[set_node][j]] = 0;
							}
						}	 
					}  	
				}
				counter++; 
			}

			// Now we have put randomly nodes into the solution and corrected it. 
			// Maybe the solution got worse so we do some rounds local search again. 
			std::vector<int> tight_nodes(n_kernel); 
		       	two_tightness(offspring, tight_nodes); 	
		      for(int t = 0; t < 5000; t++) { 
		       		// In every round we look at a different order of the nodes. 
				std::shuffle(std::begin(nodes), std::end(nodes), std::default_random_engine(mutation_distribution(mutation_generator))); 
				int nein = 0; 
				for(int i = 0; i < nodes.size(); i++) {
					if(offspring[nodes[i]] == 1) { 
						for(long unsigned int j = 0; j < adj_two_list[nodes[i]].size(); j++) {
							int candidate = adj_two_list[nodes[i]][j];
							 if(tight_nodes[candidate] == 1 && algo.global_status.node_status[old_ids[candidate]] != pack_status::unsafe) {	
								for(long unsigned int k = 0; k < adj_two_list[nodes[i]].size(); k++) {
									if(candidate != adj_two_list[nodes[i]][k]) {
										int second_candidate = adj_two_list[nodes[i]][k]; 
										if(tight_nodes[second_candidate] == 1 && algo.global_status.node_status[old_ids[second_candidate]] != pack_status::unsafe) {
											for(long unsigned int n = 0; n < adj_two_list[candidate].size(); n++) {
												if(adj_two_list[candidate][n] == second_candidate) {
													nein = 1; 
												}
											}
											if(nein == 0) {
												if(node_weights[nodes[i]] < node_weights[candidate] + node_weights[second_candidate]) {
													offspring[nodes[i]] = 0; 
													offspring[candidate] = 1; 
													offspring[second_candidate] = 1; 
											
													for(long unsigned int a = 0; a < adj_two_list[nodes[i]].size(); a++) {
														if(offspring[adj_two_list[nodes[i]][a]] != 1) {
															tight_nodes[adj_two_list[nodes[i]][a]]--;
														}
													}
													for(long unsigned int a = 0; a < adj_two_list[candidate].size(); a++) {
														if(offspring[adj_two_list[candidate][a]] != 1) {
															tight_nodes[adj_two_list[candidate][a]]++;
														}
													}
													for(long unsigned int a = 0; a < adj_two_list[second_candidate].size(); a++) {
														if(offspring[adj_two_list[second_candidate][a]] != 1) {
															tight_nodes[adj_two_list[second_candidate][a]]++;
														}
													}
													tight_nodes[candidate] = -1; 
													tight_nodes[second_candidate] = -1; 
													int tight = 0; 
													for(long unsigned int c = 0; c < adj_two_list[nodes[i]].size(); c++) {
														if(offspring[adj_two_list[nodes[i]][c]] == 1) {
															tight++; 
														} 
													}
													tight_nodes[nodes[i]] = tight; 
													k = adj_two_list[nodes[i]].size(); 
													j = adj_two_list[nodes[i]].size(); 
												}
											} else {
												nein = 0; 
											}
										}
									}
								}
						       }
						}
					}
				}
			}
				
		}
		// Here we implemented the local search. There are ways to do this faster...
		// We can call it for some time or for some rounds.
		void local_search(std::vector<int>& sol, int time_to_run, std::vector<NodeID>& nodes, int seed, int rounds, reduce_algorithm& algo) {

			std::default_random_engine generator(seed); 
			std::uniform_int_distribution<int> distribution;
			std::vector<int> tight_nodes(n_kernel);
		       	two_tightness(sol, tight_nodes);
			if(time_to_run != 0) {
			auto start = std::chrono::high_resolution_clock::now(); 
			auto stop_time = std::chrono::high_resolution_clock::now(); 
			auto time = std::chrono::duration_cast<std::chrono::seconds>(stop_time-start); 	
			while(time.count() < time_to_run) { 
				std::shuffle(std::begin(nodes), std::end(nodes), std::default_random_engine(distribution(generator)));  
				int nein = 0; 
				for(long unsigned int i = 0; i < nodes.size(); i++) {
					if(sol[nodes[i]] == 1) { 
						for(long unsigned int j = 0; j < adj_two_list[nodes[i]].size(); j++) {
							int candidate = adj_two_list[nodes[i]][j];
							 if(tight_nodes[candidate] == 1 && algo.global_status.node_status[old_ids[candidate]] != pack_status::unsafe) {
								for(long unsigned int k = 0; k < adj_two_list[nodes[i]].size(); k++) {
									if(candidate != adj_two_list[nodes[i]][k]) {
										int second_candidate = adj_two_list[nodes[i]][k]; 
										if(tight_nodes[second_candidate] == 1 && algo.global_status.node_status[old_ids[second_candidate]] != pack_status::unsafe) {
											for(long unsigned int n = 0; n < adj_two_list[candidate].size(); n++) {
												if(adj_two_list[candidate][n] == second_candidate) {
													nein = 1; 
												}
											}
											if(nein == 0) {
												if(node_weights[nodes[i]] < node_weights[candidate]+node_weights[second_candidate]) {
													sol[nodes[i]] = 0; 
													sol[candidate] = 1; 
													sol[second_candidate] = 1; 
											
													for(long unsigned int a = 0; a < adj_two_list[nodes[i]].size(); a++) {
														if(sol[adj_two_list[nodes[i]][a]] != 1) {
															tight_nodes[adj_two_list[nodes[i]][a]]--;
														}
													}
													for(long unsigned int a = 0; a < adj_two_list[candidate].size(); a++) {
														if(sol[adj_two_list[candidate][a]] != 1) {
															tight_nodes[adj_two_list[candidate][a]]++;
														}
													}
													for(long unsigned int a = 0; a < adj_two_list[second_candidate].size(); a++) {
														if(sol[adj_two_list[second_candidate][a]] != 1) {
															tight_nodes[adj_two_list[second_candidate][a]]++;
														}
													}
													tight_nodes[candidate] = -1; 
													tight_nodes[second_candidate] = -1; 
													int tight = 0; 
													for(long unsigned int c = 0; c < adj_two_list[nodes[i]].size(); c++) {
														if(sol[adj_two_list[nodes[i]][c]] == 1) {
															tight++; 
														} 
													}
													tight_nodes[nodes[i]] = tight; 
													k = adj_two_list[nodes[i]].size(); 
													j = adj_two_list[nodes[i]].size(); 
												}	
											} else {
												nein = 0; 
											}
										}
									}
								}
						       	}
						}
					}
				}
				stop_time = std::chrono::high_resolution_clock::now(); 
				time = std::chrono::duration_cast<std::chrono::seconds>(stop_time-start);
			}
			} else {
			for(int r = 0; r < rounds; r++) { 
				std::shuffle(std::begin(nodes), std::end(nodes), std::default_random_engine(distribution(generator)));  
				int nein = 0; 
				for(long unsigned int i = 0; i < nodes.size(); i++) {
					if(sol[nodes[i]] == 1) { 
						for(long unsigned int j = 0; j < adj_two_list[nodes[i]].size(); j++) {
							int candidate = adj_two_list[nodes[i]][j];
							 if(tight_nodes[candidate] == 1 && algo.global_status.node_status[old_ids[candidate]] != pack_status::unsafe) {	
								for(long unsigned int k = 0; k < adj_two_list[nodes[i]].size(); k++) {
									if(candidate != adj_two_list[nodes[i]][k]) {
										int second_candidate = adj_two_list[nodes[i]][k]; 
										if(tight_nodes[second_candidate] == 1 && algo.global_status.node_status[old_ids[second_candidate]] != pack_status::unsafe) {
											for(long unsigned int n = 0; n < adj_two_list[candidate].size(); n++) {
												if(adj_two_list[candidate][n] == second_candidate) {
													nein = 1; 
												}
											}
											if(nein == 0) {
												if(node_weights[nodes[i]] < node_weights[candidate] + node_weights[second_candidate]) {
													sol[nodes[i]] = 0; 
													sol[candidate] = 1; 
													sol[second_candidate] = 1; 
											
													for(long unsigned int a = 0; a < adj_two_list[nodes[i]].size(); a++) {
														if(sol[adj_two_list[nodes[i]][a]] != 1) {
															tight_nodes[adj_two_list[nodes[i]][a]]--;
														}
													}
													for(long unsigned int a = 0; a < adj_two_list[candidate].size(); a++) {
														if(sol[adj_two_list[candidate][a]] != 1) {
															tight_nodes[adj_two_list[candidate][a]]++;
														}
													}
													for(long unsigned int a = 0; a < adj_two_list[second_candidate].size(); a++) {
														if(sol[adj_two_list[second_candidate][a]] != 1) {
															tight_nodes[adj_two_list[second_candidate][a]]++;
														}
													}
													tight_nodes[candidate] = -1; 
													tight_nodes[second_candidate] = -1; 
													int tight = 0; 
													for(long unsigned int c = 0; c < adj_two_list[nodes[i]].size(); c++) {
														if(sol[adj_two_list[nodes[i]][c]] == 1) {
															tight++; 
														} 
													}
													tight_nodes[nodes[i]] = tight; 
													k = adj_two_list[nodes[i]].size(); 
													j = adj_two_list[nodes[i]].size();
												}
											} else {
												nein = 0; 
											}
										}
									}
								}
						       }
						}
					}
				}
			}		
			}	
		}

		std::vector<int> run_evo(int run_seed, reduce_algorithm& algo) { // TODO: HIER WILL ICH NOCH AUS DEM RÜCKGABEWERT EINE REFERENZ MACHEN.
	 
			std::cout << "========================================================================" << std::endl; 
			std::cout << "                   Start evolutionary algorihtm." << std::endl;
		       	std::cout << "========================================================================" << std::endl; 
			std::cout << "\n";
			
			/*
			// Get a timestamp so we know which output corresponds to which run of the algorithm.
			time_t rawtime; 
			struct tm *timeinfo; 

			// Get time and date of running the algorithm. 
			time(&rawtime); 
			timeinfo = localtime(&rawtime); 
			std::cout << asctime(timeinfo) << std::endl;
			*/

			//std::cout << n << std::endl; 
			int random_seed = run_seed; // One random seed as input that defines all randomness in our algorithm.
			std::cout << "The random seed of this run is: " << random_seed << std::endl; 
			std::cout << "\n"; 

			std::default_random_engine generator(random_seed); 
			std::uniform_int_distribution<int> distribution; 
			
			std::cout << "Start running time." << std::endl; 
			std::cout << "\n"; 

			// Starting point for measuring running times.
			auto start_point = std::chrono::high_resolution_clock::now(); 
			auto stop_time = std::chrono::high_resolution_clock::now();
			auto time_now = std::chrono::duration_cast<std::chrono::seconds>(stop_time-start_point); 
			
			
			std::cout << "Compute weighted node separator of the graph for combine operation ..." << std::endl; 
			// We build the node separator for the independent set problem using kaHIP.
			int* vwgt_test = NULL; 
			int* adjcwgt = NULL; // The edges are unweighted.
			int nparts = 2; // Number of blocks.
			double imbalance = 0.03; // Imbalance of the blocks.
			bool suppress_output = true; // We don't need the output of the algorithm as we just want to use the outcome.
			int seed = distribution(generator); // The algorithm needs a random seed.
			int num_of_vertices; // This is the size of the separator.
			int *separator = NULL; // Here we have the nodes that are in the separator.
		       	int* part = new int [n_org]; // This gives us the actual partition of V into V_1 \cup V_2 \cup S, where S is the separator.
			
			// Node separator call with KaHIP
			node_separator(&n_org, vwgt_test, xadj_org, adjcwgt, adjncy_org, &nparts, &imbalance, suppress_output, seed, ECO, &num_of_vertices, &separator, part);
			std::cout << "Done." << std::endl; 
			std::cout << "\n"; 
			
			// We have to assign the blocks in the separator to another block.
			for(int i = 0; i < num_of_vertices; i++) {
				part[separator[i]] = 2; 
			}

			// This is the partition we will work with.
			std::vector<int> partition_org; 
			for(int i = 0; i < n_org; i++) {
				partition_org.push_back(part[i]); 
			}
			
			
			// Now we will extend the node separator so we can use it for the 2-packing set problem. 
			// Since our node separator should be as small as possible we have to find out which "next layer to the separator" is smaller.
			std::vector<int> layer_0; // The next layer in block V_0.
			std::vector<int> layer_1; // The next layer in block V_1.
      
       			int second_layer_0_size = 0; // Size of layer_0.
	        	int second_layer_1_size = 0; // Size of layer_1.
        		for(int i = 0; i < n_org; i++) {
				if(partition_org[i] == 2) {
					for(int j = xadj_org[i]; j < xadj_org[i+1]; j++) {
						if(partition_org[adjncy_org[j]] == 0) {
							layer_0.push_back(adjncy_org[j]); 
							second_layer_0_size++; 
						} else if(partition_org[adjncy_org[j]] == 1) {
							layer_1.push_back(adjncy_org[j]);
							second_layer_1_size++;
						}
					}
				}
        		}

			// Now we actually build the separator depending on which layer is smaller. 
			if(second_layer_0_size < second_layer_1_size) {
				for(int i = 0; i < second_layer_0_size; i++) {
                		        partition_org[layer_0[i]] = 2; 
        			}
			} else {
				for(int i = 0; i < second_layer_1_size; i++) {
					partition_org[layer_1[i]] = 2; 
				}	
			}
			
			
			// Translate the partition of the original graph to a partition of the reduced graph
			// Idea: Separator of the original graph is in particular a separator of any subgraph.
			std::vector<int> partition; 
			for(int i = 0; i < n_kernel; i++) { 
				partition.push_back(partition_org[old_ids[i]]); 
			}
			

			/* // This is now unnecessary since we build a separator of the original graph
			for(int i = 0; i < n_kernel; i++) {
				if(partition[i] == 0) {
					for(int j = 0; j < adj_two_list[i].size(); j++) {
						if(partition[adj_two_list[i][j]] == 1) {
							partition[i] = -1; // conflict nodes can occur with hide_node_imprecise... 
							//std::cout << "BIG UUUF" << std::endl; 
						} 
					}
				}
				if(partition[i] == 1) {
					for(int j = 0; j < adj_two_list[i].size(); j++) {
						if(partition[adj_two_list[i][j]] == 0) {
							partition[i] = -1; // conflict nodes can occur with hide_node_imprecise...
							//std::cout << "BIG UUUF" << std::endl; 
						} 
					}
				}
			}
			*/
			
		  	
			std::cout << "Build initial solution greedily ..." << std::endl;   
			
			std::vector<std::vector<int>> solutions; // Here we store the solutions. 
			std::vector<int> sol_sizes; // Here we store the sizes of the solutions. 
		
			std::vector<NodeID> nodes; // This list of nodes will help us later on to traverse the nodes in a random order.
			for(int i = 0; i < n_kernel; i++) {
				if(algo.global_status.node_status[old_ids[i]] != pack_status::included && algo.global_status.node_status[old_ids[i]] != pack_status::excluded && algo.global_status.node_status[old_ids[i]] != pack_status::unsafe) {
					nodes.push_back(i); 
				}
			}
			std::cout << "Build random solutions ..." << std::endl; 
			std::cout << "\n"; 
			// We build 50 random solutions and improve the solution quality by some local search.
			for(int i = 0; i < 30; i++) {
				std::shuffle(std::begin(nodes), std::end(nodes), std::default_random_engine(distribution(generator))); 

				int stop = 0; 
				std::vector<int> s(n_kernel, 0); 
				for(long unsigned int i = 0; i < nodes.size(); i++) {
				//	if(nodes_to_ignore[nodes[i]] == false) {
						if(s[nodes[i]] == 0) {
							for(long unsigned int j = 0; j < adj_two_list[nodes[i]].size(); j++) {
								if(s[adj_two_list[nodes[i]][j]] == 1) {
									stop = 1; 
								}
							}
							if(stop == 0) {
								s[nodes[i]] = 1; 
							} else {
								stop = 0; 
							}
						}
					//}
				}
				local_search(s, 45, nodes, distribution(generator), 0, algo); // Local search for 45 seconds. 
				maximize_individual(s, distribution(generator), algo, nodes); 
				solutions.push_back(s);
				int sol_size = 0; 
				for(int i = 0; i < n_kernel; i++) {
					if(s[i] == 1) {
						sol_size += node_weights[i]; 
					}
				}
				sol_sizes.push_back(sol_size); 
			}
			
			std::cout << "Build initial solution by sorting the 2-degrees ..." << std::endl; 
			std::cout << "\n"; 

			// Next we build another 100 solutions in a greedy way with some local search to improve the solution quality. 
			std::priority_queue<two_deg_node> q; // We insert the nodes according to the size of the 2-neighborhood (smallest 2-neighborhood first).
			int q_size = 0; 
			for(int i = 0; i < n_kernel; i++) {
				if(algo.global_status.node_status[old_ids[i]] != pack_status::unsafe) {
					two_deg_node node; 
					node.v = i; 
					node.deg = adj_two_list[i].size(); 
					q.push(node); 
					q_size++; 
				}
			}

			std::vector<int> two_priority(q_size); 
			for(int i = 0; i < q_size; i++) {
				int x = (q.top()).v;
			       	q.pop(); 	
				two_priority[i] = x;
			}

			for(int i = 0; i < 20; i++) {
				int stop = 0; 
				std::vector<int> s_new(n_kernel, 0); 
				for(int j = 0; j < q_size; j++) { 
				//	if(nodes_to_ignore[two_priority[j]] == false) {
						if(s_new[two_priority[j]] == 0) {
							for(long unsigned int k = 0; k < adj_two_list[two_priority[j]].size(); k++) {
								if(s_new[adj_two_list[two_priority[j]][k]] == 1) {
									stop = 1; 
								}
							}
							if(stop == 0) {
								s_new[two_priority[j]] = 1; 
							} else {
								stop = 0; 
							}
						}
				//	}
				}
				local_search(s_new, 45, nodes, distribution(generator), 0, algo); // Local search for 45 seconds for diversity.
				maximize_individual(s_new, distribution(generator), algo, nodes); 
				solutions.push_back(s_new); 
				int sol_size = 0; 
				for(int l = 0; l < n_kernel; l++) {
					if(s_new[l] == 1) {
						sol_size+= node_weights[l]; 
					}
				}
				sol_sizes.push_back(sol_size);
			}
			
			std::cout << "Build initial solutions by the degrees of the vertices ..." << std::endl;
			std::cout << "\n"; 

			// We create the next 100 solutions in the same way as above but the criteria is now the size of the neighborhood.
			std::priority_queue<deg_node> q_new;
		       	int q_new_size = 0; 	
			for(int i = 0; i < n_kernel; i++) {
				if(algo.global_status.node_status[old_ids[i]] != pack_status::unsafe) {
					deg_node new_node; 
					new_node.v = i; 
					new_node.deg = xadj[i+1] - xadj[i]; 
					q_new.push(new_node);
					q_new_size++; 
				}	
			}
			std::vector<int> one_priority(q_new_size); 
			for(int i = 0; i < q_new_size; i++) {
				int x = (q_new.top()).v;
			       	q_new.pop(); 	
				one_priority[i] = x;
			}

			for(int i = 0; i < 10; i++) {
				int stop = 0; 
				std::vector<int> s_new(n_kernel, 0); 
				for(int j = 0; j < q_new_size; j++) { 
					//if(nodes_to_ignore[one_priority[j]] == false) {
						if(s_new[one_priority[j]] == 0) {
							for(long unsigned int k = 0; k < adj_two_list[one_priority[j]].size(); k++) {
								if(s_new[adj_two_list[one_priority[j]][k]] == 1) {
									stop = 1; 
								}
							}
							if(stop == 0) {
								s_new[one_priority[j]] = 1; 
							} else {
								stop = 0; 
							}
						}
					//}
				}
				local_search(s_new, 45, nodes, distribution(generator), 0, algo); // Local search for 45 seconds for diversity.
				maximize_individual(s_new, distribution(generator), algo, nodes); 
				solutions.push_back(s_new); 
				int sol_size = 0; 
				for(int l = 0; l < n_kernel; l++) {
					if(s_new[l] == 1) {
						sol_size += node_weights[l]; 
					}
				}
				sol_sizes.push_back(sol_size);
			}
			// sort according to the weight -> new initial heuristic. 
			
			std::cout << "Build initial solutions by sorting the weights of the vertices ..." << std::endl; 
			std::cout << "\n"; 

			std::priority_queue<weighted_node> q_weight;
		       	int q_weight_size = 0; 	
			for(int i = 0; i < n_kernel; i++) {
				if(algo.global_status.node_status[old_ids[i]] != pack_status::unsafe) {
					weighted_node new_node; 
					new_node.v = i; 
					new_node.weight = node_weights[i]; 
					q_weight.push(new_node);
					q_weight_size++; 
				}	
			}
			std::vector<int> weight_priority(q_weight_size); 
			for(int i = 0; i < q_weight_size; i++) {
				int x = (q_weight.top()).v;
			       	q_weight.pop(); 	
				weight_priority[i] = x;
			}

			for(int i = 0; i < 10; i++) {
				int stop = 0; 
				std::vector<int> s_new(n_kernel, 0); 
				for(int j = 0; j < q_weight_size; j++) { 
					//if(nodes_to_ignore[one_priority[j]] == false) {
						if(s_new[weight_priority[j]] == 0) {
							for(long unsigned int k = 0; k < adj_two_list[weight_priority[j]].size(); k++) {
								if(s_new[adj_two_list[weight_priority[j]][k]] == 1) {
									stop = 1; 
								}
							}
							if(stop == 0) {
								s_new[weight_priority[j]] = 1; 
							} else {
								stop = 0; 
							}
						}
					//}
				}
				local_search(s_new, 45, nodes, distribution(generator), 0, algo); // Local search for 45 seconds for diversity.
				maximize_individual(s_new, distribution(generator), algo, nodes); 
				solutions.push_back(s_new); 
				int sol_size = 0; 
				for(int l = 0; l < n_kernel; l++) {
					if(s_new[l] == 1) {
						sol_size += node_weights[l]; 
					}
				}
				sol_sizes.push_back(sol_size);
			}

			std::cout << "Building initial solutions DONE." << std::endl; 
			
	// Now we have 250 initial solutions stored in a matrix and a vector with their sizes.  
	// START OF THE LOOP FOR THE EVOLUTIONARY ALGORITHM.
			
			int which_round = 1; // gives us the actual round.
			while(time_now.count() < 21600) { // We want to run the algorithm for 10 hours.

				std::vector<std::vector<int>> off_springs; // initially we produce two offsprings but we only keep the better one. 
				std::vector<int> off_spring_1(n_kernel, 0); 
				std::vector<int> off_spring_2(n_kernel, 0);
				off_springs.push_back(off_spring_1); 
				off_springs.push_back(off_spring_2); 

				// Now we do tournament selection to determine the parents for our offsprings.
				// We pick 4 random individuals from our start population and our parents are the best two of them according to the solution size. 
		
				int min2 = 0; 
				int max2 = solutions.size()-1; 

				std::default_random_engine tournament_generator(distribution(generator)); 
				std::uniform_int_distribution<int> tournament_distribution(min2, max2);
				int tournament_size = 4; // size of the tournament.
	
				std::vector<tournament_cand> tournament_candidates(tournament_size); 

				for(int i = 0; i < tournament_size; i++) {
					tournament_cand candi; 
					candi.candidate = tournament_distribution(tournament_generator); 
					candi.fitness = sol_sizes[candi.candidate];
					tournament_candidates.push_back(candi);  
				}	
	
				std::sort(tournament_candidates.begin(), tournament_candidates.end()); // Sort the random picked individuals by size to determine the winner. 
	
				// Now we combine our selected parents with the combine operation using the node separator to produce our offsprings.
		
				// First offspring.
				for(int i = 0; i < n_kernel; i++) {
					if((partition[i] == 0 && solutions[tournament_candidates[0].candidate][i] == 1) || (partition[i] == 1 && solutions[tournament_candidates[1].candidate][i] == 1)) {
						if(algo.global_status.node_status[old_ids[i]] != pack_status::unsafe) {
							off_springs[0][i] = 1;
						}
					}
				}
				// Second offspring.
				for(int i = 0; i < n_kernel; i++) {
					if((partition[i] == 0 && solutions[tournament_candidates[1].candidate][i] == 1) || (partition[i] == 1 && solutions[tournament_candidates[0].candidate][i] == 1)) {
						if(algo.global_status.node_status[old_ids[i]] != pack_status::unsafe) {
							off_springs[1][i] = 1;
					       	}	
					}
				}
		
				// We compute the sizes of the offsprings to determine the better one. 
				int off_spring_1_size = 0; 
				int off_spring_2_size = 0; 

				for(int i = 0; i < n_kernel; i++) {
					if(off_springs[0][i] == 1) {
						off_spring_1_size += node_weights[i]; 
					}
				}

				for(int i = 0; i < n_kernel; i++) {
					if(off_springs[1][i] == 1) {
						off_spring_2_size += node_weights[i]; 
					}
				}
		
				// We only keep the better one of the created offsprings.
				std::vector<int> offspring(n_kernel); 
				if(off_spring_1_size < off_spring_2_size) {
					offspring = off_springs[1]; 
				} else {
					offspring = off_springs[0]; 
				}




				// Before we do our local search we maximize our solution so the local search is more efficient.
				// We do this by going through the nodes in a random order and put them into the solution if they are free. 
				maximize_individual(offspring, distribution(generator), algo, nodes);   
		

				// Now we perform local search on the offspring (15000 rounds). 
				local_search(offspring, 0, nodes, distribution(generator), 15000, algo);
			      	maximize_individual(offspring, distribution(generator), algo, nodes); 	
 
				// We update the solution size of the offspring.
				int new_off_spring_size = 0; 
				for(int i = 0; i < n; i++) {
					if(offspring[i] == 1) {
						new_off_spring_size += node_weights[i]; 
					}
				}

				// The last step is the eviction. We check which individuals are wors than our offspring. 
				// If there is at least one we delete a random individual of them and insert our offspring instead.

				int threshold = new_off_spring_size; // This is our threshold.

				// Here we will store the candidates which have solution size worse than the offspring.
				std::vector<int> candidates_for_replacement; 
				for(long unsigned int i = 0; i < solutions.size(); i++) {
					if(sol_sizes[i] < threshold) {
						candidates_for_replacement.push_back(i);  
					}
				}
	
				int min3 = 0; 
				int max3 = candidates_for_replacement.size()-1; 

				std::default_random_engine replacement_generator(distribution(generator)); 
				std::uniform_int_distribution<int> replacement_distribution(min3, max3);
	       	
				if(candidates_for_replacement.size() > 1) {
					// We choose the individual that gets replaced randomly.
					int replace_1 = replacement_distribution(replacement_generator); 
 
					// Before we insert the offspring we do some mutation for the diversity of the population to avoid premature convergence.
					mutation_of_individual(offspring, new_off_spring_size, distribution(generator), algo, nodes);   
					maximize_individual(offspring, distribution(generator), algo, nodes); 

					// We update the solution size of the offspring. 
				       	new_off_spring_size = 0;  	
					for(int i = 0; i < n_kernel; i++) {
						if(offspring[i] == 1) {
							new_off_spring_size += node_weights[i]; 
						}
					}

					// Now we replace the randomly chosen candidate by our computed offspring.
					for(int i = 0; i < n_kernel; i++) {
						solutions[candidates_for_replacement[replace_1]][i] = offspring[i];  
					}

					// Update the solution size.
					sol_sizes[candidates_for_replacement[replace_1]] = new_off_spring_size; 
 
				// This is the same as above but if there is only one individual that is worse, we don't have to choose one. 	
				} else if(candidates_for_replacement.size() == 1) {
						mutation_of_individual(offspring, new_off_spring_size, distribution(generator), algo, nodes);
						maximize_individual(offspring, distribution(generator), algo, nodes);
						new_off_spring_size = 0; 
						for(int i = 0; i < n_kernel; i++) {
							if(offspring[i] == 1) {
								new_off_spring_size += node_weights[i]; 
							}
						}
						for(int i = 0; i < n_kernel; i++) {
							solutions[candidates_for_replacement[0]][i] = offspring[i]; 
						}
						sol_sizes[candidates_for_replacement[0]] = new_off_spring_size; 
				}
			
				// We keep track of the best solution in every round to see if there is a development in solution quality. 
				int maximum_solution_size = 0; 
				for(long unsigned int y = 0; y < sol_sizes.size(); y++) {
					if(maximum_solution_size < sol_sizes[y]) {
						maximum_solution_size = sol_sizes[y]; 
					}
				}

				// Update the time that the algorithm needed. 
				stop_time = std::chrono::high_resolution_clock::now(); 
				time_now = std::chrono::duration_cast<std::chrono::seconds>(stop_time-start_point);

			       	// Output the best solution found so far.	
				std::cout << "The maximum solution size of Round " << which_round << "is: " << maximum_solution_size << std::endl;
				std::cout << "Time: " << time_now.count() << std::endl;
				std::cout << std::endl; 
				which_round++;
				
			}	
			
			// Now we want to evaluate this algorithm and return the solution size of the best individual.
			
			int maximum_solution_size = 0;
			int index_of_fittest_individual = 0; 	
			for(long unsigned int y = 0; y < sol_sizes.size(); y++) {
				if(maximum_solution_size < sol_sizes[y]) {
					index_of_fittest_individual = y; 
					maximum_solution_size = sol_sizes[y]; 
				}
			}
			
			// Output the best solution size.
			std::cout << "The solution size of the fittest individual is: " << maximum_solution_size << std::endl; 
			if(is_two_packing_set(solutions[index_of_fittest_individual], algo) == 0) {
				std::cout << "Is two-packing set!" << std::endl; 
			} else {
				std::cout << "Error" << std::endl; 
			}
	
			// Get ending timepoint
			auto stop_point = std::chrono::high_resolution_clock::now(); 
			
       			
			// Get the time the algorithm needed.
			auto duration = std::chrono::duration_cast<std::chrono::seconds>(stop_point - start_point); 	
			std::cout << "Running time: " << duration.count() << " seconds" << std::endl; 
			
			//std::vector<int> empty_test(1,0); 
			return solutions[index_of_fittest_individual];
			//return empty_test;  
		}

		NodeID safe_in_solution(std::vector<int>& fittest_solution) {	
			std::vector<weighted_node> nodes_in_the_solution; 
			for(long unsigned int i = 0; i < fittest_solution.size(); i++) {
				if(fittest_solution[i] == 1) {
					weighted_node vertex; 
					vertex.v = i; 
					vertex.weight = node_weights[i]; 
					nodes_in_the_solution.push_back(vertex); 
				}
			}
			std::sort(nodes_in_the_solution.begin(), nodes_in_the_solution.end());
		       	
			return nodes_in_the_solution[0].v; 	
		}

		
		
		int n, m; // Number of nodes and number of edges G=(V,E), |V|=n, |E|=m.
		int n_kernel, m_kernel;
		int* xadj; 
		int* adjncy;
 		int* vwgt; // weights for the node separator.      	
		
		/***************** FOR THE SEPARATOR *********************/
		int n_org; 
		int m_org; 
		int* xadj_org; 
		int* adjncy_org;
		int* vwgt_org; 
		/********************************************************/
		std::vector<int> help_adjncy; 
		std::vector<std::vector<int>> adj_two_list; // Our modified data structure for the 2-neighborhoods.
		std::vector<std::vector<int>> adj_list; 
		std::vector<NodeWeight> node_weights; 
		std::vector<bool> hidden_nodes; 
		std::vector<bool> nodes_to_ignore; 
		std::vector<NodeID> old_ids; 
		std::vector<NodeID> new_ids; 
		std::vector<bool> unsafe_nodes; 
		std::vector<int> new_degrees; 
}; 


















