cmake_minimum_required(VERSION 3.16)

project(Approximate2Packing CXX)

include(CheckCXXCompilerFlag)

set(CMAKE_CXX_STANDARD 17)

if(CMAKE_BUILD_TYPE EQUAL "Release")
    add_definitions(-DNDEBUG)
    add_compile_options(-O3)
endif()

if(CMAKE_BUILD_TYPE EQUAL "Debug")
    add_compile_options(-g)
    add_compile_options(-fsanitize=address)
endif()

# cplex integration from https://github.com/martinsch/pgmlink/blob/ad0c7318a002897f64bb60284f2968a3a722051e/CMakeLists.txt
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "-pthread -fopenmp ${CMAKE_CXX_FLAGS}")
endif()

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/)
find_package( CPLEX )

if(CPLEX_FOUND)
    set(OPTIMIZER_INCLUDE_DIRS ${CPLEX_INCLUDE_DIRS})
    set(OPTIMIZER_LIBRARIES ${CPLEX_LIBRARIES})
endif()

include_directories(${OPTIMIZER_INCLUDE_DIRS})

add_definitions(-DIL_STD)

add_executable(apx-2p modified_source.cpp)
target_link_libraries(apx-2p ${OPTIMIZER_LIBRARIES})