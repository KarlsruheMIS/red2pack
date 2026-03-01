#ifndef INC_2_PACKING_SET_GRAPH_IO_H
#define INC_2_PACKING_SET_GRAPH_IO_H

#include "red2pack/data_structure/m2s_graph_access.h"

namespace red2pack {
class m2s_graph_io {
       public:
        static void writeTwoPackingSet(const std::vector<bool> &sol, const std::string &filename);
        static int readGraphWeighted(m2s_graph_access &G, std::string filename);
        static int writeGraphWeighted(m2s_graph_access &G, const std::string& filename_direct_neighborhood, const std::string& filename_two_neighborhood);
};
}  // namespace red2pack

#endif  // INC_2_PACKING_SET_GRAPH_IO_H
