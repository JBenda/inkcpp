set_property(GLOBAL PROPERTY USE_FOLDERS ON)

list(APPEND SOURCES 
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
	snapshot_impl.h snapshot_impl.cpp snapshot_interface.h
    story_ptr.cpp
    system.cpp
    value.h value.cpp
	tuple.hpp
    string_table.h string_table.cpp avl_array.h
	list_table.h list_table.cpp
  list_impl.h list_impl.cpp
	operations.h operation_bases.h
	list_operations.h list_operations.cpp
	container_operations.h container_operations.cpp
	numeric_operations.h numeric_operations.cpp
	string_operations.h string_operations.cpp
	string_operations.cpp
	numeric_operations.cpp
	casting.h
	executioner.h
	string_utils.h
	header.cpp
	random.h
)
list(APPEND COLLECTION_SOURCES
    collections/restorable.h 
    collections/restorable.cpp
)
source_group(Collections REGULAR_EXPRESSION collections/.*)
add_library(inkcpp ${SOURCES} ${COLLECTION_SOURCES})
target_include_directories(inkcpp PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
)
FILE(GLOB PUBLIC_HEADERS "include/*")
set_target_properties(inkcpp PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

# Make sure the include directory is included 
target_link_libraries(inkcpp PRIVATE inkcpp_shared)
# Make sure this project and all dependencies use the C++17 standard
target_compile_features(inkcpp PUBLIC cxx_std_17)


# Unreal installation
install(DIRECTORY "include/" DESTINATION "inkcpp/Source/inkcpp/Public/ink/" COMPONENT unreal EXCLUDE_FROM_ALL)
install(FILES ${SOURCES} DESTINATION "inkcpp/Source/inkcpp/Private/ink/" COMPONENT unreal EXCLUDE_FROM_ALL)
install(FILES ${COLLECTION_SOURCES} DESTINATION "inkcpp/Source/inkcpp/Private/ink/collections/" COMPONENT unreal EXCLUDE_FROM_ALL)
