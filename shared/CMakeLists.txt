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
file(GLOB UNREAL_SOURCE_PUBLIC "public/*")
foreach(SRC_FILE IN LISTS UNREAL_SOURCE_PUBLIC)
	get_filename_component(SRC_FILE "${SRC_FILE}" NAME)
	configure_file("${CMAKE_CURRENT_SOURCE_DIR}/public/${SRC_FILE}" "${CMAKE_BINARY_DIR}/unreal/inkcpp/Source/shared/Public/${SRC_FILE}" COPYONLY)
endforeach()
file(GLOB UNREAL_SOURCE_PRIVATE "private/*")
foreach(SRC_FILE IN LISTS UNREAL_SOURCE_PRIVATE)
	get_filename_component(SRC_FILE "${SRC_FILE}" NAME)
	configure_file("${CMAKE_CURRENT_SOURCE_DIR}/private/${SRC_FILE}" "${CMAKE_BINARY_DIR}/unreal//inkcpp/Source/shared/Private/${SRC_FILE}" COPYONLY)
endforeach()
