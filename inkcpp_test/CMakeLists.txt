add_executable(
  inkcpp_test
  catch.hpp
  Main.cpp
  Array.cpp
  Pointer.cpp
  Stack.cpp
  Callstack.cpp
  Restorable.cpp
  Value.cpp
  Globals.cpp
  Lists.cpp
  Tags.cpp
  NewLines.cpp
  FallbackFunction.cpp
  LabelCondition.cpp
  Observer.cpp
  InkyJson.cpp
  SpaceAfterBracketChoice.cpp
  ThirdTierChoiceAfterBrackets.cpp
  NoEarlyTags.cpp
  ExternalFunctionsExecuteProperly.cpp
  LookaheadSafe.cpp
  EmptyStringForDivert.cpp
  MoveTo.cpp
  Fixes.cpp
  Migration.cpp)

target_link_libraries(inkcpp_test PUBLIC inkcpp inkcpp_compiler inkcpp_shared)
target_include_directories(inkcpp_test PRIVATE ../shared/private/)

# For https://en.cppreference.com/w/cpp/filesystem#Notes
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.1")
    target_link_libraries(inkcpp_test PRIVATE stdc++fs)
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
    target_link_libraries(inkcpp_test PRIVATE stdc++fs)
  endif()
endif()

add_test(NAME UnitTests COMMAND $<TARGET_FILE:inkcpp_test>)

if($ENV{INKLECATE})
  set(INKLECATE_CMD $ENV{INKLECATE})
else()
  set(INKLECATE_CMD "inklecate")
endif()

string(TOUPPER "${INKCPP_INKLECATE}" inkcpp_inklecate_upper)
if((inkcpp_inklecate_upper STREQUAL "ALL") OR (inkcpp_inklecate_upper STREQUAL "OS"))
  if(UNIX AND NOT APPLE)
    FetchContent_GetProperties(inklecate_linux)
    if(inklecate_linux_POPULATED)
      set(INKLECATE_CMD "${inklecate_linux_SOURCE_DIR}/inklecate")
    else()
      message(
        FATAL_ERROR
          "inklecate download is not provided, please check if the download failed.   \
				You may set INKCPP_INKLECATE=NONE and provide inkcleate \
				via your PATH or the INKLECATE enviroment variable. \
				You can also disable tests altogether by setting INKCPP_TEST=OFF")
    endif(inklecate_linux_POPULATED)
  elseif(APPLE)
    if(inklecate_mac_POPULATED)
      set(INKLECATE_CMD "${inklecate_mac_SOURCE_DIR}/inklecate")
    else()
      message(
        FATAL_ERROR
          "inklecate download is not provided, please check if the download failed. \
				You may set INKCPP_INKLECATE=NONE and provide inkcleate \
				via your PATH or the INKLECATE enviroment variable.
				You can also disable tests altogether by setting INKCPP_TEST=OFF")
    endif(inklecate_mac_POPULATED)
  elseif(
    MSYS
    OR MINGW
    OR WIN32
    OR CYGWIN)
    if(inklecate_windows_POPULATED)
      set(INKLECATE_CMD "${inklecate_windows_SOURCE_DIR}/inklecate")
    else()
      message(
        FATAL_ERROR
          "inklecate download is not provided, please check if the download failed. \
				You may set INKCPP_INKLECATE=NONE and provide inkcleate \
				via your PATH or the INKLECATE enviroment variable. \
				You can also disable tests altogether by setting INKCPP_TEST=OFF")
    endif(inklecate_windows_POPULATED)
  else()
    message(
      FATAL_ERROR
        "Current os could not be identified, \
			therfore inklecate must be provided explicit for the tests to work \
			please set INKCPP_INKLECATE=NONE and provide inklecate \
			via your PATH or the INKLECATE enviroment variables.
			Alternatily disable tests by setting INKCPP_TEST=OFF.")
  endif()
endif((inkcpp_inklecate_upper STREQUAL "ALL") OR (inkcpp_inklecate_upper STREQUAL "OS"))

set(INK_TEST_RESOURCE_DIR "${PROJECT_BINARY_DIR}/ink")
file(MAKE_DIRECTORY "${INK_TEST_RESOURCE_DIR}")

target_compile_definitions(inkcpp_test PRIVATE INK_TEST_RESOURCE_DIR="${INK_TEST_RESOURCE_DIR}/")

file(GLOB JSON_FILES "${CMAKE_CURRENT_SOURCE_DIR}/ink/*.json")
file(COPY ${JSON_FILES} DESTINATION ${INK_TEST_RESOURCE_DIR})

file(GLOB INK_FILES "${CMAKE_CURRENT_SOURCE_DIR}/ink/*.ink")
foreach(ink_file IN LISTS INK_FILES)
  get_filename_component(INK_FILENAME ${ink_file} NAME_WE)
  set(OUTPUT "${INK_TEST_RESOURCE_DIR}/${INK_FILENAME}.bin")
  add_custom_command(
    OUTPUT ${OUTPUT}
    COMMAND $<TARGET_FILE:inkcpp_cl> -o "${OUTPUT}" --inklecate "${INKLECATE_CMD}" "${ink_file}"
    DEPENDS ${ink_file} inkcpp_compiler
    COMMENT "Compile test ink file '${INK_FILENAME}.ink' -> '${OUTPUT}'")
  list(APPEND INK_OUT_FILES ${OUTPUT})
endforeach()
target_sources(inkcpp_test PRIVATE ${INK_OUT_FILES})

if(TARGET inkcpp_c)
  file(GLOB TEST_FILES "${PROJECT_SOURCE_DIR}/inkcpp_c/tests/*.c")
  foreach(test_file IN LISTS TEST_FILES)
    get_filename_component(TEST_NAME ${test_file} NAME_WE)
    add_executable(${TEST_NAME} ${test_file})
    target_link_libraries(${TEST_NAME} PRIVATE inkcpp_c)
    target_compile_definitions(${TEST_NAME}
                               PRIVATE INK_TEST_RESOURCE_DIR="${INK_TEST_RESOURCE_DIR}/")
    add_test(NAME ${TEST_NAME} COMMAND $<TARGET_FILE:${TEST_NAME}>)
  endforeach()
endif(TARGET inkcpp_c)
