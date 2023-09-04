#ifndef INC_2_PACKING_SET_BRANCH_AND_REDUCE_H
#define INC_2_PACKING_SET_BRANCH_AND_REDUCE_H

#include "data_structure/m2s_dynamic_graph.h"
#include "data_structure/m2s_graph_access.h"
#include "graph_access.h"
#include "m2s_config.h"
#include "reduce_algorithm.h"

namespace red2pack {

class reduce_and_transform {
public:
  /**
   * Constructor for 2-packing-set reduce-and-transform algorithm
   * @param graph
   * @param m2s_cfg
   */
 reduce_and_transform(m2s_graph_access& G, M2SConfig m2s_cfg);

  /**
   * Find maximum 2-packing-set for the input graph
   * @return solved if true, false if a non-empty condensed graph as mis problem left
   */
  bool run_reduce_and_transform(graph_access& condensed_graph);

  NodeID get_solution_offset_size() {
          return solution_offset_size;
  }

  const std::vector<bool>& get_solution() {
          return solution_status;
  }

protected:
  m2s_graph_access graph;
  M2SConfig m2s_cfg;

  // indicate solution vertices of the 2-packing-set for the input graph
  std::vector<bool> solution_status;

  // buffers for kernel
  std::vector<NodeID> reduced_node_id;
  std::vector<NodeID> former_node_id;

  // HELPERS to build the condensed graphs
  // you can overwrite them if your MIS solver requires special properties for the condensed graphs
  /**
   * Build a condensed graph given a m2s_graph_access instance (input graph).
   * @param condensed_graph
   * @param graph
   * @param nodes of condensed_graph
   * @param edges of condensed_graph
   */
  virtual void build_condensed_graph_from_access(graph_access& condensed_graph, m2s_graph_access& graph, NodeID nodes, EdgeID edges);

  /**
   * Build a condensed graph given a m2s_dynamic_graph instance and a node status with respect to the found 2-packing-set so far.
   * @param condensed_graph
   * @param reduced_graph
   * @param reduced_node_status
   * @param nodes
   * @param edges
   */
  virtual void build_condensed_graph_from_status(graph_access& condensed_graph, m2s_dynamic_graph& reduced_graph, std::vector<reduce_algorithm::two_pack_status>& reduced_node_status, NodeID nodes, EdgeID edges);

 private:

  NodeID solution_offset_size = 0;

  /**
   * Trasform the input graph to a condensed graph using red2pack reductions.
   * @param condensed_graph
   */
  void transform_with_reductions(graph_access& condensed_graph);

  /**
   * Transform the input graph to a condensed graph without using red2pack reduction.
   * Note that the condensed graph ist the square graph of the input graph.
   * @param condensed_graph
   */
  void transform_without_reductions(graph_access& condensed_graph);
};
} // namespace two_packing_set

#endif // INC_2_PACKING_SET_BRANCH_AND_REDUCE_H
