set(ONLINEMIS_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/ils/local_search_online.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ils/local_search_online.h
        ${CMAKE_CURRENT_SOURCE_DIR}/ils/online_ils.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ils/online_ils.h
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/mis_log.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/mis_log.h
        ${CMAKE_CURRENT_SOURCE_DIR}/initial_mis/greedy_mis_online.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/initial_mis/greedy_mis_online.h
        ${CMAKE_CURRENT_SOURCE_DIR}/data_structure/candidate_list.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/data_structure/candidate_list.h
        ${CMAKE_CURRENT_SOURCE_DIR}/data_structure/mis_permutation_online.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/data_structure/mis_permutation_online.h
        ${CMAKE_CURRENT_SOURCE_DIR}/data_structure/priority_queues/bucket_array.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/data_structure/priority_queues/bucket_array.h
)
set(ONLINEMIS_LIB libonlinemis)

# onlinemis includes
set(ONLINEMIS_INCLUDES
        ${ONLINEMIS}/../
        ${ONLINEMIS}
        ${ONLINEMIS}/ils
        ${ONLINEMIS}/initial_mis
        ${ONLINEMIS}/data_structure
        )


add_library(${ONLINEMIS_LIB} ${ONLINEMIS_SOURCES})
target_link_libraries(${ONLINEMIS_LIB} PRIVATE kamis::wmis)
target_include_directories(${ONLINEMIS_LIB} PRIVATE "${KAMIS_WMIS_KAHIP_INCLUDES}")
target_include_directories(${ONLINEMIS_LIB} PUBLIC "${ONLINEMIS_INCLUDES}")
