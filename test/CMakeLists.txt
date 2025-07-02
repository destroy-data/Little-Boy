include(FetchContent)

find_package(Catch2 3.0 CONFIG)

if(NOT Catch2_FOUND)
    message(STATUS "Catch2 not found, fetching from source.")
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.7.1
        GIT_SHALLOW TRUE
        GIT_PROGRESS FALSE
    )
    FetchContent_MakeAvailable(Catch2)
else()
    message(STATUS "Found Catch2 ${Catch2_VERSION}.")
endif()

list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
include_directories(${CMAKE_SOURCE_DIR}/test)

add_subdirectory(${CMAKE_SOURCE_DIR}/test/core)
if(EMULATOR_TARGET STREQUAL "raylib")
    # add_subdirectory(${CMAKE_SOURCE_DIR}/test/cartridge_impls)
    add_subdirectory(${CMAKE_SOURCE_DIR}/test/raylib)
elseif(EMULATOR_TARGET STREQUAL "terminal")
    # TODO
elseif(EMULATOR_TARGET STREQUAL "cortex-m")
    # TODO
endif()

add_custom_target(
    automatic_test
    COMMAND ${CMAKE_CTEST_COMMAND} -LE "interactive"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running all automated tests"
)
