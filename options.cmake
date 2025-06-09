# Bolleans
option(BUILD_TESTS "Build the test suite" ON)

# --------------------------------------------------
# Standard options
set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

#--------------------------------------------------
set(VALID_TARGETS "raylib" "terminal" "cortex-m")
string(REPLACE ";" ", " FORMATTED_TARGETS "${VALID_TARGETS}")
set(EMULATOR_TARGET
    "DEFINE_ME_SENPAI"
    CACHE STRING
    "Choose emulator target type"
)
set_property(CACHE EMULATOR_TARGET PROPERTY STRINGS ${VALID_TARGETS})

if(EMULATOR_TARGET STREQUAL "DEFINE_ME_SENPAI")
    message(
        FATAL_ERROR
        "Specify emulator target!\n"
        "Valid targets: ${FORMATTED_TARGETS}.\n"
        "Use -D EMULATOR_TARGET=<target>"
    )
elseif(NOT EMULATOR_TARGET IN_LIST VALID_TARGETS)
    message(
        FATAL_ERROR
        "Invalid emulator target!\n"
        "Valid targets: ${FORMATTED_TARGETS}\n"
        "Use -D EMULATOR_TARGET=<target>"
    )
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/core)
if(EMULATOR_TARGET STREQUAL "raylib")
    add_subdirectory(${CMAKE_SOURCE_DIR}/src/std_logging)
    add_subdirectory(${CMAKE_SOURCE_DIR}/src/cartridge_impls)
    add_subdirectory(${CMAKE_SOURCE_DIR}/src/raylib)
elseif(EMULATOR_TARGET STREQUAL "terminal")
    message(FATAL_ERROR "Sorry, implementation not even started yet.")
elseif(EMULATOR_TARGET STREQUAL "cortex-m")
    message(FATAL_ERROR "Sorry, implementation not even started yet.")
endif()

#--------------------------------------------------
