file(GLOB_RECURSE RED2PACK_SOURCE_FILES CONFIGURE_DEPENDS
        *.cpp *.h)

add_library(libred2pack-scip-ilp ${RED2PACK_SOURCE_FILES})
target_include_directories(libred2pack-scip-ilp PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../")
target_link_libraries(libred2pack-scip-ilp PUBLIC libred2pack)
target_link_libraries(libred2pack-scip-ilp PRIVATE libscip)
target_include_directories(libred2pack-scip-ilp PRIVATE ${SCIP_INCLUDE_DIRS})

target_include_directories(libred2pack-scip-ilp PRIVATE ${KAMIS_WMIS_KAHIP_INCLUDES})
target_link_libraries(libred2pack-scip-ilp PRIVATE kamis::wmis)

add_library(libred2pack-scip-ilp-interface INTERFACE)
target_include_directories(libred2pack-scip-ilp-interface INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/../")

add_library(red2pack::scip-ilp::interface ALIAS libred2pack-scip-ilp-interface)
add_library(red2pack::scip-ilp::lib ALIAS libred2pack-scip-ilp)


