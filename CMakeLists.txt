cmake_minimum_required(VERSION 3.21)

if(${CMAKE_VERSION} VERSION_GREATER "3.24.0")
	cmake_policy(SET CMP0135 NEW)
endif()
include(FetchContent)
FetchContent_Declare(
	inklecate_mac
	URL https://github.com/inkle/ink/releases/download/v1.1.1/inklecate_mac.zip
	URL_HASH SHA256=c516402bca5fa249a7712e62591b048b137eba3098c53f9fb85a4253f9b9e2c0
	SOURCE_DIR "${CMAKE_BINARY_DIR}/inklecate/mac")
FetchContent_Declare(
	inklecate_windows
	URL https://github.com/inkle/ink/releases/download/v1.1.1/inklecate_windows.zip
	URL_HASH SHA256=6f317cb4c59bf1b31c6dd61e80c6a2287a1d8c241a703f0586f736ae00871aab
	SOURCE_DIR "${CMAKE_BINARY_DIR}/inklecate/windows")
FetchContent_Declare(
	inklecate_linux
	URL https://github.com/inkle/ink/releases/download/v1.1.1/inklecate_linux.zip
	URL_HASH SHA256=26f4e188e02536d6e99e73e71d9b13e2c2144187f1368a87e82fd5066176cff8
	SOURCE_DIR "${CMAKE_BINARY_DIR}/inklecate/linux")
set(FETCHCONTENT_QUIET OFF)
mark_as_advanced(FETCHCONTENT_QUIET)
set(CMAKE_TLS_VERIFY true)
mark_as_advanced(CMAKE_TLS_VERIFY)
mark_as_advanced(FETCHCONTENT_BASE_DIR)
mark_as_advanced(FETCHCONTENT_FULLY_DISCONNECTED)
mark_as_advanced(FETCHCONTENT_UPDATES_DISCONNECTED)
mark_as_advanced(PYBIND11_DISABLE_HANDLE_TYPE_NAME_DEFAULT_IMPLEMENTATION)
mark_as_advanced(PYBIND11_FINDPYTHON)
mark_as_advanced(PYBIND11_INSTALL)
mark_as_advanced(PYBIND11_INTERNALS_VERSION)
mark_as_advanced(PYBIND11_NOPYTHON)
mark_as_advanced(PYBIND11_NUMPY_1_ONLY)
mark_as_advanced(PYBIND11_PYTHONLIBS_OVERWRITE)
mark_as_advanced(PYBIND11_PYTHON_VERSION)
mark_as_advanced(PYBIND11_SIMPLE_GIL_MANAGEMENT)
mark_as_advanced(PYBIND11_TEST)
mark_as_advanced(PYBIND11_USE_CROSSCOMPILING)
# Testing enabled
enable_testing()

# Project setup
project(inkcpp VERSION 0.1.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INSTALL_LIBRARY_DIR lib)
set(CMAKE_INSTALL_INCLUDE_DIR include)
if(MSVC)
	# disable: aligment problems non explicit switch case
	add_compile_options(/W4 /wd4820 /wd4061)
else()
	add_compile_options(-Wall -Wextra -Wpedantic)
endif()
# Add subdirectories
include(CMakeDependentOption)
option(INKCPP_PY "Build python bindings" OFF)
cmake_dependent_option(WHEEL_BUILD "Set for build wheel python lib. (see setup.py for ussage)" OFF
											 "INKCPP_PY" OFF)
option(INKCPP_C "Build c library" OFF)
option(INKCPP_TEST "Build inkcpp tests (requires: inklecate in path / env: INKLECATE set \
	/ INKCPP_INKLECATE=OS or ALL)" OFF)
set(INKCPP_INKLECATE
		"NONE"
		CACHE STRING "If inklecate should be downloaded automatically from the official release page. \
			NONE -> No, OS -> Yes, but only for the current OS, ALL -> Yes, for all availible OSs")
set_property(CACHE INKCPP_INKLECATE PROPERTY STRINGS "NONE" "OS" "ALL")
option(INKCPP_NO_RTTI
			 "Disable real time type information depended code. Used to build without RTTI." OFF)
option(INKCPP_NO_EXCEPTIONS
			 "Used to build without support for exceptions, disables try/catch blocks and throws" OFF)
option(INKCPP_NO_STD "Disables the use of C(++) std libs." OFF)

if(INKCPP_NO_RTTI)
	add_definitions(-DINKCPP_NO_RTTI)
endif()
if(INKCPP_NO_EXCEPTIONS)
	add_definitions(-DINKCPP_NO_EXCEPTIONS)
