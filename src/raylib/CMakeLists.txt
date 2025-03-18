file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_executable(raylib_frontend ${SOURCES})

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
target_link_libraries(raylib_frontend raylib)
target_link_libraries(raylib_frontend gb_core)
