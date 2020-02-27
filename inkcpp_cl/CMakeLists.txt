# Create executable
add_executable(inkcpp_cl inkcpp_cl.cpp test.h test.cpp)

# Include compiler and runtime libraries
target_link_libraries(inkcpp_cl PUBLIC inkcpp inkcpp_compiler)

# Install
install(TARGETS inkcpp_cl DESTINATION bin)