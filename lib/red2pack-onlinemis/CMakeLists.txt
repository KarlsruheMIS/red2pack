file(GLOB_RECURSE RED2PACK_SOURCE_FILES CONFIGURE_DEPENDS
        *.cpp *.h)

add_library(libred2pack-onlinemis STATIC ${RED2PACK_SOURCE_FILES})
target_include_directories(libred2pack-onlinemis PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../")
target_include_directories(libred2pack-onlinemis PUBLIC ${KAMIS_WMIS_KAHIP_INCLUDES})
target_link_libraries(libred2pack-onlinemis PUBLIC libred2pack kamis::wmis onlinemis::onlinemis)

add_library(libred2pack-onlinemis-interface INTERFACE)
target_include_directories(libred2pack-onlinemis-interface INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/../")

add_library(red2pack::onlinemis::interface ALIAS libred2pack-onlinemis-interface)
add_library(red2pack::onlinemis::lib ALIAS libred2pack-onlinemis)
