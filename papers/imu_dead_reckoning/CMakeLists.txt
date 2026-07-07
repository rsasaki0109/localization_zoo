add_library(imu_dead_reckoning src/imu_dead_reckoning.cpp)
target_include_directories(imu_dead_reckoning PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(imu_dead_reckoning PUBLIC Eigen3::Eigen)

add_executable(test_imu_dead_reckoning test/test_imu_dead_reckoning.cpp)
target_link_libraries(test_imu_dead_reckoning PRIVATE imu_dead_reckoning GTest::gtest_main)
add_test(NAME test_imu_dead_reckoning COMMAND test_imu_dead_reckoning)

# Offline ESKF consistency (NEES) analysis over a dogfooding fixture.
add_executable(eskf_consistency tools/eskf_consistency.cpp)
target_link_libraries(eskf_consistency PRIVATE imu_dead_reckoning Eigen3::Eigen)

# Top-down trajectory dump (GT / pure DR / ESKF) for plotting.
add_executable(dump_trajectory tools/dump_trajectory.cpp)
target_link_libraries(dump_trajectory PRIVATE imu_dead_reckoning Eigen3::Eigen)
