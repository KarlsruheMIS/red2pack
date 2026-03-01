file(GLOB_RECURSE RED2PACK_SOURCE_FILES CONFIGURE_DEPENDS
        *.cpp *.h)


add_library(libred2pack-htwis-hils ${RED2PACK_SOURCE_FILES})
target_include_directories(libred2pack-htwis-hils PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../")
target_link_libraries(libred2pack-htwis-hils PUBLIC kamis::wmis htwis::htwis hils::hils libred2pack)
target_include_directories(libred2pack-htwis-hils PUBLIC ${KAMIS_WMIS_KAHIP_INCLUDES})

add_library(libred2pack-htwis-hils-interface INTERFACE)
target_include_directories(libred2pack-htwis-hils-interface INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/../")

add_library(red2pack::htwis-hils::interface ALIAS libred2pack-htwis-hils-interface)
add_library(red2pack::htwis-hils::lib ALIAS libred2pack-htwis-hils)


