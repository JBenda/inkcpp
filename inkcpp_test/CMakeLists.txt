add_executable(inkcpp_test catch.hpp Main.cpp 
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
)

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

if ($ENV{INKLECATE})
  set(INKLECATE_CMD $ENV{INKLECATE})
else()
  set(INKLECATE_CMD "inklecate")
endif()


set(INK_TEST_RESOURCE_DIR "${PROJECT_BINARY_DIR}/ink")
file(MAKE_DIRECTORY "${INK_TEST_RESOURCE_DIR}")

target_compile_definitions(inkcpp_test PRIVATE 
  INK_TEST_RESOURCE_DIR="${INK_TEST_RESOURCE_DIR}/") 

file(GLOB JSON_FILES "${CMAKE_CURRENT_SOURCE_DIR}/ink/*.json")
file(COPY ${JSON_FILES} DESTINATION ${INK_TEST_RESOURCE_DIR}) 

file(GLOB INK_FILES "${CMAKE_CURRENT_SOURCE_DIR}/ink/*.ink")
foreach(INK_FILE IN LISTS INK_FILES)
  get_filename_component(INK_FILENAME ${INK_FILE} NAME_WE)
  set(output "${INK_TEST_RESOURCE_DIR}/${INK_FILENAME}.bin")
  add_custom_command(
    OUTPUT ${output} 
    COMMAND $<TARGET_FILE:inkcpp_cl> -o "${output}" "${INK_FILE}"
    DEPENDS ${INK_FILE}
    COMMENT "Compile test ink file '${INK_FILENAME}.ink' -> '${output}'"
  )
  list(APPEND INK_OUT_FILES ${output})
endforeach()
target_sources(inkcpp_test PRIVATE ${INK_OUT_FILES})

if(TARGET inkcpp_c)
  file(GLOB TEST_FILES "${PROJECT_SOURCE_DIR}/inkcpp_c/tests/*.c")
  foreach(TEST_FILE IN LISTS TEST_FILES)
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_FILE} ${INK_OUT_FILES})
    target_link_libraries(${TEST_NAME} PRIVATE inkcpp_c)
    target_compile_definitions(${TEST_NAME} PRIVATE 
      INK_TEST_RESOURCE_DIR="${INK_TEST_RESOURCE_DIR}/") 
    add_test(
      NAME ${TEST_NAME}
      COMMAND $<TARGET_FILE:${TEST_NAME}>
    )
  endforeach()
endif(TARGET inkcpp_c)
