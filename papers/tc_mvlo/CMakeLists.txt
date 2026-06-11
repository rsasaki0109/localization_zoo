add_library(tc_mvlo src/tc_mvlo.cpp)
target_include_directories(tc_mvlo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(tc_mvlo PUBLIC Eigen3::Eigen opl_lvio)

add_executable(test_tc_mvlo test/test_tc_mvlo.cpp)
target_link_libraries(test_tc_mvlo PRIVATE tc_mvlo GTest::gtest_main)
add_test(NAME test_tc_mvlo COMMAND test_tc_mvlo)
