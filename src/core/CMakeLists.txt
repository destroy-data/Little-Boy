file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_library(gb_core STATIC ${SOURCES}
        emulator.cpp)
add_strict_warnings(gb_core)
