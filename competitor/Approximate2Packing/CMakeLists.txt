cmake_minimum_required(VERSION 3.16)

project(Approximate2Packing C CXX)

include(CheckCXXCompilerFlag)

if(CMAKE_BUILD_TYPE EQUAL "Release")
    add_definitions(-DNDEBUG)
    add_compile_options(-O3)
endif()

if(CMAKE_BUILD_TYPE EQUAL "Debug")
    add_compile_options(-g)
    add_compile_options(-fsanitize=address)
endif()

# cplex integration from https://github.com/martinsch/pgmlink/blob/ad0c7318a002897f64bb60284f2968a3a722051e/CMakeLists.txt
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/)
find_package( Cplex )

if(CPLEX_FOUND)
    set(OPTIMIZER_INCLUDE_DIRS ${CPLEX_INCLUDE_DIRS})
    set(OPTIMIZER_LIBRARIES ${CPLEX_LIBRARIES})
endif()

ADD_DEFINITIONS(-DIL_STD)

add_executable(apx-2p modified_source.cpp)