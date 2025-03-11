Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.7.1
)

FetchContent_MakeAvailable(Catch2)

include_directories(${CMAKE_SOURCE_DIR}/include)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)

#One test module for prod module
foreach(module ${MODULES})
    file(GLOB MODULE_TESTS
        ${CMAKE_CURRENT_SOURCE_DIR}/${module}/test_*.cpp
    )
    add_executable(test_${module}
        test_main.cpp
        ${MODULE_TESTS}
    )

    target_link_libraries(test_${module}
        PRIVATE
        Catch2::Catch2WithMain
        gb_core
    )
    catch_discover_tests(test_${module})
endforeach()
