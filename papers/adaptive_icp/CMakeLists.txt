add_library(adaptive_icp src/adaptive_icp.cpp)
target_include_directories(adaptive_icp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(adaptive_icp PUBLIC Eigen3::Eigen)

add_executable(test_adaptive_icp test/test_adaptive_icp.cpp)
target_link_libraries(test_adaptive_icp PRIVATE adaptive_icp GTest::gtest_main)
add_test(NAME test_adaptive_icp COMMAND test_adaptive_icp)
