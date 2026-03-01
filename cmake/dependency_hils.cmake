####################
# Dependency Hils
####################
set(HILS ${CMAKE_CURRENT_SOURCE_DIR}/extern/hils)
add_library(hils_lib
        ${HILS}/src/ArgPack.h
        ${HILS}/src/ArgPack.cpp
        ${HILS}/src/Graph.h
        ${HILS}/src/Graph.cpp
        ${HILS}/src/Solution.cpp
        ${HILS}/src/Solution.h
)
target_include_directories(hils_lib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/extern")
add_library(hils::hils ALIAS hils_lib)
