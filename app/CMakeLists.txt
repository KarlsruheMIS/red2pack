# executables for benchmarks
set(RED2PACK_CLI_LIB
    m2s_parse_parameters.h
    m2s_configuration_m2s.h
    benchmark_red2pack_solver.h
)
add_library(libred2pack_cli INTERFACE ${RED2PACK_CLI_LIB})
target_include_directories(libred2pack_cli INTERFACE "${KAMIS_WMIS_INCLUDES}")
add_library(kamis_wmis_based_solver_lib INTERFACE m2s_configuration_mis.h)

# set executables
set(RED2PACK_APP ${CMAKE_CURRENT_SOURCE_DIR})

set(RED2PACK_WEIGHTED_REDUCER_SOURCE  ${RED2PACK_APP}/reduce/redw2pack_reduce.cpp)
set(RED2PACK_WEIGHTED_DRP  ${RED2PACK_APP}/heuristic/redw2pack_drp.cpp)
set(RED2PACK_WEIGHTED_RNT_CHILS  ${RED2PACK_APP}/heuristic/redw2pack_rnt_chils.cpp)
set(RED2PACK_WEIGHTED_TRANSFORMER_SOURCE  ${RED2PACK_APP}/reduce/redw2pack_transform.cpp)
set(RED2PACK_BNR_SOURCE  ${RED2PACK_APP}/exact/red2pack_branch_and_reduce.cpp)
set(RED2PACK_WEIGHTED_RNT_EXACT_SOURCE  ${RED2PACK_APP}/exact/redw2pack_rnt_exact.cpp)
set(RED2PACK_WEIGHTED_RNT_MMWIS_SOURCE  ${RED2PACK_APP}/heuristic/redw2pack_rnt_mmwis.cpp)
set(RED2PACK_HEURISTIC_SOURCE  ${RED2PACK_APP}/heuristic/red2pack_heuristic.cpp)
set(RED2PACK_WEIGHTED_RNT_HTWIS  ${RED2PACK_APP}/heuristic/redw2pack_rnt_htwis.cpp)
set(RED2PACK_WEIGHTED_RNT_HILS  ${RED2PACK_APP}/heuristic/redw2pack_rnt_hils.cpp)

if (RED2PACK_BUILD_ILP_SOLVERS)
  set(W2PACK_SCIP_ILP_SOURCE  ${RED2PACK_APP}/exact/w2pack_scip_ilp.cpp)
  set(RED2PACK_WEIGHTED_RNT_SCIP_ILP  ${RED2PACK_APP}/exact/redw2pack_rnt_scip_ilp.cpp)
endif()


# add executables
add_executable(redw2pack_reduce ${RED2PACK_WEIGHTED_REDUCER_SOURCE})
add_executable(redw2pack_transform ${RED2PACK_WEIGHTED_TRANSFORMER_SOURCE})
add_executable(redw2pack_drp ${RED2PACK_WEIGHTED_DRP})
add_executable(redw2pack_rnt_chils ${RED2PACK_WEIGHTED_RNT_CHILS})
add_executable(red2pack_branch_and_reduce ${RED2PACK_BNR_SOURCE})
add_executable(redw2pack_rnt_exact ${RED2PACK_WEIGHTED_RNT_EXACT_SOURCE})
add_executable(redw2pack_rnt_htwis ${RED2PACK_WEIGHTED_RNT_HTWIS})
add_executable(redw2pack_rnt_hils ${RED2PACK_WEIGHTED_RNT_HILS})
add_executable(redw2pack_rnt_mmwis ${RED2PACK_WEIGHTED_RNT_MMWIS_SOURCE})
add_executable(red2pack_heuristic ${RED2PACK_HEURISTIC_SOURCE})

if (RED2PACK_BUILD_ILP_SOLVERS)
  add_executable(w2pack_scip_ilp ${W2PACK_SCIP_ILP_SOURCE})
  add_executable(redw2pack_rnt_scip_ilp ${RED2PACK_WEIGHTED_RNT_SCIP_ILP})
endif()

