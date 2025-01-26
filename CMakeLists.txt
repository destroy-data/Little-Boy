cmake_minimum_required(VERSION 3.31)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
project(gameboy C CXX)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wconversion -Wsign-conversion -Wno-missing-field-initializers")
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

pico_sdk_init()

#Add implicit include dirs explicitely so they are in compile_commands.json
# include_directories(SYSTEM ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES} ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")
add_executable(gameboy ${SOURCES})

target_include_directories(gameboy PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(gameboy pico_stdlib)
# add_compile_options(--target=arm-none-eabi)
pico_add_extra_outputs(gameboy)
