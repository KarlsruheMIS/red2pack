#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>
#include <vector>
#include <algorithm>
#include <unordered_set>
#include <queue>
#include <chrono>

class graph {
	public:
		bool init_graph(const std::string& file) {

			std::string line; 
			std::string filename(file); 

			// open file for reading
			std::ifstream in(filename.c_str()); 
			if (!in) {
				std::cerr << "Error opening " << filename << std::endl; 
			}

			std::getline(in,line); 
			// skip comments
			while(line[0] == '%') {
				std::getline(in,line); 
			}
	
			int ew = 0; 
			long nmbNodes; 
			long nmbEdges; 
			std::stringstream ss(line); 
			ss >> nmbNodes; 
			ss >> nmbEdges; 
			ss >> ew; 
 
			bool read_ew = false; 
			bool read_nw = false; 

			if(ew == 1) {
				read_ew = true; 
			} else if(ew == 11) {
				read_ew = true; 
				read_nw = true; 
			} else if(ew == 10) {
				read_nw = true; 
			}

			n = nmbNodes; 
			m = nmbEdges; 
			xadj = new int[n+1]; 
			adjncy = new int[2*m];
			node_weights.resize(n); 

			int vertex = 0; 
			int counter = 0; 
			if(read_nw == true) {
				while(std::getline(in, line)) {
					if(line[0] == '%') {
						continue; 
					}
					xadj[vertex] = counter; 
					std::stringstream ss(line); 
					int node; 
					int weight; 
					ss >> weight; 
					node_weights[vertex] = weight; 
					while(ss >> node) {
						adjncy[counter] = node -1; 
						counter++; 
					}

					vertex++; 

					if(vertex == n) {
						xadj[n] = counter; 
					}

					if(in.eof()) {
						break; 
					}
				}
			} else {
				while(std::getline(in, line)) {
					if(line[0] == '%') {
						continue; 
					}
					xadj[vertex] = counter; 
					std::stringstream ss(line); 
					int node; 
					int weight;  
					node_weights[vertex] = 1; 
					while(ss >> node) {
						adjncy[counter] = node -1; 
						counter++; 
					}

					vertex++; 

					if(vertex == n) {
						xadj[n] = counter; 
					}

					if(in.eof()) {
						break; 
					}
				}		
			}
			return read_nw; 
		}

		// now we compute the graph which has an additional edge whenever two nodes have distance two.
		void condense_and_sort_graph(bool read_nw) {
			adj_two_list.resize(n);
		       	number_of_directed_edges = 0; 	

			for(int v_idx = 0; v_idx < n; v_idx++) {
				if(read_nw) {
					adj_two_list[v_idx].push_back(node_weights[v_idx]); 
				}
				std::vector<bool> touched(n, false); 
				std::queue<int> bfsqueue; 
				int nodes_left = n; 
				int startNode = v_idx; 

				touched[startNode] = true; 
				
				nodes_left--; 

				for(int j = xadj[v_idx]; j < xadj[v_idx+1]; j++) {
					int target = adjncy[j]; 
					if(!touched[target]) {
						adj_two_list[v_idx].push_back(target); 
						touched[target] = true; 
						bfsqueue.push(target);
					       	number_of_directed_edges++; 	
					}
				}

				while(!bfsqueue.empty()) {
					int source = bfsqueue.front(); 
					bfsqueue.pop(); 
					nodes_left--; 

					for(int j = xadj[source]; j < xadj[source+1]; j++) {
						int target = adjncy[j]; 
						if(!touched[target]) {
							adj_two_list[v_idx].push_back(target); 
							touched[target] = true; 
							number_of_directed_edges++; 
						}
					}
				}
			       	std::sort(adj_two_list[v_idx].begin(), adj_two_list[v_idx].end()); 	
			}
		}

		void print_dense_graph(bool weighted) {
			if(weighted) {
				std::cout << n << " " << number_of_directed_edges/2 << " " << "10" << std::endl;	
			} else {
				std::cout << n << " " << number_of_directed_edges/2 << std::endl; 
			}	
			for(int v = 0; v < n; v++) {
				for(long unsigned int j = 0; j < adj_two_list[v].size(); j++) {
					if(j == 0 && weighted == 1) {
						std::cout << adj_two_list[v][j]; 
					} else {
						std::cout << adj_two_list[v][j]+1; 
					}
					if(j != adj_two_list[v].size()-1) {
						std::cout << " "; 
					}
				}
				std::cout << "\n"; 
			}
		}
		
		long n, m; 
	private:  
		int* xadj; 
		int* adjncy; 
		std::vector<long> node_weights; 
		std::vector<std::vector<int>> adj_two_list; 
		long number_of_directed_edges; 
}; 

int main(int argn, char **argv) {
	if(argn != 2) {
		std::cout << "Usage: sort_adjacencies FILE" << std::endl; 
		exit(0); 
	}
	graph G; 
	bool read_weighted; 
	read_weighted = G.init_graph(argv[1]);
	auto start_point = std::chrono::high_resolution_clock::now(); 
       	G.condense_and_sort_graph(read_weighted);
	auto stop_point = std::chrono::high_resolution_clock::now(); 
	auto time_now = std::chrono::duration_cast<std::chrono::seconds>(stop_point-start_point); 
	std::cout << "%Time to condense the graph: " << time_now.count() << " seconds" << std::endl; 
	G.print_dense_graph(read_weighted); 

}
