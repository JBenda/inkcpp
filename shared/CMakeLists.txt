add_library(inkcpp_shared INTERFACE)

target_include_directories(
	inkcpp_shared
	INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/public>
						$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/private> $<INSTALL_INTERFACE:include>)
file(GLOB PUBLIC_HEADERS "public/*")
set_target_properties(inkcpp_shared PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

# Unreal installation
file(GLOB UNREAL_SOURCE_PUBLIC "public/*")
foreach(src_file IN LISTS UNREAL_SOURCE_PUBLIC)
	get_filename_component(src_file "${SRC_FILE}" NAME)
	configure_file("${CMAKE_CURRENT_SOURCE_DIR}/public/${src_file}"
								 "${CMAKE_BINARY_DIR}/unreal/inkcpp/Source/shared/Public/${src_file}" COPYONLY)
endforeach()
file(GLOB UNREAL_SOURCE_PRIVATE "private/*")
foreach(src_file IN LISTS UNREAL_SOURCE_PRIVATE)
	get_filename_component(src_file "${SRC_FILE}" NAME)
	configure_file("${CMAKE_CURRENT_SOURCE_DIR}/private/${src_file}"
								 "${CMAKE_BINARY_DIR}/unreal//inkcpp/Source/shared/Private/${src_file}" COPYONLY)
endforeach()
