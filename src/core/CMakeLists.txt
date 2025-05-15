file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_library(gb_core STATIC ${SOURCES})
target_link_libraries(gb_core PRIVATE std_logging)
add_strict_warnings(gb_core)
