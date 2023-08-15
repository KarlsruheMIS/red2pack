cmake_minimum_required(VERSION 3.16)
project(2-packing-set C CXX)

include(CheckCXXCompilerFlag)

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

CHECK_CXX_COMPILER_FLAG(-fpermissive COMPILER_SUPPORTS_PERMISSIVE)
if(COMPILER_SUPPORTS_PERMISSIVE)
    add_compile_options(-fpermissive)
endif()

CHECK_CXX_COMPILER_FLAG(-fno-omit-frame-pointer COMPILER_SUPPORTS_OMIT_FRAME_POINTER)
if(COMPILER_SUPPORTS_OMIT_FRAME_POINTER)
    add_compile_options(-fno-omit-frame-pointer)
endif()

CHECK_CXX_COMPILER_FLAG(-march=native COMPILER_SUPPORTS_MARCH_NATIVE)
if(COMPILER_SUPPORTS_MARCH_NATIVE)
    add_compile_options(-march=native)
endif()

CHECK_CXX_COMPILER_FLAG(-Wno-unused-value COMPILER_SUPPORTS_NOUNUSED)
if(COMPILER_SUPPORTS_NOUNUSED)
    add_compile_options(-Wno-unused-value)
endif()

CHECK_CXX_COMPILER_FLAG(-Wno-unused-value COMPILER_SUPPORTS_NOUNUSEDRES)
if(COMPILER_SUPPORTS_NOUNUSEDRES)
    add_compile_options(-Wno-unused-result)
endif()

CHECK_CXX_COMPILER_FLAG(-fno-stack-limit COMPILER_SUPPORTS_FNOSTACKLIMITS)
if(COMPILER_SUPPORTS_FNOSTACKLIMITS)
    add_compile_options(-fno-stack-limit)
endif()

CHECK_CXX_COMPILER_FLAG(-funroll-loops COMPILER_SUPPORTS_FUNROLL_LOOPS)
if(COMPILER_SUPPORTS_FUNROLL_LOOPS)
    add_compile_options(-funroll-loops)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# M2S CPP Files
set(M2S_BIN "app/m2s_heuristic.cpp")
set(M2S_LIB_SOURCES
        lib/algorithms/heuristic.cpp
        lib/algorithms/kernel/reduce_algorithm.cpp
        lib/algorithms/kernel/reductions.cpp
        lib/tools/m2s_graph_io.cpp
        lib/tools/mis_log.cpp
        lib/tools/m2s_log.cpp)

set(KAMIS ${CMAKE_CURRENT_SOURCE_DIR}/extern/KaMIS)

# KaMIS/wmis Headers
include_directories(
        ${KAMIS}/extern/argtable3-3.0.3
        ${KAMIS}/lib
        ${KAMIS}/lib/data_structure
        ${KAMIS}/lib/data_structure/priority_queues
        ${KAMIS}/lib/mis
        ${KAMIS}/lib/mis/ils
        ${KAMIS}/lib/mis/initial_mis
        ${KAMIS}/lib/mis/kernel
        ${KAMIS}/lib/tools
        ${KAMIS}/extern/KaHIP
        ${KAMIS}/extern/KaHIP/app
        ${KAMIS}/extern/KaHIP/interface
        ${KAMIS}/extern/KaHIP/lib
        ${KAMIS}/extern/KaHIP/lib/algorithms
        ${KAMIS}/extern/KaHIP/lib/data_structure
        ${KAMIS}/extern/KaHIP/lib/data_structure/matrix
        ${KAMIS}/extern/KaHIP/lib/data_structure/priority_queues
        ${KAMIS}/extern/KaHIP/lib/io
        ${KAMIS}/extern/KaHIP/lib/parallel_mh
        ${KAMIS}/extern/KaHIP/lib/parallel_mh/exchange
        ${KAMIS}/extern/KaHIP/lib/parallel_mh/galinier_combine
        ${KAMIS}/extern/KaHIP/lib/partition
        ${KAMIS}/extern/KaHIP/lib/partition/uncoarsening/refinement/quotient_graph_refinement/flow_refinement
        ${KAMIS}/extern/KaHIP/lib/tools
)


# KaMIS source files
add_subdirectory(${KAMIS})
# add_subdirectory(${KAMIS_wmis})

# M2S Headers
include_directories(lib)
include_directories(app)

# Maximum 2-Packing Set library
set(M2S_LIB two_packing_set_lib)
add_library(two_packing_set_lib OBJECT ${M2S_LIB_SOURCES})

# Executables
# Benchmark for heuristic algorithm
set(M2S_EXE m2s_heuristic)
add_executable(${M2S_EXE} ${M2S_BIN} $<TARGET_OBJECTS:libkaffpa> $<TARGET_OBJECTS:libfiles> $<TARGET_OBJECTS:libonlinemis> $<TARGET_OBJECTS:${M2S_LIB}>)
