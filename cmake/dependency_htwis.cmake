####################
# Dependency HtWIS
####################
set(HTWIS_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/extern/htwis)
add_library(htwis_lib extern/htwis/Graph.h extern/htwis/Graph.cpp)
target_include_directories(htwis_lib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/extern")

add_library(htwis::htwis ALIAS htwis_lib)