endif()
if(INKCPP_NO_STD)
	add_definitions(-DINKCPP_NO_STD)
endif()
string(TOUPPER "${INKCPP_INKLECATE}" inkcpp_inklecate_upper)
if(inkcpp_inklecate_upper STREQUAL "ALL")
	FetchContent_MakeAvailable(inklecate_windows inklecate_mac inklecate_linux)
elseif(inkcpp_inklecate_upper STREQUAL "OS")
	if(UNIX AND NOT APPLE)
		FetchContent_MakeAvailable(inklecate_linux)
	elseif(APPLE)
		FetchContent_MakeAvailable(inklecate_mac)
	elseif(
		MSYS
		OR MINGW
		OR WIN32
		OR CYGWIN)
		FetchContent_MakeAvailable(inklecate_windows)
	else()
		message(
			FATAL_ERROR
				"Unable to identify OS for option INKCPP_INKLECATE=OS, please consider using NONE or ALL.")
	endif()
endif()

if(INKCPP_PY)
	add_compile_options(-fPIC)
	add_subdirectory(inkcpp_python)
endif(INKCPP_PY)
add_subdirectory(shared)
add_subdirectory(inkcpp)
add_subdirectory(inkcpp_compiler)
if(INKCPP_C)
	add_subdirectory(inkcpp_c)
endif(INKCPP_C)
if(NOT WHEEL_BUILD)
	add_subdirectory(inkcpp_cl)
	if(INKCPP_TEST)
		add_subdirectory(inkcpp_test)
	endif(INKCPP_TEST)
	add_subdirectory(unreal)
endif(NOT WHEEL_BUILD)

install(
	TARGETS inkcpp inkcpp_shared inkcpp_compiler
	EXPORT inkcppTarget
	ARCHIVE DESTINATION "lib/ink"
					COMPONENT lib
					EXCLUDE_FROM_ALL
	PUBLIC_HEADER
		DESTINATION "include/ink"
		COMPONENT lib
		EXCLUDE_FROM_ALL)

install(
	EXPORT inkcppTarget
	FILE inkcppTargets.cmake
	DESTINATION "lib/cmake/inkcpp"
	COMPONENT lib
	EXCLUDE_FROM_ALL)

include(CMakePackageConfigHelpers)
configure_package_config_file(
	${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/inkcppConfig.cmake"
	INSTALL_DESTINATION "lib/cmake/inkcpp"
	NO_SET_AND_CHECK_MACRO NO_CHECK_REQUIRED_COMPONENTS_MACRO)
write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/inkcppConfigVersion.cmake"
	VERSION "${inkcpp_VERSION_MAJOR}.${inkcpp_VERSION_MINOR}"
	COMPATIBILITY AnyNewerVersion)
install(
	FILES ${CMAKE_CURRENT_BINARY_DIR}/inkcppConfig.cmake
				${CMAKE_CURRENT_BINARY_DIR}/inkcppConfigVersion.cmake
	DESTINATION lib/cmake/inkcpp
	COMPONENT lib
	EXCLUDE_FROM_ALL)
