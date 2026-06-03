add_library(svn_icp src/svn_icp.cpp)
target_include_directories(svn_icp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(svn_icp PUBLIC Eigen3::Eigen)

add_executable(test_svn_icp test/test_svn_icp.cpp)
target_link_libraries(test_svn_icp PRIVATE svn_icp GTest::gtest_main)
add_test(NAME test_svn_icp COMMAND test_svn_icp)
