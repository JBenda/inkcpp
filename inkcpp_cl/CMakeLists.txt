# Create executable
add_executable(inkcpp_cl inkcpp_cl.cpp test.h test.cpp)

# Include compiler and runtime libraries
target_link_libraries(inkcpp_cl PUBLIC inkcpp inkcpp_compiler)

# For https://en.cppreference.com/w/cpp/filesystem#Notes
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.1")
    target_link_libraries(inkcpp_cl PRIVATE stdc++fs)
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
    target_link_libraries(inkcpp_cl PRIVATE stdc++fs)
  endif()
endif()

# Install
install(TARGETS inkcpp_cl DESTINATION . COMPONENT cl EXCLUDE_FROM_ALL)
