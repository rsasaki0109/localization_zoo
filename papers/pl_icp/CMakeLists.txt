add_library(pl_icp src/pl_icp.cpp)
target_include_directories(pl_icp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(pl_icp PUBLIC Eigen3::Eigen)

add_executable(test_pl_icp test/test_pl_icp.cpp)
target_link_libraries(test_pl_icp PRIVATE pl_icp GTest::gtest_main)
add_test(NAME test_pl_icp COMMAND test_pl_icp)
