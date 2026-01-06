# Create executable
if(INKCPP_NO_STL)
	message(WARNING "Will not build CL because")
	return()
endif()
add_executable(inkcpp_cl inkcpp_cl.cpp test.h test.cpp)

# Include compiler and runtime libraries
target_link_libraries(inkcpp_cl PUBLIC inkcpp inkcpp_compiler inkcpp_shared)

# For https://en.cppreference.com/w/cpp/filesystem#Notes
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.1")
		target_link_libraries(inkcpp_cl PRIVATE stdc++fs)
	endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
		target_link_libraries(inkcpp_cl PRIVATE stdc++fs)
	endif()
endif()

# Install
install(
	TARGETS inkcpp_cl
	DESTINATION .
	COMPONENT cl
	EXCLUDE_FROM_ALL)
string(TOUPPER "${INKCPP_INKLECATE}" inkcpp_inklecate_upper)
unset(INKLECATE_DIR)
if((inkcpp_inklecate_upper STREQUAL "ALL") OR (inkcpp_inklecate_upper STREQUAL "OS"))
	if(UNIX AND NOT APPLE)
		FetchContent_GetProperties(inklecate_linux)
		if(inklecate_linux_POPULATED)
			set(INKLECATE_DIR "${inklecate_linux_SOURCE_DIR}")
		else()
			message(WARNING "Inklecate for this OS was not downloaded successfully, \
					and will therfore not be packat with cl")
		endif(inklecate_linux_POPULATED)
	elseif(APPLE)
		if(inklecate_mac_POPULATED)
			set(INKLECATE_DIR "${inklecate_mac_SOURCE_DIR}")
		else()
			message(WARNING "Inklecate for this OS was not downloaded successfully, \
					and will therfore not be packat with cl")
		endif(inklecate_mac_POPULATED)
	elseif(
		MSYS
		OR MINGW
		OR WIN32
		OR CYGWIN)
		if(inklecate_windows_POPULATED)
			set(INKLECATE_DIR "${inklecate_windows_SOURCE_DIR}")
		else()
			message(WARNING "Inklecate for this OS was not downloaded successfully, \
					and will therfore not be packat with cl")
		endif(inklecate_windows_POPULATED)
	else()
		message(WARNING "Failed to determine OS for bundling inklecate with command line application!")
	endif()
	if(INKLECATE_DIR)
		file(GLOB inklecate_files "${INKLECATE_DIR}/*")
		install(
			FILES ${inklecate_files}
			DESTINATION .
			COMPONENT cl
			EXCLUDE_FROM_ALL)
	endif(INKLECATE_DIR)
endif((inkcpp_inklecate_upper STREQUAL "ALL") OR (inkcpp_inklecate_upper STREQUAL "OS"))
