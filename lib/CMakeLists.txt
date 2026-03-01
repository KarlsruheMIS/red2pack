# add the red2pack and its extensions
add_subdirectory(red2pack)
add_subdirectory(red2pack-kamis-wmis)
add_subdirectory(red2pack-htwis-hils)
add_subdirectory(red2pack-mmwis)
add_subdirectory(red2pack-chils)
add_subdirectory(red2pack-onlinemis)

if (RED2PACK_BUILD_ILP_SOLVERS)
  add_subdirectory(red2pack-scip-ilp)
endif()
