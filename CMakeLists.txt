cmake_minimum_required(VERSION 3.25)
project(gameboy C CXX)

function(add_strict_warnings target)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(
            ${target}
            PRIVATE
                -Wall
                -Wextra
                -Wpedantic
                -Wconversion
                "$<$<NOT:$<CONFIG:Debug>>:-Werror>"
        )
    elseif(MSVC)
        target_compile_options(
            ${target}
            PRIVATE
                /W4
                /w14928 # Covers some -Wextra warnings (illegal copy-initialization)
                /w14265 # Covers some -Wextra warnings (class with virtual functions but non-virtual destructor)
                /permissive- # Standards compliance (-Wpedantic)
                /w14242 # Conversion warnings (-Wconversion)
                /w14244 # Conversion warnings (-Wconversion)
                /w14254 # Conversion warnings (-Wconversion)
                /w14267 # Conversion warnings (-Wconversion)
                "$<$<NOT:$<CONFIG:Debug>>:/WX>"
        )
    endif()
endfunction()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_definitions(your_target PRIVATE RELEASE)
endif()

include(options.cmake)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(${CMAKE_SOURCE_DIR}/test)
endif()
