include_directories(${GTEST_INCLUDE_DIRS})

add_executable(obs_ptr_tests basicfunctest.cpp)

target_link_libraries(obs_ptr_tests GTest::gtest_main pthread cereal)

add_test(NAME SmartPointerTests COMMAND obs_ptr_tests)
