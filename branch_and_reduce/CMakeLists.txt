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
set(M2S_BIN "app/m2s_branch_and_reduce.cpp")
set(M2S_LIB_SOURCES
        lib/algorithms/branch_and_reduce.cpp
        lib/algorithms/kernel/reduce_algorithm.cpp
        lib/algorithms/kernel/reductions.cpp
        lib/tools/graph_io.cpp
        lib/tools/m2s_log.cpp)

set(KAMIS ${CMAKE_CURRENT_SOURCE_DIR}/extern/KaMIS)
set(KAMIS_wmis ${KAMIS}/wmis)

# KaMIS/wmis Headers
include_directories(
        ${KAMIS_wmis}/extern/argtable3-3.0.3
        ${KAMIS_wmis}/lib
        ${KAMIS_wmis}/lib/data_structure
        ${KAMIS_wmis}/lib/data_structure/priority_queues
        ${KAMIS_wmis}/lib/mis
        ${KAMIS_wmis}/lib/mis/ils
        ${KAMIS_wmis}/lib/mis/initial_mis
        ${KAMIS_wmis}/lib/mis/kernel
        ${KAMIS_wmis}/lib/tools
        ${KAMIS_wmis}/extern/KaHIP
        ${KAMIS_wmis}/extern/KaHIP/app
        ${KAMIS_wmis}/extern/KaHIP/interface
        ${KAMIS_wmis}/extern/KaHIP/lib
        ${KAMIS_wmis}/extern/KaHIP/lib/algorithms
        ${KAMIS_wmis}/extern/KaHIP/lib/data_structure
        ${KAMIS_wmis}/extern/KaHIP/lib/data_structure/matrix
        ${KAMIS_wmis}/extern/KaHIP/lib/data_structure/priority_queues
        ${KAMIS_wmis}/extern/KaHIP/lib/io
        ${KAMIS_wmis}/extern/KaHIP/lib/parallel_mh
        ${KAMIS_wmis}/extern/KaHIP/lib/parallel_mh/exchange
        ${KAMIS_wmis}/extern/KaHIP/lib/parallel_mh/galinier_combine
        ${KAMIS_wmis}/extern/KaHIP/lib/partition
        ${KAMIS_wmis}/extern/KaHIP/lib/partition/uncoarsening/refinement/quotient_graph_refinement/flow_refinement
        ${KAMIS_wmis}/extern/KaHIP/lib/tools
)

# KaMIS source files
add_subdirectory(${KAMIS_wmis})

# M2S Headers
include_directories(lib)
include_directories(app)

# Maximum 2-Packing Set library
set(M2S_LIB two_packing_set_lib)
add_library(two_packing_set_lib OBJECT ${M2S_LIB_SOURCES})

# TESTS
if (CMAKE_ENABLE_TESTING)
    add_subdirectory(tests)
    enable_testing()
endif (CMAKE_ENABLE_TESTING)

# Executables
# Benchmark for Branch and Reduce algorithm
set(M2S_EXE m2s_branch_and_reduce)
add_executable(${M2S_EXE} ${M2S_BIN} $<TARGET_OBJECTS:libkaffpa2> $<TARGET_OBJECTS:libsources> $<TARGET_OBJECTS:${M2S_LIB}>)
