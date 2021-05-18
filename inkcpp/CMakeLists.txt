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
	string_operations.cpp
	numeric_operations.cpp
	header.cpp
)
source_group(Collections REGULAR_EXPRESSION collections/.*)
add_library(inkcpp ${SOURCES})

# Make sure the include directory is included 
target_include_directories(inkcpp 
    PUBLIC include
    PUBLIC ../shared/public
    PRIVATE ../shared/private
    )

# Make sure this project and all dependencies use the C++17 standard
target_compile_features(inkcpp PUBLIC cxx_std_17)

# Default installation
install(TARGETS inkcpp DESTINATION lib)

# Unreal installation
install(DIRECTORY "include/" DESTINATION "inkcpp/Source/inkcpp/Public/ink/" COMPONENT unreal)
install(FILES ${SOURCES} DESTINATION "inkcpp/Source/inkcpp/Private/ink/" COMPONENT unreal)
