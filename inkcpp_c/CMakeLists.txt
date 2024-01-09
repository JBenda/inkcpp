add_library(inkcpp_c inkcpp.cpp)
target_include_directories(inkcpp_c PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
set_target_properties(inkcpp_c PROPERTIES PUBLIC_HEADER "include/inkcpp.h")
target_link_libraries(inkcpp_c PRIVATE inkcpp_shared inkcpp inkcpp_compiler)
target_compile_definitions(inkcpp_c PRIVATE INK_BUILD_CLIB)

install(TARGETS inkcpp_c
  EXPORT inkcpp_cTarget
  ARCHIVE DESTINATION "lib/ink"
  COMPONENT clib EXCLUDE_FROM_ALL
  PUBLIC_HEADER DESTINATION "include/ink"
  COMPONENT clib EXCLUDE_FROM_ALL
)

# configure in two steps to get the current installation prefix
set(PREFIX "@PREFIX@")
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/inkcpp_c.pc.in
  ${CMAKE_BINARY_DIR}/inkcpp_c.pc.in 
  @ONLY)
install(CODE [[ 
  get_filename_component(PREFIX ${CMAKE_INSTALL_PREFIX} ABSOLUTE) 
  configure_file(inkcpp_c.pc.in ${PREFIX}/lib/pkgconfig/inkcpp_c.pc @ONLY) 
  ]] COMPONENT clib EXCLUDE_FROM_ALL)

install(EXPORT inkcpp_cTarget
  FILE inkcpp_cTargets.cmake DESTINATION "lib/cmake/inkcpp"
  COMPONENT clib EXCLUDE_FROM_ALL)
