file(GLOB_RECURSE RED2PACK_SOURCE_FILES CONFIGURE_DEPENDS
        *.cpp *.h)

add_library(libred2pack-chils ${RED2PACK_SOURCE_FILES})
target_include_directories(libred2pack-chils PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../")
target_link_libraries(libred2pack-chils PUBLIC CHILS::CHILS libred2pack)

if (OpenMP_FOUND)
  target_link_libraries(libred2pack-chils PUBLIC OpenMP::OpenMP_CXX)
endif ()

target_include_directories(libred2pack-chils PRIVATE ${KAMIS_WMIS_KAHIP_INCLUDES})
target_link_libraries(libred2pack-chils PRIVATE kamis::wmis)

add_library(libred2pack-chils-interface INTERFACE)
target_include_directories(libred2pack-chils-interface INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/../")

add_library(red2pack::chils::interface ALIAS libred2pack-chils-interface)
add_library(red2pack::chils::lib ALIAS libred2pack-chils)


