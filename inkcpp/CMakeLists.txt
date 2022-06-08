set_property(GLOBAL PROPERTY USE_FOLDERS ON)

list(APPEND SOURCES 
    collections/restorable.h 
    collections/restorable.cpp
    array.h
    choice.cpp
    functional.cpp
    functions.h functions.cpp    
    globals_impl.h globals_impl.cpp
    output.h output.cpp
    platform.h
    runner_impl.h runner_impl.cpp
    simple_restorable_stack.h stack.h stack.cpp
    story_impl.h story_impl.cpp
    story_ptr.cpp
    system.cpp
    value.h value.cpp
    string_table.h string_table.cpp avl_array.h
	list_table.h list_table.cpp
	list_operations.h list_operations.cpp
	container_operations.h container_operations.cpp
	string_operations.cpp
	numeric_operations.cpp
	header.cpp
)
source_group(Collections REGULAR_EXPRESSION collections/.*)
add_library(inkcpp ${SOURCES})
FILE(GLOB PUB_HEADER "include/*")
target_sources(inkcpp
	PUBLIC FILE_SET HEADERS BASE_DIRS include FILES ${PUB_HEADER}
)

# Make sure the include directory is included 
target_link_libraries(inkcpp inkcpp_shared)
# Make sure this project and all dependencies use the C++17 standard
target_compile_features(inkcpp PUBLIC cxx_std_17)

# Default installation
install(TARGETS inkcpp
	FILE_SET HEADERS
	DESTINATION "${CMAKE_INSTALL_INCLUDE_DIR}/inkcpp"
	COMPONENT lib EXCLUDE_FROM_ALL)
install(TARGETS inkcpp
	DESTINATION "${CMAKE_INSTALL_LIBRARY_DIR}/inkcpp"
	COMPONENT lib EXCLUDE_FROM_ALL
)

# Unreal installation
install(DIRECTORY "include/" DESTINATION "inkcpp/Source/inkcpp/Public/ink/" COMPONENT unreal EXCLUDE_FROM_ALL)
install(FILES ${SOURCES} DESTINATION "inkcpp/Source/inkcpp/Private/ink/" COMPONENT unreal EXCLUDE_FROM_ALL)
