cmake_minimum_required(VERSION 3.28.3)
project(gameboy C CXX)

function(add_strict_warnings target)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wconversion
            -Werror
        )
    elseif(MSVC)
        target_compile_options(${target} PRIVATE
            /W4
            /w14928  # Covers some -Wextra warnings (illegal copy-initialization)
            /w14265  # Covers some -Wextra warnings (class with virtual functions but non-virtual destructor)
            /permissive- # Standards compliance (-Wpedantic)
            /w14242  # Conversion warnings (-Wconversion)
            /w14244  # Conversion warnings (-Wconversion)
            /w14254  # Conversion warnings (-Wconversion)
            /w14267  # Conversion warnings (-Wconversion)
        )
    endif()
endfunction()

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(MODULES
    core
    raylib
)

# Add -Werror for all configurations except Debug
add_compile_options("$<$<NOT:$<CONFIG:Debug>>:-Werror>")
include_directories(${CMAKE_SOURCE_DIR}/include)

foreach(MODULE ${MODULES})
    add_subdirectory(${CMAKE_SOURCE_DIR}/src/${MODULE})
endforeach()

enable_testing()
add_subdirectory(${CMAKE_SOURCE_DIR}/test)
