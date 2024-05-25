find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(obs_ptr_tests basicfunctest.cpp testclasses.h)

target_link_libraries(obs_ptr_tests ${GTEST_LIBRARIES} pthread)

add_test(NAME SmartPointerTests COMMAND obs_ptr_tests)
