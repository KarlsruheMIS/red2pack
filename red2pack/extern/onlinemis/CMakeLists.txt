set(ONLINEMIS_SOURCES
        ils/local_search_online.cpp
        ils/online_ils.cpp
        tools/mis_log.cpp
        initial_mis/greedy_mis_online.cpp
        data_structure/candidate_list.cpp
        data_structure/mis_permutation_online.cpp
        data_structure/priority_queues/bucket_array.cpp)
set(ONLINEMIS_LIB libonlinemis)

# onlinemis includes
set(ONLINEMIS_INCLUDES
        ${ONLINEMIS}
        ${ONLINEMIS}/ils
        ${ONLINEMIS}/initial_mis
        ${ONLINEMIS}/data_structure
        )

include_directories(ONLINEMIS_INCLUDES)

add_library(${ONLINEMIS_LIB} ${ONLINEMIS_SOURCES})