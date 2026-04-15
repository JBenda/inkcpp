list(APPEND SOURCES
    compiler.cpp binary_stream.h binary_stream.cpp json.hpp
    json_compiler.h json_compiler.cpp 
    emitter.h emitter.cpp
    reporter.h reporter.cpp
    binary_emitter.h binary_emitter.cpp
	list_data.h list_data.cpp
    command.cpp
    )
add_definitions(-DINK_EXPOSE_JSON)
add_library(inkcpp_compiler_o OBJECT ${SOURCES})
add_library(inkcpp_compiler $<TARGET_OBJECTS:inkcpp_compiler_o>)

target_include_directories(inkcpp_compiler_o PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
)
target_include_directories(inkcpp_compiler PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
)
FILE(GLOB PUBLIC_HEADERS "include/*")
set_target_properties(inkcpp_compiler PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

target_link_libraries(inkcpp_compiler PUBLIC inkcpp_shared)
target_link_libraries(inkcpp_compiler_o PUBLIC inkcpp_shared)

# Unreal installation
list(REMOVE_ITEM SOURCES "json.hpp")
configure_file("json.hpp" "${CMAKE_BINARY_DIR}/unreal/inkcpp/Source/ThirdParty/Private/json.hpp" COPYONLY)
foreach(INCLUDE_FILE IN LISTS PUBLIC_HEADERS)
	if(NOT IS_DIRECTORY "include/${INCLUDE_FILE}")
		get_filename_component(INCLUDE_FILE "${INCLUDE_FILE}" NAME)
		configure_file("include/${INCLUDE_FILE}" "${CMAKE_BINARY_DIR}/unreal/inkcpp/Source/inkcpp_editor/Private/ink/${INCLUDE_FILE}" COPYONLY)
	endif()
endforeach()
foreach(FILE IN LISTS SOURCES)
	configure_file("${FILE}" "${CMAKE_BINARY_DIR}/unreal/inkcpp/Source/inkcpp_editor/Private/ink" COPYONLY)
endforeach()
