file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_library(rom_cartridge STATIC ${SOURCES})
add_strict_warnings(std_logging)
