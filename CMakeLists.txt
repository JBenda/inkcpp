cmake_minimum_required(VERSION 3.16)

# Testing enabled
enable_testing()

# Project setup
project(inkcpp VERSION 0.1)
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
# Add subdirectories
add_subdirectory(shared)
add_subdirectory(inkcpp)
add_subdirectory(inkcpp_compiler)
add_subdirectory(inkcpp_cl)
add_subdirectory(inkcpp_test)
add_subdirectory(unreal)
