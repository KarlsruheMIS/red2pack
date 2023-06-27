Include(FetchContent)

FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v2.13.9 # or a later release
)

FetchContent_MakeAvailable(Catch2)

add_executable(test_optimality test_optimality.cpp $<TARGET_OBJECTS:libkaffpa2> $<TARGET_OBJECTS:libsources> $<TARGET_OBJECTS:two_packing_set_lib>)
target_link_libraries(test_optimality Catch2::Catch2)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/contrib)
include(CTest)
include(Catch)
catch_discover_tests(test_optimality)