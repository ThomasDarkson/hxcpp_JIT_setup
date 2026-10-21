cmake_minimum_required(VERSION 3.16)
project(hxcpp_JIT_setup CXX)

if(NOT MSVC)
  message(FATAL_ERROR "Build this with MSVC (x64 Native Tools Command Prompt).")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(hxcpp_JIT_setup WIN32 hxcpp_jit_setup.cpp hxcpp_jit_setup.rc)
set_target_properties(hxcpp_JIT_setup PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded")
target_compile_definitions(hxcpp_JIT_setup PRIVATE UNICODE _UNICODE)
target_link_libraries(hxcpp_JIT_setup PRIVATE urlmon shell32 user32 advapi32 ole32 comctl32 uuid gdi32)