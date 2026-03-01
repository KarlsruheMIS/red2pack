####################
# Dependency CHILS
####################
set(CHILS_SOURCES
        extern/CHILS/src/graph.c
        extern/CHILS/src/chils.c
        extern/CHILS/src/local_search.c
        extern/CHILS/src/main.c
)

set(CHILS_INCLUDES
        extern/CHILS/include
)

find_package(OpenMP REQUIRED)
add_library(CHILS STATIC ${CHILS_INCLUDES} ${CHILS_SOURCES})
target_include_directories(CHILS PUBLIC ${CHILS_INCLUDES})
target_link_libraries(CHILS PUBLIC OpenMP::OpenMP_CXX)
add_library(CHILS::CHILS ALIAS CHILS)


