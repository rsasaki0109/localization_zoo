add_library(kiss_icp src/kiss_icp.cpp)
target_include_directories(kiss_icp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(kiss_icp PUBLIC Eigen3::Eigen)

add_executable(test_kiss_icp test/test_kiss_icp.cpp)
target_link_libraries(test_kiss_icp PRIVATE kiss_icp GTest::gtest_main)
add_test(NAME test_kiss_icp COMMAND test_kiss_icp)
