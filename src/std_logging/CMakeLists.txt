file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_library(std_logging STATIC ${SOURCES})
add_strict_warnings(std_logging)

if(MSVC)
	# MSVC requires DbgHelp for stacktrace
	target_link_libraries(std_logging PRIVATE DbgHelp.lib)
	set_source_files_properties(
		"${CMAKE_CURRENT_SOURCE_DIR}/logging.cpp"
		PROPERTIES COMPILE_FLAGS "/wd4101"
	)
else()
	# needed for stacktrace in GCC/Clang
	target_link_libraries(std_logging PRIVATE stdc++exp)
	# for better stacktraces
	target_compile_options(std_logging PRIVATE -fno-omit-frame-pointer)

	set_source_files_properties(
		"${CMAKE_CURRENT_SOURCE_DIR}/logging.cpp"
		PROPERTIES COMPILE_FLAGS "-Wno-unused-variable"
	)
endif()
