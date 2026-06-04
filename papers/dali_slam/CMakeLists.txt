add_library(dali_slam src/dali_slam.cpp)
target_include_directories(dali_slam PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(dali_slam PUBLIC Eigen3::Eigen)

add_executable(test_dali_slam test/test_dali_slam.cpp)
target_link_libraries(test_dali_slam PRIVATE dali_slam GTest::gtest_main)
add_test(NAME test_dali_slam COMMAND test_dali_slam)
