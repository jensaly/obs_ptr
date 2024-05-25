cmake_minimum_required(VERSION 3.16)

project(obs_ptr)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED)

add_library(obs_ptr INTERFACE)

target_include_directories(obs_ptr INTERFACE memobserver.h memobservee.h obs_ptr.h)

if(ENABLE_TESTS)
    add_subdirectory(tests)
endif()