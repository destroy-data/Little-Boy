file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_library(cartridge_impls STATIC ${SOURCES})
target_link_libraries(cartridge_impls PUBLIC gb_core)
add_strict_warnings(cartridge_impls)
