file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_library(std_logging STATIC ${SOURCES})
add_strict_warnings(std_logging)

if(MSVC)
	# MSVC requires DbgHelp for stacktrace
	target_link_libraries(std_logging PRIVATE DbgHelp.lib)
else()
	# needed for stacktrace in GCC/Clang
	target_link_libraries(std_logging PRIVATE stdc++exp)

	target_compile_options(std_logging PRIVATE -fno-omit-frame-pointer)
endif()
