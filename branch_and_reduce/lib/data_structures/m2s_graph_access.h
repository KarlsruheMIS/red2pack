/******************************************************************************
 *  m2s_graph_access.h based on graph_access.h
 *
 * Source of KaHIP -- Karlsruhe High Quality Partitioning.
 *
 *****************************************************************************/

#ifndef INC_2_PACKING_SET_M2S_GRAPH_ACCESS_H
#define INC_2_PACKING_SET_M2S_GRAPH_ACCESS_H

#include <bitset>
#include <cassert>
#include <chrono>
#include <iostream>
#include <queue>
#include <vector>

#include "definitions.h"
#include "fast_set.h"
#include "graph_access.h"

namespace two_packing_set {
struct Node {
        EdgeID firstEdge;
        NodeWeight weight;
};

struct Edge {
        NodeID target;
        EdgeWeight weight;
};

struct refinementNode {
        PartitionID partitionIndex;
        // Count queueIndex;
};

struct coarseningEdge {
        EdgeRatingType rating;
};

class m2s_graph_access;

// construction etc. is encapsulated in basicGraph / access to properties etc. is encapsulated in  m2s_graph_access
class basicGraph {
        friend class m2s_graph_access;

       public:
        basicGraph() : m_building_graph(false) {}

       private:
        // methods only to be used by friend class
        EdgeID number_of_edges() { return m_edges.size(); }

        NodeID number_of_nodes() { return m_nodes.size() - 1; }

        inline EdgeID get_first_edge(const NodeID& node) { return m_nodes[node].firstEdge; }

        inline EdgeID get_first_invalid_edge(const NodeID& node) { return m_nodes[node + 1].firstEdge; }

        // construction of the graph
        void start_construction(NodeID n, EdgeID m) {
                m_building_graph = true;
                node = 0;
                e = 0;
                m_last_source = -1;

                // resizes property arrays
                m_nodes.resize(n + 1);
                m_refinement_node_props.resize(n + 1);
                m_edges.resize(m);
                m_coarsening_edge_props.resize(m);

                m_nodes[node].firstEdge = e;
        }

        EdgeID new_edge(NodeID source, NodeID target) {
                ASSERT_TRUE(m_building_graph);
                ASSERT_TRUE(e < m_edges.size());

                m_edges[e].target = target;
                EdgeID e_bar = e;
                ++e;

                ASSERT_TRUE(source + 1 < m_nodes.size());
                m_nodes[source + 1].firstEdge = e;

                // fill isolated sources at the end
                if ((NodeID)(m_last_source + 1) < source) {
                        for (NodeID i = source; i > (NodeID)(m_last_source + 1); i--) {
                                m_nodes[i].firstEdge = m_nodes[m_last_source + 1].firstEdge;
                        }
                }
                m_last_source = source;
                return e_bar;
        }

        NodeID new_node() {
                ASSERT_TRUE(m_building_graph);
                return node++;
        }

        void finish_construction() {
                // inert dummy node
                m_nodes.resize(node + 1);
                m_refinement_node_props.resize(node + 1);

                m_edges.resize(e);
                m_coarsening_edge_props.resize(e);

                m_building_graph = false;

                // fill isolated sources at the end
                if ((unsigned int)(m_last_source) != node - 1) {
                        // in that case at least the last node was an isolated node
                        for (NodeID i = node; i > (unsigned int)(m_last_source + 1); i--) {
                                m_nodes[i].firstEdge = m_nodes[m_last_source + 1].firstEdge;
                        }
                }
        }

        // %%%%%%%%%%%%%%%%%%% DATA %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        // split properties for coarsening and uncoarsening
        std::vector<Node> m_nodes;
        std::vector<Edge> m_edges;

        // this is the equivalent of xadj/adjncy for the 2-neighborhood.
        std::vector<Node> m2_nodes;
        std::vector<Edge> m2_edges;

        std::vector<refinementNode> m_refinement_node_props;
        std::vector<coarseningEdge> m_coarsening_edge_props;

