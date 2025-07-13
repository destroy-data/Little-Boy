include(FetchContent)
find_package(nlohmann_json 3.11 CONFIG)

if(NOT nlohmann_json_FOUND)
    message(STATUS "nlohmann_json not found, fetching from source.")
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY "https://github.com/nlohmann/json.git"
        GIT_TAG "v3.11.3"
        GIT_SHALLOW TRUE
        GIT_PROGRESS FALSE
    )
    FetchContent_MakeAvailable(nlohmann_json)
else()
    message(STATUS "Found nlohmann_json ${nlohmann_json_VERSION}")
endif()

FetchContent_Declare(
    opcode_json_tests
    GIT_REPOSITORY "https://github.com/SingleStepTests/sm83.git"
    GIT_TAG "main"
    GIT_SHALLOW TRUE
    GIT_PROGRESS FALSE
)
FetchContent_MakeAvailable(opcode_json_tests)
#-------------------------------------------------

file(GLOB CORE_TESTS test_*.cpp)

add_executable(test_core ${CMAKE_SOURCE_DIR}/test/ppu_helper.cpp ${CORE_TESTS})
add_strict_warnings(test_core)

target_compile_definitions(
    test_core
    PRIVATE OPCODE_TESTS_PATH="${opcode_json_tests_SOURCE_DIR}/v1"
)

target_link_libraries(
    test_core
    PRIVATE Catch2::Catch2WithMain gb_core nlohmann_json::nlohmann_json
)

catch_discover_tests(test_core)
