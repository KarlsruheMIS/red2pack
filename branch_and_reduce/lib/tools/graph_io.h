//
// Created by Jannick Borowitz on 21.06.23.
//

#ifndef INC_2_PACKING_SET_GRAPH_IO_H
#define INC_2_PACKING_SET_GRAPH_IO_H

#include "data_structures/m2s_graph_access.h"

namespace two_packing_set {
class graph_io {
public:
  static int readGraphWeighted(m2s_graph_access &G, std::string filename);
};
} // namespace two_packing_set

#endif // INC_2_PACKING_SET_GRAPH_IO_H
