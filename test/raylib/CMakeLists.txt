file(GLOB RAYLIB_TESTS
    test_*.cpp
)

add_executable(test_raylib
    ${CMAKE_SOURCE_DIR}/test/test_main.cpp
    ${RAYLIB_TESTS}
    ${HELPER_FILE}
)

target_link_libraries(test_raylib
    PRIVATE
    Catch2::Catch2WithMain
    gb_core
    gb_raylib
    raylib
)

catch_discover_tests(test_raylib)

file(GLOB RAYLIB_INTERACTIVE_TESTS
    itest_*.cpp
)

foreach(itest ${RAYLIB_INTERACTIVE_TESTS})
    get_filename_component(filename_we ${itest} NAME_WE)
    string(REPLACE "itest_" "" test_name ${filename_we})

    add_executable(itest_raylib_${test_name}
        ${CMAKE_SOURCE_DIR}/test/test_main.cpp
        ${itest}
        ${HELPER_FILE}
    )

    target_link_libraries(itest_raylib_${test_name}
        PRIVATE
        Catch2::Catch2WithMain
        gb_core
        gb_raylib
        raylib
    )

    catch_discover_tests(itest_raylib_${test_name} PROPERTIES LABELS "interactive")
endforeach()
