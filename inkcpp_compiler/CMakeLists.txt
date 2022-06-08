cmake_minimum_required(VERSION 3.23.2)

list(APPEND SOURCES
    compiler.cpp binary_stream.h binary_stream.cpp json.hpp
    json_compiler.h json_compiler.cpp 
    emitter.h emitter.cpp
    reporter.h reporter.cpp
    text_emitter.h text_emitter.cpp
    binary_emitter.h binary_emitter.cpp
	list_data.h list_data.cpp
    command.cpp
    )
add_definitions(-DINK_COMPILER -DINK_EXPOSE_JSON)
add_library(inkcpp_compiler ${SOURCES})
FILE(GLOB PUB_HEADER "include/*")
target_sources(inkcpp_compiler
	PUBLIC FILE_SET HEADERS BASE_DIRS include FILES ${PUB_HEADER}
)

target_link_libraries(inkcpp_compiler inkcpp_shared)

# Make sure this project and all dependencies use the C++17 standard
target_compile_features(inkcpp_compiler PUBLIC cxx_std_17)

# Default installation
install(TARGETS inkcpp_compiler
	FILE_SET HEADERS
	DESTINATION "${CMAKE_INSTALL_INCLUDE_DIR}/inkcpp/compiler"
	COMPONENT lib EXCLUDE_FROM_ALL)
install(TARGETS inkcpp_compiler
	DESTINATION "${CMAKE_INSTALL_LIBRARY_DIR}/inkcpp"
	COMPONENT lib EXCLUDE_FROM_ALL
)

# Unreal installation
install(DIRECTORY "include/" DESTINATION "inkcpp/Source/inkcpp_editor/Private/ink/" COMPONENT unreal)
install(FILES ${SOURCES} DESTINATION "inkcpp/Source/inkcpp_editor/Private/ink/" COMPONENT unreal)
