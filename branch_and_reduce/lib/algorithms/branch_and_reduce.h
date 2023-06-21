//
// Created by Jannick Borowitz on 21.06.23.
//

#ifndef INC_2_PACKING_SET_BRANCH_AND_REDUCE_H
#define INC_2_PACKING_SET_BRANCH_AND_REDUCE_H

#include "data_structures/m2s_graph_access.h"
#include "graph_access.h"
#include "mis_config.h"
#include "m2s_config.h"

namespace two_packing_set {

class branch_and_reduce {
public:
  /**
   * Constructor for 2-packing-set branch-and-reduce solver
   * @param graph
   * @param m2s_cfg
   * @param mis_cfg
   */
  branch_and_reduce(m2s_graph_access& G, M2SConfig m2s_cfg, MISConfig mis_cfg);

  /**
   * Find maximum 2-packing-set for graph
   * @return solved if true, false for timeout
   */
  bool run();

  NodeID get_solution_size() {
          return solution_size;
  }

private:
  m2s_graph_access graph;
  M2SConfig m2s_cfg;
  MISConfig mis_cfg;

  std::vector<bool> solution_status;
  NodeID solution_size = 0;

  // buffers for kernel
  std::vector<NodeID> reduced_node_id;
  std::vector<NodeID> former_node_id;

  bool timeout = false;

  /**
   * Perform initial reductions
   */
  graph_access perform_initial_reductions();
};
} // namespace two_packing_set

#endif // INC_2_PACKING_SET_BRANCH_AND_REDUCE_H
