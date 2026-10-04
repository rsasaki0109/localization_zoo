add_library(lf_gicp src/lf_gicp.cpp)
target_include_directories(lf_gicp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(lf_gicp PUBLIC Eigen3::Eigen)

add_executable(test_lf_gicp test/test_lf_gicp.cpp)
target_link_libraries(test_lf_gicp PRIVATE lf_gicp GTest::gtest_main)
add_test(NAME test_lf_gicp COMMAND test_lf_gicp)
