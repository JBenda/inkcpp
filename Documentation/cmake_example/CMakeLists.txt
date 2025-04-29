cmake_minimum_required(VERSION 3.16)
project(main)

find_package(inkcpp CONFIG REQUIRED)

# for CXX builds
add_executable(main_cpp main.cpp)
set_property(TARGET main_cpp PROPERTY CXX_STANDARD 17)
target_link_libraries(main_cpp inkcpp inkcpp_compiler)

# for C builds
# add_executable(main_c main.c)
# target_link_libraries(main_c inkcpp_c)
