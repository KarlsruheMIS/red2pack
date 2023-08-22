Include(FetchContent)

FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.4.0 # or a later release
)

FetchContent_MakeAvailable(Catch2)

add_compile_options(-fpermissive)

add_compile_definitions(INSTANCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../graphs/example")

add_executable(test_red2pack_solver test_red2pack_solver.cpp $<TARGET_OBJECTS:libkaffpa2> $<TARGET_OBJECTS:libsources> $<TARGET_OBJECTS:libonlinemis> "$<TARGET_OBJECTS:${RED2PACK_LIB}>" "$<TARGET_OBJECTS:${RED2PACK_LIB}-onlinemis>")
target_link_libraries(test_red2pack_solver PRIVATE Catch2::Catch2WithMain)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(test_red2pack_solver)

