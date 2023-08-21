
# RED2PACK CXX Files
set(RED2PACK_LIB_SOURCES
        algorithms/kernel/reduce_and_transform.cpp
        algorithms/kernel/reduce_and_transform.h
        algorithms/kernel/reduce_algorithm.cpp
        algorithms/kernel/reduce_algorithm.h
        algorithms/kernel/reductions.cpp
        algorithms/kernel/reductions.h
        algorithms/solver_scheme.cpp
        algorithms/solver_scheme.h
        algorithms/branch_and_reduce.cpp
        algorithms/branch_and_reduce.h
        tools/m2s_graph_io.cpp
        tools/m2s_graph_io.h
        tools/m2s_log.cpp
        tools/m2s_log.h
        data_structure/m2s_dynamic_graph.h
        data_structure/m2s_graph_access.h
        m2s_config.h)

# sources depending on ONLINEMIS
set(RED2PACK_LIB_SOURCES_ONLINEMIS
        algorithms/heuristic.cpp
        algorithms/heuristic.h
        )

# M2S Headers
include_directories(.)

# Maximum 2-Packing Set library red2pack_lib
set(RED2PACK_LIB libred2pack)
add_library(${RED2PACK_LIB} OBJECT ${RED2PACK_LIB_SOURCES})
add_library(${RED2PACK_LIB}-onlinemis OBJECT ${RED2PACK_LIB_SOURCES_ONLINEMIS})