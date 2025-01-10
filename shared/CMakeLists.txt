add_library(inkcpp_shared INTERFACE)

target_include_directories(inkcpp_shared
	INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/public>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/private>
	$<INSTALL_INTERFACE:inkcpp>
)
FILE(GLOB PUBLIC_HEADERS "public/*")
set_target_properties(inkcpp_shared PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

# Unreal installation
file(GLOB_RECURSE UNREAL_SOURCE_PUBLIC LIST_DIRECTORIES TRUE RELATIVE "public/" "public/*")
foreach(SRC_FILE IN LISTS UNREAL_SOURCE_PUBLIC)
	if (NOT IS_DIRECTORY "public/${SRC_FILE}")
		configure_file("${CMAKE_CURRENT_SOURCE_DIR}/public/${SRC_FILE}" "${CMAKE_CURRENT_BINARY_DIR}/../unreal/inkcpp/Source/shared/Public/${SRC_FILE}" COPYONLY)
	endif()
endforeach()
file(GLOB_RECURSE UNREAL_SOURCE_PRIVATE LIST_DIRECTORIES TRUE RELATIVE "private/" "private/*")
foreach(SRC_FILE IN LISTS UNREAL_SOURCE_PRIVATE)
	if (NOT IS_DIRECTORY "public/${SRC_FILE}")
		configure_file("${CMAKE_CURRENT_SOURCE_DIR}/private/${SRC_FILE}" "${CMAKE_CURRENT_BINARY_DIR}/../unreal//inkcpp/Source/shared/Private/${SRC_FILE}" COPYONLY)
	endif()
endforeach()
