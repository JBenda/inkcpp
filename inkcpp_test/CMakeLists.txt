add_executable(inkcpp_test catch.hpp Main.cpp 
    Array.cpp 
    Pointer.cpp 
    Stack.cpp
    Callstack.cpp
    Restorable.cpp
    Value.cpp
    Globals.cpp
    Tags.cpp
    )

target_link_libraries(inkcpp_test PUBLIC inkcpp inkcpp_compiler)

add_test(NAME UnitTests COMMAND $<TARGET_FILE:inkcpp_test>)
set (source "${CMAKE_CURRENT_SOURCE_DIR}/ink")
set (destination "${CMAKE_CURRENT_BINARY_DIR}/ink")
add_custom_command(
    TARGET inkcpp_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${source} ${destination}
    DEPENDS ${destination}
    COMMENT "symbolic link resources folder from ${source} => ${destination}"
)
