add_library(lidar_iba src/lidar_iba.cpp)
target_include_directories(lidar_iba PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(lidar_iba PUBLIC Eigen3::Eigen)

add_executable(test_lidar_iba test/test_lidar_iba.cpp)
target_link_libraries(test_lidar_iba PRIVATE lidar_iba GTest::gtest_main)
add_test(NAME test_lidar_iba COMMAND test_lidar_iba)
