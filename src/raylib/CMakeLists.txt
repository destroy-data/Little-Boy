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

FetchContent_Declare(
    tinyfiledialogs
    URL "https://sourceforge.net/projects/tinyfiledialogs/files/tinyfiledialogs-current.zip/download"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(tinyfiledialogs)
FetchContent_MakeAvailable(raylib)

add_library(tinyfiledialogs STATIC
    ${tinyfiledialogs_SOURCE_DIR}/tinyfiledialogs.c
    ${tinyfiledialogs_SOURCE_DIR}/tinyfiledialogs.h
)
target_include_directories(tinyfiledialogs PUBLIC ${tinyfiledialogs_SOURCE_DIR})

target_link_libraries(gb_raylib PUBLIC
    raylib
    gb_core
    cartridge_impls
    tinyfiledialogs
)

add_executable(gb_raylib_executable ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
target_link_libraries(gb_raylib_executable PRIVATE gb_raylib)
set_target_properties(gb_raylib_executable PROPERTIES
    OUTPUT_NAME_RELEASE "little-boy"
)
