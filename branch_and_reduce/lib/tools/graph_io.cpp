//
// Created by Jannick Borowitz on 21.06.23.
//
#include "graph_io.h"
#include <fstream>
#include <iostream>
#include <limits>
#include <ostream>
#include <sstream>

namespace two_packing_set {
int graph_io::readGraphWeighted(m2s_graph_access &G, std::string filename) {
  std::string line;

  // open file for reading
  std::ifstream in(filename.c_str());
  if (!in) {
    std::cerr << "Error opening " << filename << std::endl;
    return 1;
  }

  long nmbNodes;
  long nmbEdges;

  std::getline(in, line);
  // skip comments
  while (line[0] == '%') {
    std::getline(in, line);
  }

  int ew = 0;
  std::stringstream ss(line);
  ss >> nmbNodes;
  ss >> nmbEdges;
  ss >> ew;

#ifdef UNSAFE_LONG
  if (2 * nmbEdges > std::numeric_limits<long>::max() ||
      nmbNodes > std::numeric_limits<long>::max()) {
    std::cerr << "The graph is too large. Currently only 64bit supported!"
              << std::endl;
    exit(0);
  }
#else
  if (2 * nmbEdges > std::numeric_limits<int>::max() ||
      nmbNodes > std::numeric_limits<int>::max()) {
    std::cerr << "The graph is too large. Currently only 32bit supported!"
              << std::endl;
    exit(0);
  }
#endif // UNSAFE_LONG

  bool read_ew = false;
  bool read_nw = false;

  if (ew == 1) {
    read_ew = true;
  } else if (ew == 11) {
    read_ew = true;
    read_nw = true;
  } else if (ew == 10) {
    read_nw = true;
  }
  nmbEdges *= 2; // since we have forward and backward edges

  NodeID node_counter = 0;
  EdgeID edge_counter = 0;
  long total_nodeweight = 0;

  G.start_construction(nmbNodes, nmbEdges);

  while (std::getline(in, line)) {

    if (line[0] == '%') { // a comment in the file
      continue;
    }

    NodeID node = G.new_node();
    node_counter++;
    G.setPartitionIndex(node, 0);

    std::stringstream ss(line);

    NodeWeight weight = 1;
    if (read_nw) {
      ss >> weight;
      total_nodeweight += weight;
      if (total_nodeweight > (long)std::numeric_limits<NodeWeight>::max()) {
        std::cerr << "The sum of the node weights is too large (it exceeds the "
                     "node weight type)."
                  << std::endl;
        std::cerr << "Currently not supported. Please scale your node weights."
                  << std::endl;
        exit(0);
      }
    }
    G.setNodeWeight(node, weight);

    NodeID target;
    NodeID prev_target = 0;
    while (ss >> target) {
      // check sortedness
      if (target < prev_target) {
        std::cerr
            << "The graph file contains unsorted edges. This is not supported. "
               "Please use the following command to sort the edges:"
            << std::endl;
        std::cerr << "python misc/conversion/sort_metis.py " << filename
                  << std::endl;
        exit(0);
      }

      // check for self-loops
      if (target - 1 == node) {
        std::cerr << "The graph file contains self-loops. This is not "
                     "supported. Please remove them from the file."
                  << std::endl;
      }

      EdgeWeight edge_weight = 1;
      if (read_ew) {
        ss >> edge_weight;
      }
      edge_counter++;
      EdgeID e = G.new_edge(node, target - 1);

      G.setEdgeWeight(e, edge_weight);
      prev_target = target;
    }

    if (in.eof()) {
      break;
    }
  }

  if (edge_counter != (EdgeID)nmbEdges) {
    std::cerr << "number of specified edges mismatch" << std::endl;
    std::cerr << edge_counter << " " << nmbEdges << std::endl;
    exit(0);
  }

  if (node_counter != (NodeID)nmbNodes) {
    std::cerr << "number of specified nodes mismatch" << std::endl;
    std::cerr << node_counter << " " << nmbNodes << std::endl;
    exit(0);
  }

  G.finish_construction();
  return 0;
}
} // namespace two_packing_set