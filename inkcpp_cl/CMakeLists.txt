add_executable(inkcpp_cl inkcpp_cl.cpp)

target_link_libraries(inkcpp_cl PUBLIC inkcpp inkcpp_compiler)