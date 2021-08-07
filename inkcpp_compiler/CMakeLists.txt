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

# Make sure the include directory is included 
target_include_directories(inkcpp_compiler
    PUBLIC include
    PUBLIC ../shared/public
    PRIVATE ../shared/private
    )

# Make sure this project and all dependencies use the C++17 standard
target_compile_features(inkcpp_compiler PUBLIC cxx_std_17)

# Default installation
install(TARGETS inkcpp_compiler DESTINATION lib)

# Unreal installation
install(DIRECTORY "include/" DESTINATION "inkcpp/Source/inkcpp_editor/Private/ink/" COMPONENT unreal)
install(FILES ${SOURCES} DESTINATION "inkcpp/Source/inkcpp_editor/Private/ink/" COMPONENT unreal)
