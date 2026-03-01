#ifndef RED2PACK_HTWIS_UTILS_H
#define RED2PACK_HTWIS_UTILS_H


#include <htwis/Graph.h>

#include <red2pack/tools/scoped_timer.h>

namespace red2pack {

inline void convert(m2s_graph_access& input_graph, htwis::Graph& htwis_graph) {
        using htwis::Graph;
        using htwis::ui;
        using htwis::Vertex;
        RED2PACK_SCOPED_TIMER("Convert m2s_graph_access to htwis::Graph");

        htwis_graph.n = static_cast<int>(input_graph.number_of_nodes());
        htwis_graph.m = static_cast<int>(input_graph.number_of_edges());

        htwis_graph.head = new Vertex[htwis_graph.n];
        Vertex* head = htwis_graph.head;
        for (int i = 0; i < htwis_graph.n; ++i) {
                Vertex a;
                head[i] = a;
        }

        // set weight
        for (ui i = 0; i < htwis_graph.n; ++i) {
                head[i].weight = input_graph.getNodeWeight(i);
        }
        // set adj. arrays
        for (ui i = 0; i < htwis_graph.n; ++i) {
                head[i].neiSize = input_graph.getNodeDegree(i) + input_graph.getLinkDegree(i);
                head[i].adjacent.reserve(head[i].neiSize);
                // direct edges
                forall_out_edges (input_graph, e, i) {
                        head[i].adjacent.push_back(input_graph.getEdgeTarget(e));
                }
                endfor
                // two-edges
                forall_out_links(input_graph, e2, i) { head[i].adjacent.push_back(input_graph.getLinkTarget(e2)); }
                endfor
        }
}

inline void perform_htwis(htwis::Graph& g) {
        RED2PACK_SCOPED_TIMER("HtWIS");
        using htwis::Graph;
        // run HtWIS
        g.htThreeAll();
        // g->htCommon();
        // g->htDegreeOne();
        // g->htLowDeg();
        // g->htDegreeTwo();
        // g->htNeighborhood();
        // g->bfs();
        // g->htPvertex();
        // g->dtThreeAll();
        // g->wtThreeAll();

        std::cout << "htwis sol: " << g.total_weight << std::endl;
}
}  // namespace red2pack

#endif