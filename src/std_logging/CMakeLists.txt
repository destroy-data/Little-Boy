file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_library(std_logging STATIC ${SOURCES})
add_strict_warnings(std_logging)
