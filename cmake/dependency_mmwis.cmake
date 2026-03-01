####################
# Dependency KaMIS/mmwis (m^2wis+s)
####################
set(MMWIS ${CMAKE_CURRENT_SOURCE_DIR}/extern/KaMIS/mmwis)
add_subdirectory(${MMWIS} EXCLUDE_FROM_ALL)
add_library(mmwis::lib ALIAS lib_mmwis)
# mmwiss depends on KaHIP
add_library(mmwis_kahip STATIC  $<TARGET_OBJECTS:libkaffpa>
        $<TARGET_OBJECTS:libmapping>
        $<TARGET_OBJECTS:libnodeordering> )
target_link_libraries(lib_mmwis PRIVATE mmwis_kahip)
# provide headers for red2pack
set(KaHIP_MMWIS ${MMWIS}/extern/KaHIP)
set(modKaHIP ${MMWIS}/lib/modKaHIPFiles)
set(KaHIP_INCLUDES
        ${modKaHIP}/interface
        ${modKaHIP}/lib
        ${modKaHIP}/lib/data_structure
        ${modKaHIP}/../data_structure
        ${modKaHIP}/../data_structure/priority_queues
        ${KaHIP_MMWIS}/app
        ${KaHIP_MMWIS}/lib
        ${KaHIP_MMWIS}/lib/io
        ${KaHIP_MMWIS}/lib/tools
        ${KaHIP_MMWIS}/lib/algorithms
        ${KaHIP_MMWIS}/lib/partition
        ${KaHIP_MMWIS}/lib/data_structure/priority_queues
        ${KaHIP_MMWIS}/lib/partition/uncoarsening/refinement/quotient_graph_refinement/flow_refinement
)
set(MMWIS_INCLUDES
        ${MMWIS}/app
        ${MMWIS}/lib
        ${MMWIS}/lib/data_structure
        ${MMWIS}/lib/data_structure/priority_queues
        ${MMWIS}/lib/mis
        ${MMWIS}/lib/mis/ils
        ${MMWIS}/lib/mis/hils
        ${MMWIS}/lib/mis/initial_mis
        ${MMWIS}/lib/mis/kernel
        ${MMWIS}/lib/mis/evolutionary
        ${MMWIS}/lib/mis/evolutionary/combine
        ${MMWIS}/lib/mis/max_flow
        ${MMWIS}/lib/tools
        ${MMWIS}/extern/
        ${MMWIS}/extern/struction/lib/mis/kernel
        ${MMWIS}/extern/struction/lib/data_structure
        ${KaHIP_INCLUDES}
)
# interface library that provides mmwiss headers
add_library(lib-mmwis-interface INTERFACE)
target_include_directories(lib-mmwis-interface INTERFACE ${MMWIS_INCLUDES})
add_library(mmwis::interface ALIAS lib-mmwis-interface)


