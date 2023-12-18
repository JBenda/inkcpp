add_subdirectory(pybind11)
add_compile_options(-fPIC)
pybind11_add_module(inkcpp_py module.cpp)
target_compile_definitions(inkcpp_py PRIVATE VERSION_INFO=${VERSION})

target_link_libraries(inkcpp_py PUBLIC inkcpp inkcpp_compiler inkcpp_shared)


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