export(EXPORT inkcppTarget FILE "${CMAKE_CURRENT_BINARY_DIR}/inkcppTargets.cmake")

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
	if(DOXYGEN_FOUND)
		option(INKCPP_DOC_BlueprintUE
					 "Building doxygen documentation with BlueprintUE visualisation for unreal blueprints. \
			(Requires node js)" ON)
		if(INKCPP_DOC_BlueprintUE)
			set(NODEJS_HINT
					"None"
					CACHE PATH
								"Directory of NodeJS executable to use for generating BlueprintUE visualisation.")
			if(IS_DIRECTORY "${NODEJS_HINT}")
				find_program(
					NODEJS_PATH node
					HINTS ${NODEJS_HINT}
					DOC "NodeJS executable to build BlueprintUE visualisation in documentation.")
			else()
				find_program(NODEJS_PATH node
										 DOC "NodeJS executable to build BlueprintUE visualisation in documentation.")
			endif(IS_DIRECTORY "${NODEJS_HINT}")
			if("${NODEJS_PATH}" STREQUAL "NODEJS-NOTFOUND")
				message(
					SEND_ERROR
						"NodeJS is required to build BlueprintUE visualisation. \
						But it was not found. Install NodeJS set NODEJS_HINT \
						to a directory containing the executable. \
						Or disable building the visualisation with setting INKCPP_DOC_BlueprintUE=OFF")
			endif("${NODEJS_PATH}" STREQUAL "NODEJS-NOTFOUND")
		endif(INKCPP_DOC_BlueprintUE)

		set(DOXYGEN_PROJECT_NAME ${PROJECT_NAME})
		# enable if update to cmake version 3.28 doxygen_add_docs(doc WORKING_DIR ${PROJECT_SOURCE_DIR}
		# CONFIG_FILE "${PROJECT_SOURCE_DIR}/Doxyfile" COMMENT "Generate docs")
		set(INPUT_FILTER "")

		if(INKCPP_DOC_BlueprintUE AND NOT "${NODEJS_PATH}" STREQUAL "NODEJS-NOTFOUND")
			# TODO: make as dependecy
			file(COPY "${PROJECT_SOURCE_DIR}/unreal/blueprint_filter.js"
					 DESTINATION ${PROJECT_BINARY_DIR})
			# file(DOWNLOAD "https://raw.githubusercontent.com/blueprintue/ \
			# blueprintue-self-hosted-edition/main/www/bue-render/render.css"
			# "${PROJECT_BINARY_DIR}/render.css" EXPECTED_HASH
			# SHA256=875364e36f8aa5d6c1d41d58043f13b48a499b5c969e8daef35bd29bbf7c6e8d)
			file(COPY "${PROJECT_SOURCE_DIR}/unreal/render.css" DESTINATION ${PROJECT_BINARY_DIR})
			file(APPEND "${PROJECT_BINARY_DIR}/render.css"
					 ".bue-render .icon { background-color: unset; }")
			file(READ "${PROJECT_SOURCE_DIR}/Doxyfile" DOXYFILE)
			string(
				REPLACE
					"FILTER_PATTERNS        ="
					"FILTER_PATTERNS        = \"*/unreal/*=node ${PROJECT_BINARY_DIR}/blueprint_filter.js\""
					DOXYFILE2 ${DOXYFILE})
			string(REPLACE "HTML_EXTRA_STYLESHEET  ="
										 "HTML_EXTRA_STYLESHEET  = ${PROJECT_BINARY_DIR}/render.css " DOXYFILE
										 ${DOXYFILE2})
			file(WRITE "${PROJECT_BINARY_DIR}/Doxyfile" ${DOXYFILE})
		else()
			configure_file("${PROJECT_SOURCE_DIR}/Doxyfile" "${PROJECT_BINARY_DIR}/Doxyfile" COPYONLY)
		endif()
		# "Build Doxygen documentation in ${PROJECT_SOURCE_DIR}/Documentation." "To view them you can
		# for example use" "'python -m http.server -d \"${PROJECT_SOURCE_DIR}/Documentation\" 8080'"
		# "'explorer http://localhost:8080/html'"
		add_custom_target(
			doc
			${DOXYGEN_EXECUTABLE} "${PROJECT_BINARY_DIR}/Doxyfile"
			WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
			COMMAND ${CMAKE_COMMAND} -E echo
			COMMENT "Generate DoxygenDocumentation")
		add_custom_command(
			TARGET doc
			POST_BUILD
			COMMENT ""
			COMMAND
				${CMAKE_COMMAND} -E cmake_echo_color --cyan
				"Build Doxygen documentation in ${PROJECT_SOURCE_DIR}/Documentation."
				"To view them you can for example use" "\"    python -m http.server -d \\\"\
				${PROJECT_SOURCE_DIR}/Documentation\\\" 8080\"" "    explorer http://localhost:8080/html")

		set(PY_HTML "${PROJECT_SOURCE_DIR}/Documentation/inkcpp_py.html")
		if(INKCPP_PY)
			find_package(Python3 REQUIRED COMPONENTS Interpreter)
			add_custom_target(
				inkcpp_py_doc
				python -m pybind11_stubgen -o . inkcpp_py
				COMMAND python -m pdoc -d google -o . inkcpp_py.pyi
				COMMAND ${CMAKE_COMMAND} -E copy "./inkcpp_py.html" ${PY_HTML}
				DEPENDS inkcpp_py
				WORKING_DIRECTORY $<TARGET_FILE_DIR:inkcpp_py>
				COMMENT "Generates simple python documentation")
			add_dependencies(doc inkcpp_py_doc)
		else()
			message(
				WARNING "The python target is disabled, therfore no python documentation will be build. \
					Set INKCPP_PY to change this")
			file(WRITE ${PY_HTML}
					 "<html><head></head><body><h2>Python Documenattion was not build!</h2></body></html>")
		endif(INKCPP_PY)
	else(DOXYGEN_FOUND)
		message("Doxygen needed to generate documntation!")
	endif(DOXYGEN_FOUND)
endif(NOT WHEEL_BUILD)