# link red2pack-based solvers with cli and red2pack-lib
target_link_libraries(redw2pack_reduce PRIVATE libred2pack_cli red2pack::lib)
target_link_libraries(redw2pack_transform PRIVATE libred2pack_cli red2pack::lib)
target_link_libraries(redw2pack_drp PRIVATE libred2pack_cli red2pack::chils::lib)
target_link_libraries(redw2pack_rnt_chils PRIVATE libred2pack_cli red2pack::chils::lib)
target_link_libraries(red2pack_branch_and_reduce PRIVATE libred2pack_cli red2pack::kamis-wmis::lib)
target_link_libraries(redw2pack_rnt_exact PRIVATE libred2pack_cli red2pack::kamis-wmis::lib)
target_link_libraries(redw2pack_rnt_htwis PRIVATE libred2pack_cli red2pack::htwis-hils::lib)
target_link_libraries(redw2pack_rnt_hils PRIVATE libred2pack_cli red2pack::htwis-hils::lib)
target_link_libraries(redw2pack_rnt_mmwis PRIVATE libred2pack_cli red2pack::mmwis::lib)
target_link_libraries(red2pack_heuristic PRIVATE libred2pack_cli red2pack::onlinemis::lib)


if (RED2PACK_BUILD_ILP_SOLVERS)
target_link_libraries(w2pack_scip_ilp PRIVATE libscip)
target_include_directories(w2pack_scip_ilp PRIVATE ${SCIP_INCLUDE_DIRS})
target_link_libraries(w2pack_scip_ilp PRIVATE libred2pack_cli red2pack::lib)

target_link_libraries(redw2pack_rnt_scip_ilp PRIVATE libscip)
target_include_directories(redw2pack_rnt_scip_ilp PRIVATE ${SCIP_INCLUDE_DIRS})
target_link_libraries(redw2pack_rnt_scip_ilp PRIVATE libred2pack_cli red2pack::scip-ilp::lib)
endif()

# link malloc count
if(RED2PACK_USE_MALLOC_COUNT)
  target_link_libraries(redw2pack_reduce PRIVATE malloc_count_lib)
  target_link_libraries(redw2pack_transform PRIVATE malloc_count_lib)
  target_link_libraries(red2pack_branch_and_reduce PRIVATE malloc_count_lib)
  target_link_libraries(redw2pack_rnt_exact PRIVATE malloc_count_lib)
  target_link_libraries(redw2pack_rnt_htwis PRIVATE malloc_count_lib)
  target_link_libraries(redw2pack_rnt_hils PRIVATE malloc_count_lib)
  target_link_libraries(redw2pack_rnt_mmwis PRIVATE malloc_count_lib)
  target_link_libraries(redw2pack_drp PRIVATE malloc_count_lib)
  target_link_libraries(redw2pack_rnt_chils PRIVATE malloc_count_lib)
  target_link_libraries(red2pack_heuristic PRIVATE malloc_count_lib)

  if(RED2PACK_BUILD_ILP_SOLVERS)
    target_link_libraries(w2pack_ilp PRIVATE malloc_count_lib)
    target_link_libraries(w2pack_scip_ilp PRIVATE malloc_count_lib)
    target_link_libraries(redw2pack_rnt_scip_ilp PRIVATE malloc_count_lib)
  endif()
endif()

install(TARGETS red2pack_branch_and_reduce DESTINATION ${INSTALL_DIR})
install(TARGETS redw2pack_reduce DESTINATION ${INSTALL_DIR})
install(TARGETS redw2pack_transform DESTINATION ${INSTALL_DIR})
install(TARGETS redw2pack_rnt_exact DESTINATION ${INSTALL_DIR})
install(TARGETS redw2pack_rnt_htwis DESTINATION ${INSTALL_DIR})
install(TARGETS redw2pack_rnt_hils DESTINATION ${INSTALL_DIR})
install(TARGETS redw2pack_rnt_mmwis DESTINATION ${INSTALL_DIR})
install(TARGETS redw2pack_drp DESTINATION ${INSTALL_DIR})
install(TARGETS redw2pack_rnt_chils DESTINATION ${INSTALL_DIR})
install(TARGETS red2pack_heuristic DESTINATION ${INSTALL_DIR})

if(RED2PACK_BUILD_ILP_SOLVERS)
install(TARGETS w2pack_ilp DESTINATION ${INSTALL_DIR})
install(TARGETS w2pack_scip_ilp DESTINATION ${INSTALL_DIR})
install(TARGETS redw2pack_rnt_scip_ilp DESTINATION ${INSTALL_DIR})
endif()
