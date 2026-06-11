set(INKCPP_UNREAL_TARGET_VERSION
		"5.7"
		CACHE STRING "Unreal engine version the plugin should target (e.g: 5.6)")
set(INKCPP_UNREAL_RunUAT_PATH
		CACHE FILEPATH
					"Path to Unreal engine installation RunUAT file. Used to automatcally build the plugin.")
option(INKCPP_UNREAL "Prepare sourcefiles for a UE Plugin (this will download " OFF)
option(INKCPP_DOC_BlueprintUE "Building doxygen documentation with BlueprintUE visualisation \
			 for unreal blueprints. (Requires node js)" ON)
set(INKCPP_UNREAL_TARGET_PLATFORM
		"Win64"
		CACHE STRING "Target platform for the UE Plugin one of Win64, Mac, Linux")
set_property(CACHE INKCPP_UNREAL_TARGET_PLATFORM PROPERTY STRINGS "Win64" "Mac" "Linux")
if(INKCPP_UNREAL)
	include(FetchContent)
	configure_file("${CMAKE_CURRENT_SOURCE_DIR}/inkcpp/inkcpp.uplugin.in"
								 "${CMAKE_CURRENT_BINARY_DIR}/inkcpp/inkcpp.uplugin")

	# Default all platform paths to empty (= "not bundled"). They are filled in only when the download
	# succeeded.
	set(INKLECATE_CMD_WIN "")
	set(INKLECATE_CMD_MAC "")
	set(INKLECATE_CMD_LINUX "")
	set(INKCPP_INKLECATE_BUNDLED FALSE)

	if(inklecate_windows_SOURCE_DIR)
		set(INKLECATE_CMD_WIN "Source/ThirdParty/inklecate/windows/inklecate.exe")
		file(COPY "${CMAKE_BINARY_DIR}/inklecate/windows"
				 DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/inkcpp/Source/ThirdParty/inklecate/")
		set(INKCPP_INKLECATE_BUNDLED TRUE)
	else()
		file(REMOVE_RECURSE "${CMAKE_CURRENT_BINARY_DIR}/inkcpp/Source/ThirdParty/inklecate/windows")
		message(WARNING "InkCPP: failed to download inklecate for Windows. "
										"A .ink file can still be imported if the user configures the inklecate path "
										"in Project Settings > Plugins > InkCPP.")
	endif()
	if(inklecate_mac_SOURCE_DIR)
		set(INKLECATE_CMD_MAC "Source/ThirdParty/inklecate/mac/inklecate")
		file(COPY "${CMAKE_BINARY_DIR}/inklecate/mac"
				 DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/inkcpp/Source/ThirdParty/inklecate/")
		set(INKCPP_INKLECATE_BUNDLED TRUE)
	else()
		file(REMOVE_RECURSE "${CMAKE_CURRENT_BINARY_DIR}/inkcpp/Source/ThirdParty/inklecate/mac")
		message(WARNING "InkCPP: failed to download inklecate for macOS. "
										"A .ink file can still be imported if the user configures the inklecate path "
										"in Project Settings > Plugins > InkCPP.")
	endif()
	if(inklecate_linux_SOURCE_DIR)
		set(INKLECATE_CMD_LINUX "Source/ThirdParty/inklecate/linux/inklecate")
		file(COPY "${CMAKE_BINARY_DIR}/inklecate/linux"
				 DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/inkcpp/Source/ThirdParty/inklecate/")
		set(INKCPP_INKLECATE_BUNDLED TRUE)
	else()
		file(REMOVE_RECURSE "${CMAKE_CURRENT_BINARY_DIR}/inkcpp/Source/ThirdParty/inklecate/linux")
		message(WARNING "InkCPP: failed to download inklecate for Linux. "
										"A .ink file can still be imported if the user configures the inklecate path "
										"in Project Settings > Plugins > InkCPP.")
	endif()

	file(
		GLOB_RECURSE SOURCE_FILES
		LIST_DIRECTORIES TRUE
		RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
		"${CMAKE_CURRENT_SOURCE_DIR}/inkcpp/*")
	list(FILTER SOURCE_FILES EXCLUDE REGEX ".*\.in$")
	foreach(src_file IN LISTS SOURCE_FILES)
		if(NOT IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${src_file}")
			configure_file("${CMAKE_CURRENT_SOURCE_DIR}/${src_file}"
										 "${CMAKE_CURRENT_BINARY_DIR}/${src_file}" COPYONLY)
		endif()
	endforeach()

	# When at least one inklecate binary was downloaded, regenerate inklecate_cmd.cpp from the .in
	# template so the bundled paths are compiled in.  The runtime code will use those paths as a
	# default and also auto-fill Project Settings on first use. When no binary was downloaded
	# (Fab/offline build) the COPYONLY loop above has already copied the static inklecate_cmd.cpp
	# (which reads only from Project Settings), so nothing extra is needed.
	if(INKCPP_INKLECATE_BUNDLED)
		configure_file(
			"${CMAKE_CURRENT_SOURCE_DIR}/inkcpp/Source/inkcpp_editor/Private/inklecate_cmd.cpp.in"
			"${CMAKE_CURRENT_BINARY_DIR}/inkcpp/Source/inkcpp_editor/Private/inklecate_cmd.cpp")
		message(STATUS "InkCPP: inklecate bundled — generated inklecate_cmd.cpp with embedded paths.")
	else()
		message(STATUS "InkCPP: no inklecate downloaded — using settings-only inklecate_cmd.cpp.")
	endif()

	install(
		DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/inkcpp/"
		DESTINATION "inkcpp"
		COMPONENT unreal
		EXCLUDE_FROM_ALL)
	file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Plugins/inkcpp/")
	if((NOT DEFINED INKCPP_UNREAL_RunUAT_PATH) OR (NOT "${INKCPP_UNREAL_RunUAT_PATH}" STREQUAL ""))
		if(NOT IS_READABLE "${INKCPP_UNREAL_RunUAT_PATH}")
			message(
				WARNING "Unable to find RunUAT script at >${INKCPP_UNREAL_RunUAT_PATH}<, will not be able \
					to build target `unreal` set the filepath with the variable INKCPP_UNREAL_RunUAT_PATH")
		endif()
	else()
		message(WARNING "To directly build the plugin with `cmake --build . --target unreal` please \
						set INKCPP_UNREAL_RunUAT_PATH to point to unreals RunUAT script.")
	endif()

	add_custom_target(
		unreal
		"${INKCPP_UNREAL_RunUAT_PATH}"
		BuildPlugin
		"-plugin=${CMAKE_CURRENT_BINARY_DIR}/inkcpp/inkcpp.uplugin"
		"-package=${CMAKE_CURRENT_BINARY_DIR}/Plugins/inkcpp"
		"-TargetPlatforms=${INKCPP_UNREAL_TARGET_PLATFORM}"
		COMMENT "Compile UE Plugin.")
	install(
		DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Plugins/inkcpp/"
		DESTINATION "inkcpp"
		COMPONENT unreal_plugin
		EXCLUDE_FROM_ALL
		PATTERN "Intermediate/*" EXCLUDE)
	# TODO: update documenation
endif()
