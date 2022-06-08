cmake_minimum_required(VERSION 3.23)

# Testing enabled
enable_testing()

# Project setup
project(inkcpp VERSION 0.1)
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_INSTALL_LIBRARY_DIR lib)
SET(CMAKE_INSTALL_INCLUDE_DIR include)
# Add subdirectories
add_subdirectory(shared)
add_subdirectory(inkcpp)
add_subdirectory(inkcpp_compiler)
add_subdirectory(inkcpp_cl)
add_subdirectory(inkcpp_test)
add_subdirectory(unreal)

# include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "${inkcpp_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${inkcpp_VERSION_MINOR}")
set(CPACK_SOURCE_GENERATOR "ZIP")
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
set(CPACK_COMPONENTS_GROUPING IGNORE)
include(CPack)
