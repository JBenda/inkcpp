project(inkcpp_py LANGUAGES CXX)
add_subdirectory(pybind11)
pybind11_add_module(inkcpp_py module.cpp)
target_compile_definitions(inkcpp_py PRIVATE VERSION_INFO=${VERSION})

