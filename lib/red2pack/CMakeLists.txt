file(GLOB_RECURSE RED2PACK_SOURCE_FILES CONFIGURE_DEPENDS
        *.cpp *.h)

add_library(libred2pack ${RED2PACK_SOURCE_FILES})
target_include_directories(libred2pack PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../")

# our core library uses KaMIS/wmis and KaMIS/wmis/extern/KaHIP in its implementations
# only expose header files of KaMIS/wmis (not KaHIP)
target_include_directories(libred2pack PRIVATE ${KAMIS_WMIS_KAHIP_INCLUDES})
target_link_libraries(libred2pack PRIVATE kamis::wmis)

add_library(libred2pack-interface INTERFACE)
target_include_directories(libred2pack-interface INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/../")

add_library(red2pack::interface ALIAS libred2pack-interface)
add_library(red2pack::lib ALIAS libred2pack)
