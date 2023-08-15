#ifndef INC_2_PACKING_SET_GRAPH_IO_H
#define INC_2_PACKING_SET_GRAPH_IO_H

#include "data_structures/m2s_graph_access.h"

namespace two_packing_set {
class m2s_graph_io {
       public:
        static void writeTwoPackingSet(const std::vector<bool> &sol, const std::string& filename);
        static int readGraphWeighted(m2s_graph_access &G, std::string filename);
};
}  // namespace two_packing_set

#endif  // INC_2_PACKING_SET_GRAPH_IO_H
