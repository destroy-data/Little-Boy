Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.7.1
  GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(Catch2)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
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

add_custom_target( automatic_test
    COMMAND ${CMAKE_CTEST_COMMAND} -LE "interactive"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running all automated tests"
)
