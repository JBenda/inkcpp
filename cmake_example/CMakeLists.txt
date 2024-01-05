cmake_minimum_required(VERSION 3.16)
project(main)

find_package(inkcpp CONFIG REQUIRED)

add_executable(main main.cpp)
set_property(TARGET main PROPERTY CXX_STANDARD 17)
target_link_libraries(main inkcpp inkcpp_compiler)
