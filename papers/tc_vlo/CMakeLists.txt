add_library(tc_vlo src/tc_vlo.cpp)
target_include_directories(tc_vlo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(tc_vlo PUBLIC Eigen3::Eigen opl_lvio)

add_executable(test_tc_vlo test/test_tc_vlo.cpp)
target_link_libraries(test_tc_vlo PRIVATE tc_vlo GTest::gtest_main)
add_test(NAME test_tc_vlo COMMAND test_tc_vlo)
