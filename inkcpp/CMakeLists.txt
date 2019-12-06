add_library(inkcpp 
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
    )

# Make sure the include directory is included 
target_include_directories(inkcpp 
    PUBLIC ../include
    PRIVATE ../common
    )

# Make sure this project and all dependencies use the C++17 standard
target_compile_features(inkcpp PUBLIC cxx_std_17)