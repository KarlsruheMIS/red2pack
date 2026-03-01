#ifndef RED2PACK_RED2PACK_DEF_H
#define RED2PACK_RED2PACK_DEF_H

#include <cstdint>
namespace red2pack {

enum class two_pack_status { not_set, included, folded, excluded };
enum class mw2ps_heuristic_ratings { weight_diff, weight, deg };

using NodeID = unsigned int;
using PartitionID = unsigned int;
#ifdef RED2PACK_64BITS_MODE
using EdgeID = uint64_t;
#else
using EdgeID = uint32_t;
#endif
using NodeWeight = unsigned int;
using EdgeWeight = int;
using EdgeRatingType = double;

}  // namespace red2pack

#endif  // RED2PACK_RED2PACK_DEF_H
