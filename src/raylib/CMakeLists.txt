include(FetchContent)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

find_package(raylib 5.0 REQUIRED CONFIG)

if(NOT raylib_FOUND)
    message(STATUS "Raylib not found, fetching from source.")
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
        GIT_TAG "master"
        GIT_SHALLOW TRUE
        GIT_PROGRESS FALSE
    )
    FetchContent_MakeAvailable(raylib)
else()
    message(STATUS "Found raylib ${raylib_VERSION}")
endif()

FetchContent_Declare(
    tinyfiledialogs
    URL
        "https://sourceforge.net/projects/tinyfiledialogs/files/tinyfiledialogs-current.zip/download"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(tinyfiledialogs)

#--------------------------------------------------
add_library(
    tinyfiledialogs
    STATIC
    ${tinyfiledialogs_SOURCE_DIR}/tinyfiledialogs.c
    ${tinyfiledialogs_SOURCE_DIR}/tinyfiledialogs.h
)
target_include_directories(tinyfiledialogs PUBLIC ${tinyfiledialogs_SOURCE_DIR})

file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
list(FILTER SOURCES EXCLUDE REGEX ".*main\\.cpp$")
add_library(gb_raylib STATIC ${SOURCES})
add_strict_warnings(gb_raylib)

target_link_libraries(
    gb_raylib
    PUBLIC raylib gb_core cartridge_impls tinyfiledialogs
)

add_executable(gb_raylib_executable ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
target_link_libraries(gb_raylib_executable PRIVATE gb_raylib)
