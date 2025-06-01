file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
list(FILTER SOURCES EXCLUDE REGEX ".*main\\.cpp$")
add_library(gb_raylib STATIC ${SOURCES})
add_strict_warnings(gb_raylib)

# Adding Raylib
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
    GIT_TAG "master"
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(raylib)
target_link_libraries(gb_raylib PUBLIC
    raylib
    gb_core
    cartridge_impls
)

add_executable(gb_raylib_executable ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp ${gb_raylib})
set_target_properties(gb_raylib_executable PROPERTIES
    OUTPUT_NAME_RELEASE "little-boy"
)
