add_library(inkcpp_shared INTERFACE)

target_include_directories(inkcpp_shared
	INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/public>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/private>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/public>
	$<INSTALL_INTERFACE:inkcpp>
)
configure_file("public/config.h.in" "public/config.h" @ONLY)
FILE(GLOB PUBLIC_HEADERS "public/*.h")
set_target_properties(inkcpp_shared PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS};${CMAKE_CURRENT_BINARY_DIR}/public/config.h")

# Unreal installation
file(GLOB UNREAL_SOURCE_PUBLIC "public/*.h")
configure_file("public/config.h.in" "${CMAKE_BINARY_DIR}/unreal/inkcpp/Source/shared/Pulic/config.h" @ONLY)
foreach(SRC_FILE IN LISTS UNREAL_SOURCE_PUBLIC)
	get_filename_component(SRC_FILE "${SRC_FILE}" NAME)
	configure_file("${CMAKE_CURRENT_SOURCE_DIR}/public/${SRC_FILE}" "${CMAKE_BINARY_DIR}/unreal/inkcpp/Source/shared/Public/${SRC_FILE}" COPYONLY)
endforeach()
file(GLOB UNREAL_SOURCE_PRIVATE "private/*.h")
foreach(SRC_FILE IN LISTS UNREAL_SOURCE_PRIVATE)
	get_filename_component(SRC_FILE "${SRC_FILE}" NAME)
	configure_file("${CMAKE_CURRENT_SOURCE_DIR}/private/${SRC_FILE}" "${CMAKE_BINARY_DIR}/unreal//inkcpp/Source/shared/Private/${SRC_FILE}" COPYONLY)
endforeach()
