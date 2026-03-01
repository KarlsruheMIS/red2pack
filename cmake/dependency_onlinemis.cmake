####################
# Dependency OnlineMIS
####################
# set OnlineMIS path
set(ONLINEMIS ${CMAKE_CURRENT_SOURCE_DIR}/extern/onlinemis)
set(ONLINEMIS_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/extern)
add_subdirectory(${ONLINEMIS} EXCLUDE_FROM_ALL)

add_library(onlinemis::onlinemis ALIAS libonlinemis)
