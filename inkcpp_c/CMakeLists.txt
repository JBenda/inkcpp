add_library(inkcpp_c inkcpp.cpp $<TARGET_OBJECTS:inkcpp_o>)
target_include_directories(inkcpp_c PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
																					 $<INSTALL_INTERFACE:include>)
target_include_directories(
	inkcpp_c
	PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
				 $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/inkcpp/include> $<INSTALL_INTERFACE:include>)
set_target_properties(inkcpp_c PROPERTIES PUBLIC_HEADER "include/inkcpp.h")
target_link_libraries(inkcpp_c PRIVATE inkcpp_shared)
target_compile_definitions(inkcpp_c PRIVATE INKCPP_BUILD_CLIB INKCPP_NO_EXCEPTIONS INKCPP_NO_RTTI)

install(
	TARGETS inkcpp_c
	EXPORT inkcppTarget
	ARCHIVE DESTINATION "lib/ink"
					COMPONENT lib
					EXCLUDE_FROM_ALL
	PUBLIC_HEADER
		DESTINATION "include/ink/c"
		COMPONENT lib
		EXCLUDE_FROM_ALL)

install(
	TARGETS inkcpp_c inkcpp_shared
	EXPORT inkcpp_cTarget
	ARCHIVE DESTINATION "lib/ink"
					COMPONENT clib
					EXCLUDE_FROM_ALL
	PUBLIC_HEADER
		DESTINATION "include/ink/"
		COMPONENT clib
		EXCLUDE_FROM_ALL)

include(CMakePackageConfigHelpers)
configure_package_config_file(
	${PROJECT_SOURCE_DIR}/Config.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/inkcppConfig.cmake"
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
	COMPONENT clib
	EXCLUDE_FROM_ALL)
export(EXPORT inkcpp_cTarget FILE "${CMAKE_CURRENT_BINARY_DIR}/inkcppTargets.cmake")
install(
	EXPORT inkcpp_cTarget
	FILE inkcppTargets.cmake
	DESTINATION "lib/cmake/inkcpp"
	COMPONENT clib
	EXCLUDE_FROM_ALL)

# configure in two steps to get the current installation prefix
set(PREFIX "@PREFIX@")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/inkcpp_c.pc.in ${CMAKE_BINARY_DIR}/inkcpp_c.pc.in @ONLY)
install(
	CODE [[
	get_filename_component(PREFIX ${CMAKE_INSTALL_PREFIX} ABSOLUTE)
	configure_file(inkcpp_c.pc.in ${PREFIX}/lib/pkgconfig/inkcpp.pc @ONLY)
	]]
	COMPONENT clib
	EXCLUDE_FROM_ALL)
