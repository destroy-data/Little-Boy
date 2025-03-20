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

#One test module per prod module
foreach(MODULE ${MODULES})
    add_subdirectory(${MODULE})
endforeach()

add_custom_target( automatic_test
    COMMAND ${CMAKE_CTEST_COMMAND} -LE "interactive"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running all automated tests"
)
