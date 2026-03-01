file(GLOB_RECURSE RED2PACK_SOURCE_FILES CONFIGURE_DEPENDS
        *.cpp *.h)

add_library(libred2pack-mmwis ${RED2PACK_SOURCE_FILES})
target_include_directories(libred2pack-mmwis PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../")
#target_link_libraries(libred2pack-mmwis PUBLIC mmwis::interface)
target_link_libraries(libred2pack-mmwis PUBLIC mmwis::lib libred2pack mmwis::interface)

add_library(libred2pack-mmwis-interface INTERFACE)
target_include_directories(libred2pack-mmwis-interface INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/../")

add_library(red2pack::mmwis::interface ALIAS libred2pack-mmwis-interface)
add_library(red2pack::mmwis::lib ALIAS libred2pack-mmwis)