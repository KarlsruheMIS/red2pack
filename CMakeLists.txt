cmake_minimum_required(VERSION 3.16)
project(
        red2pack
        VERSION 2.0.0
        DESCRIPTION "heuristic and exact solvers for the maximum (weight) 2-packing set problem"
        LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CheckCXXCompilerFlag)

###########################
# Build Options
###########################
option(RED2PACK_ENABLE_APPS "Build all executables." ON)
option(RED2PACK_ENABLE_TESTS "Build tests" OFF)
option(RED2PACK_ENABLE_EXAMPLES "Build examples" ON)
option(RED2PACK_BUILD_ILP_SOLVERS "Build ILP solvers" OFF)
option(RED2PACK_USE_MALLOC_COUNT "Use malloc_count to profile memory usage" ON)
option(RED2PACK_64BITS_MODE "Use 64bit for edge ids" OFF)

if (RED2PACK_64BITS_MODE)
  add_definitions(-DRED2PACK_64BITS_MODE)
  add_definitions(-D64BITMODE)
  message("sizeof(red2pack::EdgeID)=64")
else ()
  message("sizeof(red2pack::EdgeID)=32")
endif ()

###########################
# Settings
###########################
set(NOMPI ON)
set(STRUCTION_ENABLE_APPS OFF)  # targets are not needed and not unique if used with KaMIS/wmis in a project

if (NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose build type: Debug or Release" FORCE)
endif (NOT DEFINED CMAKE_BUILD_TYPE)
message("Build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_program(CCACHE_PROGRAM ccache)
if (CCACHE_PROGRAM)
    message(STATUS "Using compiler cache")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CCACHE_PROGRAM}")
endif ()

###############################
# Build Types & Compile Options
###############################
add_compile_options(-Wall)
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-DNDEBUG)
    add_compile_options(-O3)
    #add_compile_options(-g)

    CHECK_CXX_COMPILER_FLAG(-fno-omit-frame-pointer COMPILER_SUPPORTS_OMIT_FRAME_POINTER)
    if (COMPILER_SUPPORTS_OMIT_FRAME_POINTER)
        #add_compile_options(-fno-omit-frame-pointer)
    endif ()

    CHECK_CXX_COMPILER_FLAG(-march=native COMPILER_SUPPORTS_MARCH_NATIVE)
    if (COMPILER_SUPPORTS_MARCH_NATIVE)
        add_compile_options(-march=native)
    endif ()

    CHECK_CXX_COMPILER_FLAG(-funroll-loops COMPILER_SUPPORTS_FUNROLL_LOOPS)
    if (COMPILER_SUPPORTS_FUNROLL_LOOPS)
        add_compile_options(-funroll-loops)
    endif ()

endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g)
    #add_compile_options(-fsanitize=address)
endif ()

CHECK_CXX_COMPILER_FLAG(-fno-stack-limit COMPILER_SUPPORTS_FNOSTACKLIMITS)
if (COMPILER_SUPPORTS_FNOSTACKLIMITS)
    add_compile_options(-fno-stack-limit)
endif ()

CHECK_CXX_COMPILER_FLAG(-fpermissive COMPILER_SUPPORTS_PERMISSIVE)
if (COMPILER_SUPPORTS_PERMISSIVE)
    add_compile_options(-fpermissive)
endif ()

CHECK_CXX_COMPILER_FLAG(-Wno-unused-value COMPILER_SUPPORTS_NOUNUSED)
if (COMPILER_SUPPORTS_NOUNUSED)
    add_compile_options(-Wno-unused-value)
endif ()

CHECK_CXX_COMPILER_FLAG(-Wno-unused-value COMPILER_SUPPORTS_NOUNUSEDRES)
if (COMPILER_SUPPORTS_NOUNUSEDRES)
    add_compile_options(-Wno-unused-result)
endif ()


####################
# Dependency malloc_count
####################
if (RED2PACK_USE_MALLOC_COUNT)
    message("Not supported for MaOS; So far only successful tested with Linux/Debian.")
    add_library(malloc_count_lib extern/malloc_count/malloc_count.c)
    target_include_directories(malloc_count_lib SYSTEM PUBLIC extern/malloc_count)
endif ()

####################
# Dependencies
####################
include(${CMAKE_CURRENT_LIST_DIR}/cmake/dependency_kamis_wmis.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/dependency_hils.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/dependency_onlinemis.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/dependency_chils.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/dependency_htwis.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/dependency_mmwis.cmake)

if (RED2PACK_BUILD_ILP_SOLVERS)
find_package(SCIP REQUIRED) 
message(STATUS "SCIP include dirs: ${SCIP_INCLUDE_DIRS}")
message(STATUS "SCIP libraries:   ${SCIP_LIBRARIES}")
endif()



####################
# Libraries
####################
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib)

####################
# Link to dl
####################
if (RED2PACK_USE_MALLOC_COUNT)
    add_definitions(-DRED2PACK_ENABLE_MALLOC)
    target_link_libraries(malloc_count_lib PUBLIC dl)
endif ()

####################
# Executables
####################
if (RED2PACK_ENABLE_APPS)
    set(INSTALL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deploy)
    add_subdirectory(app/)
endif ()

####################
# Tests
####################
if (RED2PACK_ENABLE_TESTS)
    add_subdirectory(tests)
    enable_testing()
endif ()
