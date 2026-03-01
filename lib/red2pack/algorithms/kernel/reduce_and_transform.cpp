#include "red2pack/algorithms/kernel/reduce_and_transform.h"

#include "red2pack/algorithms/kernel/reduce_algorithm.h"
#include "red2pack/tools/m2s_graph_io.h"
#include "red2pack/tools/m2s_log.h"
#include "red2pack/tools/scoped_timer.h"

#include <chrono>
#include <utility>



namespace red2pack {

void reduce_and_transform::attach(std::unique_ptr<m2s_graph_access> G, M2SConfig m2s_cfg) {
        this->m2s_cfg = std::move(m2s_cfg);
        solution_status.resize(G->number_of_nodes());
        reduced_node_id.resize(G->number_of_nodes(), G->number_of_nodes());
        former_node_id.resize(G->number_of_nodes());
        for (NodeID node = 0; node < G->number_of_nodes(); node++) {
                solution_status[node] = false;
                reduced_node_id[node] = G->number_of_nodes();
                former_node_id[node] = 0;
        }
        graph = std::move(G);
}

std::unique_ptr<m2s_graph_access> &reduce_and_transform::detach() { return graph; }

void reduce_and_transform::init_reducer() { reducer = std::move(std::make_unique<reduce_algorithm>(*graph, m2s_cfg)); }

bool reduce_and_transform::use_reducer() {
        return !(m2s_cfg.disable_fast_domination && m2s_cfg.disable_domination && m2s_cfg.disable_deg_two &&
               m2s_cfg.disable_deg_one && m2s_cfg.disable_twin && m2s_cfg.disable_clique);
}
void reduce_and_transform::transform_with_reductions() {
        // construct 2neighborhood
        if (!m2s_cfg.on_demand_two_neighborhood) {
                RED2PACK_SCOPED_TIMER("Construct 2-neighborhood");
                graph->construct_2neighborhood();
        }

        // run first reductions
        {
                RED2PACK_SCOPED_TIMER("Reductions");
                init_reducer();
                reducer->run_reductions();
        }

        // compute new graph access
        reducer->get_exact_kernel(reduced_graph, reduced_node_id, former_node_id);

        // set solution offset weight
        solution_offset_weight = reducer->get_solution_weight();

        m2s_log::instance()->print_reduction(solution_offset_weight, reduced_graph.number_of_nodes(),
                                             reduced_graph.number_of_edges() / 2,
                                             reduced_graph.number_of_links() / 2);

        if(m2s_cfg.write_transformed) {
                m2s_graph_io::writeGraphWeighted(reduced_graph, m2s_cfg.transformed_graph_filename, " reduced-and-transformed graph of " + m2s_cfg.graph_filename +
                                                     ", offset: " + std::to_string(solution_offset_weight));
        }
}
void reduce_and_transform::transform_without_reductions() {

        {
                RED2PACK_SCOPED_TIMER("Construct 2-neighborhood");
                graph->construct_2neighborhood();

                for (size_t i = 0; i < graph->number_of_nodes(); i++) {
                        reduced_node_id[i] = i;
                        former_node_id[i] = i;
                }
        }

        m2s_log::instance()->print_reduction(solution_offset_weight, graph->number_of_nodes(),
                                             graph->number_of_edges() / 2, graph->number_of_links() / 2);

}
std::pair<bool, m2s_graph_access &> reduce_and_transform::run_reduce_and_transform() {
        RED2PACK_SCOPED_TIMER("Red(W)2pack reducer");
        // all reductions disabled?
        if (use_reducer()) {
                transform_with_reductions();
                return {reduced_graph.number_of_nodes() == 0, reduced_graph};
        } else {
                transform_without_reductions();
                return {graph->number_of_nodes() == 0, *graph};
        }
}

}  // namespace red2pack
