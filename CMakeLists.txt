cmake_minimum_required(VERSION 3.16)

# Testing enabled
enable_testing()

# Project setup
project(inkcpp VERSION 0.1.1)
SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_INSTALL_LIBRARY_DIR lib)
SET(CMAKE_INSTALL_INCLUDE_DIR include)

# Add subdirectories
set(INKCPP_PY OFF CACHE BOOL "Build python bindings")
set(WHEEL_BUILD OFF CACHE BOOL "Set for build wheel python lib (do not forgett INKCPP_PY")

if (INKCPP_PY)
	add_compile_options(-fPIC)
	add_subdirectory(inkcpp_py)
endif(INKCPP_PY)
add_subdirectory(shared)
add_subdirectory(inkcpp)
add_subdirectory(inkcpp_compiler)
if (NOT WHEEL_BUILD)
add_subdirectory(inkcpp_cl)
add_subdirectory(inkcpp_test)
add_subdirectory(unreal)
endif(NOT WHEEL_BUILD)



install(TARGETS inkcpp inkcpp_compiler inkcpp_shared
	EXPORT inkcppTarget
	ARCHIVE DESTINATION "lib/ink"
	COMPONENT lib EXCLUDE_FROM_ALL
	PUBLIC_HEADER DESTINATION "include/ink"
	COMPONENT lib EXCLUDE_FROM_ALL
)

install(EXPORT inkcppTarget
	FILE inkcppTargets.cmake DESTINATION "lib/cmake/inkcpp"
	COMPONENT lib EXCLUDE_FROM_ALL)

include(CMakePackageConfigHelpers)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
	"${CMAKE_CURRENT_BINARY_DIR}/inkcppConfig.cmake"
	INSTALL_DESTINATION "lib/cmake/inkcpp"
	NO_SET_AND_CHECK_MACRO
	NO_CHECK_REQUIRED_COMPONENTS_MACRO)
write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/inkcppConfigVersion.cmake"
	VERSION "${inkcpp_VERSION_MAJOR}.${inkcpp_VERSION_MINOR}"
	COMPATIBILITY AnyNewerVersion)
install(FILES
	${CMAKE_CURRENT_BINARY_DIR}/inkcppConfig.cmake
	${CMAKE_CURRENT_BINARY_DIR}/inkcppConfigVersion.cmake
	DESTINATION lib/cmake/inkcpp COMPONENT lib EXCLUDE_FROM_ALL)
export(EXPORT inkcppTarget
	FILE "${CMAKE_CURRENT_BINARY_DIR}/inkcppTargets.cmake")

# include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "${inkcpp_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${inkcpp_VERSION_MINOR}")
set(CPACK_SOURCE_GENERATOR "ZIP")
set(CPACK_GENERATOR "ZIP")
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
set(CPACK_COMPONENTS_GROUPING IGNORE)
include(CPack)

find_package(Doxygen)
if (DOXYGEN_FOUND)
	set(DOXYGEN_PROJECT_NAME ${PROJECT_NAME})
	doxygen_add_docs(doc ${PROJECT_SOURCE_DIR} COMMENT "Generate docs")
else(DOXYGEN_FOUND)
	message("Doxygen needed to generate documntation!")
endif(DOXYGEN_FOUND)
