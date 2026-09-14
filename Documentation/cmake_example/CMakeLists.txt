cmake_minimum_required(VERSION 3.16)
project(main C CXX)

find_package(inkcpp CONFIG REQUIRED)

# Set when linking a libc++-built "lib" package against a compiler using a different C++ ABI (e.g.
# GCC on macOS), where the C++ example can never link.
option(INKCPP_EXAMPLE_SKIP_CPP "Skip building the C++ example (main_cpp)" OFF)

# for CXX builds - only available when the "lib" package was installed
if(NOT INKCPP_EXAMPLE_SKIP_CPP
	 AND TARGET inkcpp
	 AND TARGET inkcpp_compiler)
	add_executable(main_cpp main.cpp)
	set_property(TARGET main_cpp PROPERTY CXX_STANDARD 17)
	target_link_libraries(main_cpp PRIVATE inkcpp inkcpp_compiler)
endif()

# for C builds - available from both the "lib" and "clib" packages
if(TARGET inkcpp_c)
	add_executable(main_c main.c)
	set_property(TARGET main_c PROPERTY C_STANDARD 17)
	set_property(TARGET main_c PROPERTY C_STANDARD_REQUIRED ON)
	target_link_libraries(main_c PRIVATE inkcpp_c)
endif()
