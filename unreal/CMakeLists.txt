set(INKCPP_UNREAL_TARGET_VERSION "5.6" CACHE STRING "Unreal engine version the plugin should target (e.g: 5.6)")
set(INKCPP_UNREAL_RunUAT_PATH CACHE FILEPATH "Path to Unreal engine installation RunUAT file. Used to automatcally build the plugin.")
option(INKCPP_UNREAL "Prepare sourcefiles for a UE Plugin (this will download " OFF)
	option(INKCPP_DOC_BlueprintUE "Building doxygen documentation with BlueprintUE visualisation for unreal blueprints. (Requires node js)" ON)
set(INKCPP_UNREAL_TARGET_PLATFORM "Win64" CACHE STRING "Target platform for the UE Plugin one of Win64, Mac, Linux")
set_property(CACHE INKCPP_UNREAL_TARGET_PLATFORM PROPERTY STRINGS "Win64" "Mac" "Linux")
if (INKCPP_UNREAL)
	include(FetchContent)
	configure_file(
		"${CMAKE_CURRENT_SOURCE_DIR}/inkcpp/inkcpp.uplugin.in"
		"${CMAKE_CURRENT_BINARY_DIR}/inkcpp/inkcpp.uplugin")
	# download inklecate for unreal plugin
	FetchContent_MakeAvailable(inklecate_mac inklecate_windows inklecate_linux)
	set(FETCHCONTENT_QUIET OFF)
	set(CMAKE_TLS_VERIFY true)
	if(NOT inklecate_windows_SOURCE_DIR)
	message(WARNING "failed to download inklecate for windows, "
		"the unreal plugin will be unable use a .ink file as asset directly")
	else()
	set(INKLECATE_CMD_WIN "Source/ThirdParty/inklecate/windows/inklecate.exe")
	file(COPY "${CMAKE_BINARY_DIR}/inklecate/windows" DESTINATION "inkcpp/Source/ThirdParty/inklecate/")
	endif()
	if(NOT inklecate_mac_SOURCE_DIR)
		message(WARNING "failed to download inklecate for MacOS, "
			"the unreal plugin will be unable use a .ink file as asset directly")
	else()
	set(INKLECATE_CMD_MAC "Source/ThirdParty/inklecate/mac/inklecate")
	file(COPY "${CMAKE_BINARY_DIR}/inklecate/mac" DESTINATION "inkcpp/Source/ThirdParty/inklecate/")
	endif()
	if(NOT inklecate_linux_SOURCE_DIR)
		message(WARNING "failed to download inklecate for linux, "
			"the unreal plugin will be unable use a .ink file as asset directly")
	else()
	set(INKLECATE_CMD_LINUX "Source/ThirdParty/inklecate/linux/inklecate")
	file(COPY "${CMAKE_BINARY_DIR}/inklecate/linux" DESTINATION "inkcpp/Source/ThirdParty/inklecate/")
	endif()
	configure_file(
		"${CMAKE_CURRENT_SOURCE_DIR}/inkcpp/Source/inkcpp_editor/Private/inklecate_cmd.cpp.in"
		"${CMAKE_CURRENT_BINARY_DIR}/inkcpp/Source/inkcpp_editor/Private/inklecate_cmd.cpp"
	)	
	file(GLOB_RECURSE SOURCE_FILES LIST_DIRECTORIES TRUE RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/inkcpp/*")
	list(FILTER SOURCE_FILES EXCLUDE REGEX ".*\.in$")
	foreach(SRC_FILE IN LISTS SOURCE_FILES)
		if (NOT IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${SRC_FILE}")
			configure_file("${CMAKE_CURRENT_SOURCE_DIR}/${SRC_FILE}" "${CMAKE_CURRENT_BINARY_DIR}/${SRC_FILE}" COPYONLY)
		endif()
	endforeach()
	install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/inkcpp/" DESTINATION "inkcpp" COMPONENT unreal EXCLUDE_FROM_ALL)
	file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Plugins/inkcpp/")
	if ((NOT DEFINED INKCPP_UNREAL_RunUAT_PATH) OR (NOT "${INKCPP_UNREAL_RunUAT_PATH}" STREQUAL ""))
		if (NOT IS_READABLE "${INKCPP_UNREAL_RunUAT_PATH}")
			message(WARNING  "Unable to find RunUAT script at >${INKCPP_UNREAL_RunUAT_PATH}<, will not be able to build target `unreal` set the filepath with the variable INKCPP_UNREAL_RunUAT_PATH")
		endif()
	else()
		message(WARNING, "To directly build the plugin with `cmake --build . --target unreal` please set INKCPP_UNREAL_RunUAT_PATH to point to unreals RunUAT script.")
	endif()

	add_custom_target(unreal 
		"${INKCPP_UNREAL_RunUAT_PATH}" BuildPlugin "-plugin=${CMAKE_CURRENT_BINARY_DIR}/inkcpp/inkcpp.uplugin" "-package=${CMAKE_CURRENT_BINARY_DIR}/Plugins/inkcpp" "-TargetPlatforms=${INKCPP_UNREAL_TARGET_PLATFORM}"
		COMMENT "Compile UE Plugin.")
	install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Plugins/inkcpp/" DESTINATION "inkcpp" COMPONENT unreal_plugin EXCLUDE_FROM_ALL PATTERN "Intermediate/*" EXCLUDE)
	# TODO: update documenation
endif()

