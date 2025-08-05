cmake_minimum_required(VERSION 3.16)

project(obs_ptr)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)
set_target_properties(gtest PROPERTIES CXX_CLANG_TIDY "")
set_target_properties(gtest_main PROPERTIES CXX_CLANG_TIDY "")
set_target_properties(gmock PROPERTIES CXX_CLANG_TIDY "")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED)

add_library(obs_ptr INTERFACE)

target_include_directories(obs_ptr INTERFACE IObserved.h IObserver.h obs_ptr.h)

if(ENABLE_TESTS)
    add_subdirectory(tests)
endif()