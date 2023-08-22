cmake_minimum_required(VERSION 3.16)
project(red2pack C CXX)

include(CheckCXXCompilerFlag)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_program(CCACHE_PROGRAM ccache)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

if(CCACHE_PROGRAM)
    message(STATUS "Using compiler cache")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CCACHE_PROGRAM}")
endif()

if (NOT CMAKE_ENABLE_TESTING)
    set(CMAKE_BUILD_TYPE False)
endif (NOT CMAKE_ENABLE_TESTING)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif (NOT CMAKE_BUILD_TYPE)

#probably set by Release, but doing anyway.
if(CMAKE_BUILD_TYPE EQUAL "Release")
    add_definitions(-DNDEBUG)
    add_compile_options(-O3)
endif()

if(CMAKE_BUILD_TYPE EQUAL "Debug")
    add_compile_options(-g)
    add_compile_options(-fsanitize=address)
endif()

add_compile_options(-fpermissive)

# set KaMIS/wmis path for red2pack-lib and includes for app/
set(KAMIS_WMIS ${CMAKE_CURRENT_SOURCE_DIR}/extern/KaMIS/wmis)
# set OnlineMIS path
set(ONLINEMIS ${CMAKE_CURRENT_SOURCE_DIR}/extern/onlinemis)
set(RED2PACK_LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(RED2PACK_LIB libred2pack)


# set executables
set(RED2PACK_APP ${CMAKE_CURRENT_SOURCE_DIR}/app)
set(RED2PACK_BNR_SOURCE  ${RED2PACK_APP}/bnr/red2pack_branch_and_reduce.cpp)
set(RED2PACK_HEURISTIC_SOURCE  ${RED2PACK_APP}/heuristic/red2pack_heuristic.cpp)

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
# onlinemis includes
set(ONLINEMIS_INCLUDES
        ${ONLINEMIS}
        ${ONLINEMIS}/ils
        ${ONLINEMIS}/initial_mis
        ${ONLINEMIS}/data_structure
        )

# includes red2pack
set(RED2PACK_INCLUDES ${RED2PACK_LIB_PATH})


# includes
include_directories(${KAMIS_WMIS_INCLUDES})
IF (NOT "${ONLINEMIS}" STREQUAL "")
    include_directories(${ONLINEMIS_INCLUDES})
ENDIF()
include_directories(${RED2PACK_INCLUDES})


# add KaMIS/wmis
add_subdirectory(${KAMIS_WMIS} EXCLUDE_FROM_ALL)

# add onlinemis
IF (NOT "${ONLINEMIS}" STREQUAL "")
    add_subdirectory(${ONLINEMIS})
ENDIF()

# add red2pack-lib
add_subdirectory(${RED2PACK_LIB_PATH})

# add tests?
set(CMAKE_ENABLE_TESTING True)
IF (CMAKE_ENABLE_TESTING)
    add_subdirectory(tests)
    enable_testing()
ENDIF ()

# add executables
add_executable(red2pack_branch_and_reduce ${RED2PACK_BNR_SOURCE} $<TARGET_OBJECTS:libkaffpa2> $<TARGET_OBJECTS:libsources> "$<TARGET_OBJECTS:${RED2PACK_LIB}>")

IF (NOT "${ONLINEMIS}" STREQUAL "")
    add_executable(red2pack_heuristic ${RED2PACK_HEURISTIC_SOURCE} $<TARGET_OBJECTS:libkaffpa2> $<TARGET_OBJECTS:libsources> $<TARGET_OBJECTS:libonlinemis> "$<TARGET_OBJECTS:${RED2PACK_LIB}>" "$<TARGET_OBJECTS:${RED2PACK_LIB}-onlinemis>")
ENDIF()

install(TARGETS red2pack_branch_and_reduce DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/deploy)

IF (NOT "${ONLINEMIS}" STREQUAL "")
    install(TARGETS red2pack_heuristic DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/deploy)
ENDIF()
