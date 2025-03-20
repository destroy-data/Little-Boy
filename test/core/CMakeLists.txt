file(GLOB CORE_TESTS
    test_*.cpp
)

add_executable(test_core
    ${CMAKE_SOURCE_DIR}/test/test_main.cpp
    ${CORE_TESTS}
    ${HELPER_FILE}
)

target_link_libraries(test_core
    PRIVATE
    Catch2::Catch2WithMain
    gb_core
)

catch_discover_tests(test_core)