        // construction properties
        bool m_building_graph;
        int m_last_source;
        NodeID node;  // current node that is constructed
        EdgeID e;     // current edge that is constructed
};

#define forall_out_edges2(G, e, n) \
        {                          \
                for (EdgeID e = G.get_first_edge2(n), end = G.get_first_invalid_edge2(n); e < end; ++e) {
class complete_boundary;

class m2s_graph_access {
        friend class complete_boundary;

       public:
        m2s_graph_access() {
                m_max_degree_computed = false;
                m_max_degree = 0;
                graphref = new basicGraph();
        }
        virtual ~m2s_graph_access() { delete graphref; };

        /* ============================================================= */
        /* build methods */
        /* ============================================================= */
        void start_construction(NodeID nodes, EdgeID edges);
        NodeID new_node();
        EdgeID new_edge(NodeID source, NodeID target);
        void finish_construction();

        /* ============================================================= */
        /* graph access methods */
        /* ============================================================= */
        NodeID number_of_nodes();
        EdgeID number_of_edges();

        void init_touched(int n);

        EdgeID get_first_edge(NodeID node);
        EdgeID get_first_invalid_edge(NodeID node);
        // =========================================================================
        // We need the equivalent of the above two functions for the 2-neighborhood.
        EdgeID get_first_edge2(NodeID node);
        EdgeID get_first_invalid_edge2(NodeID node);
        // =========================================================================

        PartitionID get_partition_count();
        void set_partition_count(PartitionID count);

        PartitionID getPartitionIndex(NodeID node);
        void setPartitionIndex(NodeID node, PartitionID id);

        PartitionID getSecondPartitionIndex(NodeID node);
        void setSecondPartitionIndex(NodeID node, PartitionID id);

        // to be called if combine in meta heuristic is used
        void resizeSecondPartitionIndex(unsigned no_nodes);

        NodeWeight getNodeWeight(NodeID node);
        void setNodeWeight(NodeID node, NodeWeight weight);

        EdgeWeight getNodeDegree(NodeID node);

        // ==================================================================================
        // here we add this function. We want to use it for the dynamic graph data structure.
        EdgeWeight get2Degree(NodeID node);
        // ==================================================================================

        EdgeWeight getWeightedNodeDegree(NodeID node);
        EdgeWeight getMaxDegree();

        EdgeWeight getEdgeWeight(EdgeID edge);
        void setEdgeWeight(EdgeID edge, EdgeWeight weight);

        NodeID getEdgeTarget(EdgeID edge);

        // =================================================================================
        // we need the equivalent of getEdgeTarget for our macro equivalent
        NodeID getEdgeTarget2(EdgeID edge);
        // =================================================================================

        EdgeRatingType getEdgeRating(EdgeID edge);
        void setEdgeRating(EdgeID edge, EdgeRatingType rating);

        int* UNSAFE_metis_style_xadj_array();
        int* UNSAFE_metis_style_adjncy_array();

        int* UNSAFE_metis_style_vwgt_array();
        int* UNSAFE_metis_style_adjwgt_array();

        int build_from_metis(int n, int* xadj, int* adjncy);
        int build_from_metis_weighted(int n, int* xadj, int* adjncy, int* vwgt, int* adjwgt);

        void construct_access(long unsigned int n, long unsigned int m, int* xadj, int* adjncy);
        void construct_2neighborhood();

        // void set_node_queue_index(NodeID node, Count queue_index);
        // Count get_node_queue_index(NodeID node);

        void copy(m2s_graph_access& Gcopy);
        // std::vector<Edge> get2neighbors(NodeID node);
        // void construct_2neigh();
        // std::vector<std::vector<int>> two_neighbors;

       private:
        basicGraph* graphref;
        bool m_max_degree_computed;
        unsigned int m_partition_count;
        EdgeWeight m_max_degree;
        std::vector<PartitionID> m_second_partition_index;
};

/* graph build methods */
inline void m2s_graph_access::start_construction(NodeID nodes, EdgeID edges) {
        graphref->start_construction(nodes, edges);
}

inline NodeID m2s_graph_access::new_node() { return graphref->new_node(); }

inline EdgeID m2s_graph_access::new_edge(NodeID source, NodeID target) { return graphref->new_edge(source, target); }

inline void m2s_graph_access::finish_construction() { graphref->finish_construction(); }

/* graph access methods */
inline NodeID m2s_graph_access::number_of_nodes() { return graphref->number_of_nodes(); }

inline EdgeID m2s_graph_access::number_of_edges() { return graphref->number_of_edges(); }

inline void m2s_graph_access::resizeSecondPartitionIndex(unsigned no_nodes) {
        m_second_partition_index.resize(no_nodes);
}

inline EdgeID m2s_graph_access::get_first_edge(NodeID node) {
#ifdef NDEBUG
        return graphref->m_nodes[node].firstEdge;
#else
        return graphref->m_nodes.at(node).firstEdge;
#endif
}

// ==============================================================
// get_first_edge(NodeID node) equivalent for the 2-neighborhood.
inline EdgeID m2s_graph_access::get_first_edge2(NodeID node) {
#ifdef NDEBUG
        return graphref->m2_nodes[node].firstEdge;
#else
        return graphref->m2_nodes.at(node).firstEdge;
#endif
}
// ===============================================================

inline EdgeID m2s_graph_access::get_first_invalid_edge(NodeID node) { return graphref->m_nodes[node + 1].firstEdge; }

// ======================================================================
// get_first_invalid_edge(NodeID node) equivalent for the 2-neighborhood.
inline EdgeID m2s_graph_access::get_first_invalid_edge2(NodeID node) { return graphref->m2_nodes[node + 1].firstEdge; }
// ======================================================================

inline PartitionID m2s_graph_access::get_partition_count() { return m_partition_count; }

inline PartitionID m2s_graph_access::getSecondPartitionIndex(NodeID node) {
#ifdef NDEBUG
        return m_second_partition_index[node];
#else
        return m_second_partition_index.at(node);
#endif
}

inline void m2s_graph_access::setSecondPartitionIndex(NodeID node, PartitionID id) {
#ifdef NDEBUG
        m_second_partition_index[node] = id;
#else
        m_second_partition_index.at(node) = id;
#endif
}

inline PartitionID m2s_graph_access::getPartitionIndex(NodeID node) {
#ifdef NDEBUG
        return graphref->m_refinement_node_props[node].partitionIndex;
#else
        return graphref->m_refinement_node_props.at(node).partitionIndex;
#endif
}

inline void m2s_graph_access::setPartitionIndex(NodeID node, PartitionID id) {
#ifdef NDEBUG
        graphref->m_refinement_node_props[node].partitionIndex = id;
#else
        graphref->m_refinement_node_props.at(node).partitionIndex = id;
#endif
}

inline NodeWeight m2s_graph_access::getNodeWeight(NodeID node) {
#ifdef NDEBUG
        return graphref->m_nodes[node].weight;
#else
        return graphref->m_nodes.at(node).weight;
#endif
}

inline void m2s_graph_access::setNodeWeight(NodeID node, NodeWeight weight) {
#ifdef NDEBUG
        graphref->m_nodes[node].weight = weight;
#else
        graphref->m_nodes.at(node).weight = weight;
#endif
}

inline EdgeWeight m2s_graph_access::getEdgeWeight(EdgeID edge) {
#ifdef NDEBUG
        return graphref->m_edges[edge].weight;
#else
        return graphref->m_edges.at(edge).weight;
#endif
}

inline void m2s_graph_access::setEdgeWeight(EdgeID edge, EdgeWeight weight) {
#ifdef NDEBUG
        graphref->m_edges[edge].weight = weight;
#else
        graphref->m_edges.at(edge).weight = weight;
#endif
}

inline NodeID m2s_graph_access::getEdgeTarget(EdgeID edge) {
#ifdef NDEBUG
        return graphref->m_edges[edge].target;
#else
        return graphref->m_edges.at(edge).target;
#endif
}

// ===============================================================
inline NodeID m2s_graph_access::getEdgeTarget2(EdgeID edge) {
#ifdef NDEBUG
        return graphref->m2_edges[edge].target;
#else
        return graphref->m2_edges.at(edge).target;
#endif
}
// ===============================================================

inline EdgeRatingType m2s_graph_access::getEdgeRating(EdgeID edge) {
#ifdef NDEBUG
        return graphref->m_coarsening_edge_props[edge].rating;
#else
        return graphref->m_coarsening_edge_props.at(edge).rating;
#endif
}

inline void m2s_graph_access::setEdgeRating(EdgeID edge, EdgeRatingType rating) {
#ifdef NDEBUG
        graphref->m_coarsening_edge_props[edge].rating = rating;
#else
        graphref->m_coarsening_edge_props.at(edge).rating = rating;
#endif
}

inline EdgeWeight m2s_graph_access::getNodeDegree(NodeID node) {
        return graphref->m_nodes[node + 1].firstEdge - graphref->m_nodes[node].firstEdge;
}

// =======================================================================================
// We want to get the degree of the degree 2 neighbors.
inline EdgeWeight m2s_graph_access::get2Degree(NodeID node) {
        return graphref->m2_nodes[node + 1].firstEdge - graphref->m2_nodes[node].firstEdge;
}
// =======================================================================================

inline EdgeWeight m2s_graph_access::getWeightedNodeDegree(NodeID node) {
        EdgeWeight degree = 0;
        for (unsigned e = graphref->m_nodes[node].firstEdge; e < graphref->m_nodes[node + 1].firstEdge; ++e) {
                degree += getEdgeWeight(e);
        }
        return degree;
}

inline EdgeWeight m2s_graph_access::getMaxDegree() {
        if (!m_max_degree_computed) {
                // compute it
                basicGraph& ref = *graphref;
                forall_nodes (ref, node) {
                        EdgeWeight cur_degree = 0;
                        forall_out_edges (ref, e, node) {
                                cur_degree += getEdgeWeight(e);
                        }
                        endfor
                        if (cur_degree > m_max_degree) {
                                m_max_degree = cur_degree;
                        }
                }
                endfor
                m_max_degree_computed = true;
        }

        return m_max_degree;
}

inline int* m2s_graph_access::UNSAFE_metis_style_xadj_array() {
        int* xadj = new int[graphref->number_of_nodes() + 1];
        basicGraph& ref = *graphref;

        forall_nodes (ref, n) {
                xadj[n] = graphref->m_nodes[n].firstEdge;
        }
        endfor
        xadj[graphref->number_of_nodes()] = graphref->m_nodes[graphref->number_of_nodes()].firstEdge;
        return xadj;
}

inline int* m2s_graph_access::UNSAFE_metis_style_adjncy_array() {
        int* adjncy = new int[graphref->number_of_edges()];
        basicGraph& ref = *graphref;
        forall_edges (ref, e) {
                adjncy[e] = graphref->m_edges[e].target;
        }
        endfor

        return adjncy;
}

inline int* m2s_graph_access::UNSAFE_metis_style_vwgt_array() {
        int* vwgt = new int[graphref->number_of_nodes()];
        basicGraph& ref = *graphref;

        forall_nodes (ref, n) {
                vwgt[n] = (int)graphref->m_nodes[n].weight;
        }
        endfor
        return vwgt;
}

inline int* m2s_graph_access::UNSAFE_metis_style_adjwgt_array() {
        int* adjwgt = new int[graphref->number_of_edges()];
        basicGraph& ref = *graphref;

        forall_edges (ref, e) {
                adjwgt[e] = (int)graphref->m_edges[e].weight;
        }
        endfor

        return adjwgt;
}

inline void m2s_graph_access::set_partition_count(PartitionID count) { m_partition_count = count; }

inline int m2s_graph_access::build_from_metis(int n, int* xadj, int* adjncy) {
        graphref = new basicGraph();
        start_construction(n, xadj[n]);

        for (unsigned i = 0; i < (unsigned)n; i++) {
                NodeID node = new_node();
                setNodeWeight(node, 1);
                setPartitionIndex(node, 0);

                for (unsigned e = xadj[i]; e < (unsigned)xadj[i + 1]; e++) {
                        EdgeID e_bar = new_edge(node, adjncy[e]);
                        setEdgeWeight(e_bar, 1);
                }
        }

        finish_construction();
        return 0;
}

inline int m2s_graph_access::build_from_metis_weighted(int n, int* xadj, int* adjncy, int* vwgt, int* adjwgt) {
        graphref = new basicGraph();
        start_construction(n, xadj[n]);

        for (unsigned i = 0; i < (unsigned)n; i++) {
                NodeID node = new_node();
                setNodeWeight(node, vwgt[i]);
                setPartitionIndex(node, 0);

                for (unsigned e = xadj[i]; e < (unsigned)xadj[i + 1]; e++) {
                        EdgeID e_bar = new_edge(node, adjncy[e]);
                        setEdgeWeight(e_bar, adjwgt[e]);
                }
        }

        finish_construction();
        return 0;
}

inline void m2s_graph_access::copy(m2s_graph_access& G_bar) {
        G_bar.start_construction(number_of_nodes(), number_of_edges());

        basicGraph& ref = *graphref;
        forall_nodes (ref, node) {
                NodeID shadow_node = G_bar.new_node();
                G_bar.setNodeWeight(shadow_node, getNodeWeight(node));
                forall_out_edges (ref, e, node) {
                        NodeID target = getEdgeTarget(e);
                        EdgeID shadow_edge = G_bar.new_edge(shadow_node, target);
                        G_bar.setEdgeWeight(shadow_edge, getEdgeWeight(e));
                }
                endfor
        }
        endfor

        G_bar.finish_construction();
}

// ==============================================================================
// Get the 2-neighbors that have distance two.
/*inline std::vector<Edge> m2s_graph_access::get2neighbors(NodeID node) {
        std::vector<Edge> deg2_nodes;
        std::queue<NodeID> bfsqueue;
        NodeID nodes_left = graphref->number_of_nodes();

        // clear touched:
        for (int i = 0; i < graphref->number_of_nodes(); i++) {
                touched[i] = false;
        }

        touched[node] = true;
        bfsqueue.push(node);
        basicGraph& ref = *graphref;

        if (!bfsqueue.empty()) {
                NodeID source = bfsqueue.front();
                bfsqueue.pop();
                nodes_left--;

                forall_out_edges (ref, e, source) {
                        NodeID target = getEdgeTarget(e);
                        if (!touched[target]) {
                                touched[target] = true;
                                bfsqueue.push(target);
                        }
                }
                endfor
        }

        while (!bfsqueue.empty()) {
                NodeID now = bfsqueue.front();
                bfsqueue.pop();

                forall_out_edges (ref, e, now) {
                        NodeID target2 = getEdgeTarget(e);
                        if (!touched[target2]) {
                                touched[target2] = true;
                                Edge vertex;
                                vertex.target = target2;
                                deg2_nodes.push_back(vertex);
                        }
                }
                endfor
        }

        return deg2_nodes;
}*/
// ====================================================================================

// ====================================================================================
// We want to build the neighbors of degree two for every node.
inline void m2s_graph_access::construct_2neighborhood() {
        auto start_t = std::chrono::system_clock::now();
        EdgeID count_2_edges = 0;
        fast_set touched(number_of_nodes());
        graphref->m2_nodes.resize(graphref->number_of_nodes() + 1);
        graphref->m2_edges.reserve(graphref->number_of_edges()*4); // guess for number of edges
        for (NodeID v_idx = 0; v_idx < number_of_nodes(); v_idx++) {
                graphref->m2_nodes[v_idx].firstEdge = count_2_edges;
                touched.add(v_idx);

                for (int j = graphref->m_nodes[v_idx].firstEdge; j < graphref->m_nodes[v_idx + 1].firstEdge; j++) {
                        touched.add(graphref->m_edges[j].target);
                }

                for (int k = graphref->m_nodes[v_idx].firstEdge; k < graphref->m_nodes[v_idx + 1].firstEdge; k++) {
                        auto source = graphref->m_edges[k].target;
                        for (EdgeID j = graphref->m_nodes[source].firstEdge;
                             j < graphref->m_nodes[source + 1].firstEdge; j++) {
                                NodeID target = graphref->m_edges[j].target;
                                if (!touched.get(target)) {
                                        graphref->m2_edges.push_back({target, 1});
                                        touched.add(target);
                                        count_2_edges++;
                                }
                        }
                }
                touched.clear();
        }
        graphref->m2_nodes[number_of_nodes()].firstEdge = count_2_edges;
        auto stop_t = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_time_construction = stop_t - start_t;
        std::cout << "time 2-neighborhood: " << elapsed_time_construction.count() << std::endl;
}
// ======================================================================================

inline void m2s_graph_access::construct_access(long unsigned int n, long unsigned int m, int* xadj, int* adjncy) {
        graphref = new basicGraph();
        graphref->m_nodes.resize(n + 1);
        graphref->m_edges.resize(m);

        for (int i = 0; i < n + 1; i++) {
                Node new_node;
                new_node.firstEdge = xadj[i];
                graphref->m_nodes[i] = new_node;
        }
        for (int j = 0; j < m; j++) {
                Edge new_edge;
                new_edge.target = adjncy[j];
                if (adjncy[j] > 101996) {
                        std::cout << "error" << std::endl;
                }
                graphref->m_edges[j] = new_edge;
        }
        /*
        for(int i = 0; i < n; i++) {
                for(int j = graphref->m_nodes[i].firstEdge; j < graphref->m_nodes[i+1].firstEdge; j++) {
                        std::cout << graphref->m_edges[j].target+1 << " ";
                }
                std::cout << "\n";
        }
        */
}
}

#endif  // INC_2_PACKING_SET_M2S_GRAPH_ACCESS_H
