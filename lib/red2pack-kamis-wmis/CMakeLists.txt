file(GLOB_RECURSE RED2PACK_SOURCE_FILES CONFIGURE_DEPENDS
        *.cpp *.h)


add_library(libred2pack-kamis-wmis ${RED2PACK_SOURCE_FILES})
target_include_directories(libred2pack-kamis-wmis PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../")
target_link_libraries(libred2pack-kamis-wmis PUBLIC kamis::wmis libred2pack)
# expose KaHIP headers for executables
target_include_directories(libred2pack-kamis-wmis PUBLIC ${KAMIS_WMIS_KAHIP_INCLUDES})

add_library(libred2pack-kamis-wmis-interface INTERFACE)
target_include_directories(libred2pack-kamis-wmis-interface INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/../")

add_library(red2pack::kamis-wmis::lib ALIAS libred2pack-kamis-wmis)
add_library(red2pack::kamis-wmis::interface ALIAS libred2pack-kamis-wmis-interface)

