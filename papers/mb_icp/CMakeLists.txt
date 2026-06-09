add_library(mb_icp src/mb_icp.cpp)
target_include_directories(mb_icp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(mb_icp PUBLIC Eigen3::Eigen)

add_executable(test_mb_icp test/test_mb_icp.cpp)
target_link_libraries(test_mb_icp PRIVATE mb_icp GTest::gtest_main)
add_test(NAME test_mb_icp COMMAND test_mb_icp)
