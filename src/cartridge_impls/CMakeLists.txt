file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_library(cartridge_impls STATIC ${SOURCES})
add_strict_warnings(cartridge_impls)
