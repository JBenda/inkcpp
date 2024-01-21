cmake_minimum_required(VERSION 3.16)

# Testing enabled
enable_testing()

# Project setup
project(inkcpp VERSION 0.1.3)
SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_INSTALL_LIBRARY_DIR lib)
SET(CMAKE_INSTALL_INCLUDE_DIR include)

# Add subdirectories
set(INKCPP_PY OFF CACHE BOOL "Build python bindings")
set(WHEEL_BUILD OFF CACHE BOOL "Set for build wheel python lib (do not forgett INKCPP_PY")
set(INKCPP_C OFF CACHE BOOL "Build c library")

if (INKCPP_PY)
	add_compile_options(-fPIC)
	add_subdirectory(inkcpp_py)
endif(INKCPP_PY)
add_subdirectory(shared)
add_subdirectory(inkcpp)
add_subdirectory(inkcpp_compiler)
if (INKCPP_C)
	add_subdirectory(inkcpp_c)
endif(INKCPP_C)
if (NOT WHEEL_BUILD)
	add_subdirectory(inkcpp_cl)
	add_subdirectory(inkcpp_test)
	add_subdirectory(unreal)
endif(NOT WHEEL_BUILD)


install(TARGETS inkcpp inkcpp_shared inkcpp_compiler
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

if(NOT WHEEL_BUILD)
find_package(Doxygen)
if (DOXYGEN_FOUND)

	set(DOXYGEN_PROJECT_NAME ${PROJECT_NAME})
	# enable if update to cmake version 3.28
	# doxygen_add_docs(doc WORKING_DIR ${PROJECT_SOURCE_DIR} CONFIG_FILE "${PROJECT_SOURCE_DIR}/Doxyfile" COMMENT "Generate docs")
	set(INPUT_FILTER "")

	if (INKCPP_DOC_BlueprintUE)
		# TODO: make as dependecy
		file(COPY "${PROJECT_SOURCE_DIR}/unreal/blueprint_filter.js" DESTINATION ${PROJECT_BINARY_DIR})
		file(DOWNLOAD 
			"https://blueprintue.com/bue-render/render.css"
			"${PROJECT_BINARY_DIR}/render.css"
			EXPECTED_HASH SHA256=875364e36f8aa5d6c1d41d58043f13b48a499b5c969e8daef35bd29bbf7c6e8d)
		file(APPEND "${PROJECT_BINARY_DIR}/render.css" ".bue-render .icon { background-color: unset; }")
		file(READ "${PROJECT_SOURCE_DIR}/Doxyfile" DOXYFILE)
		string(REPLACE "FILTER_PATTERNS        =" "FILTER_PATTERNS        = \"*/unreal/*=node ${PROJECT_BINARY_DIR}/blueprint_filter.js\"" DOXYFILE2 ${DOXYFILE})
		string(REPLACE "HTML_EXTRA_STYLESHEET  =" "HTML_EXTRA_STYLESHEET  = ${PROJECT_BINARY_DIR}/render.css " DOXYFILE ${DOXYFILE2})
		file(WRITE "${PROJECT_BINARY_DIR}/Doxyfile" ${DOXYFILE})
	else ()
		configure_file(
			"${PROJECT_SOURCE_DIR}/Doxyfile"
			"${PROJECT_BINARY_DIR}/Doxyfile"
			COPYONLY)
	endif()

	add_custom_target(doc
		${DOXYGEN_EXECUTABLE} "${PROJECT_BINARY_DIR}/Doxyfile"
		WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
		COMMENT "Generate doxygen documentation")
	set(PY_HTML "${PROJECT_SOURCE_DIR}/Documentation/inkcpp_py.html")
	if (INKCPP_PY)
		find_package(
        Python3
        REQUIRED
        COMPONENTS Interpreter
		)
		add_custom_target(inkcpp_py_doc
			python -m pydoc -w inkcpp_py
			COMMAND mv "./inkcpp_py.html" ${PY_HTML}
			DEPENDS inkcpp_py
			WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/inkcpp_py"
			COMMENT "Generates simple python documentation") 
		add_dependencies(doc inkcpp_py_doc)
	else()
			message(WARNING "The python target is disabled, therfore no python documentation will be build. Set INKCPP_PY to change this")
			file(WRITE ${PY_HTML} "<html><head></head><body><h2>Python Documenattion was not build!</h2></body></html>")
	endif(INKCPP_PY)
else(DOXYGEN_FOUND)
	message("Doxygen needed to generate documntation!")
endif(DOXYGEN_FOUND)
endif(NOT WHEEL_BUILD)
