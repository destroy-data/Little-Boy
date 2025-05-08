file(GLOB CORE_TESTS
    test_*.cpp
)

add_executable(test_core
    ${CMAKE_SOURCE_DIR}/test/test_main.cpp
    ${CMAKE_SOURCE_DIR}/test/ppu_helper.cpp
    ${CMAKE_SOURCE_DIR}/test/desktop_logging.cpp
    ${CORE_TESTS}
)
add_strict_warnings(test_core)

target_link_libraries(test_core
    PRIVATE
    Catch2::Catch2WithMain
    gb_core
)

catch_discover_tests(test_core)