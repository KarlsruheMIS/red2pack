####################
# Dependency KaMIS/wmis
####################
# set KaMIS/wmis path for red2pack-lib and includes for app/
set(KAMIS_WMIS ${CMAKE_CURRENT_SOURCE_DIR}/extern/KaMIS/wmis)
# includes KaMIS
set(KAMIS_WMIS_INCLUDES
        ${KAMIS_WMIS}/extern/argtable3-3.0.3
        ${KAMIS_WMIS}/lib
        ${KAMIS_WMIS}/lib/data_structure
        ${KAMIS_WMIS}/lib/data_structure/priority_queues
        ${KAMIS_WMIS}/lib/mis
        ${KAMIS_WMIS}/lib/mis/ils
        ${KAMIS_WMIS}/lib/mis/initial_mis
        ${KAMIS_WMIS}/lib/mis/kernel
        ${KAMIS_WMIS}/lib/tools

)
set(KAMIS_WMIS_KAHIP_INCLUDES
        ${KAMIS_WMIS}/extern/KaHIP
        ${KAMIS_WMIS}/extern/KaHIP/app
        ${KAMIS_WMIS}/extern/KaHIP/interface
        ${KAMIS_WMIS}/extern/KaHIP/lib
        ${KAMIS_WMIS}/extern/KaHIP/lib/algorithms
        ${KAMIS_WMIS}/extern/KaHIP/lib/data_structure
        ${KAMIS_WMIS}/extern/KaHIP/lib/data_structure/matrix
        ${KAMIS_WMIS}/extern/KaHIP/lib/data_structure/priority_queues
        ${KAMIS_WMIS}/extern/KaHIP/lib/io
        ${KAMIS_WMIS}/extern/KaHIP/lib/parallel_mh
        ${KAMIS_WMIS}/extern/KaHIP/lib/parallel_mh/exchange
        ${KAMIS_WMIS}/extern/KaHIP/lib/parallel_mh/galinier_combine
        ${KAMIS_WMIS}/extern/KaHIP/lib/partition
        ${KAMIS_WMIS}/extern/KaHIP/lib/partition/uncoarsening/refinement/quotient_graph_refinement/flow_refinement
        ${KAMIS_WMIS}/extern/KaHIP/lib/tools
)
# add KaMIS/wmis
add_subdirectory(${KAMIS_WMIS} EXCLUDE_FROM_ALL)

# this is the original KaHIP version used by KaMIS
add_library(lib_kamis_kahip STATIC $<TARGET_OBJECTS:libkaffpa2>)
target_include_directories(lib_kamis_kahip PUBLIC ${KAMIS_WMIS_KAHIP_INCLUDES})

# KaMIS only exposes its own header files
add_library(lib_kamis_wmis STATIC $<TARGET_OBJECTS:libsources>)
target_include_directories(lib_kamis_wmis PUBLIC ${KAMIS_WMIS_INCLUDES})
target_link_libraries(lib_kamis_wmis PRIVATE lib_kamis_kahip)
add_library(kamis::wmis ALIAS lib_kamis_wmis)