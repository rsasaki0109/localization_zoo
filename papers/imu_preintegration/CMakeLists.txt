add_library(imu_preintegration
  src/imu_preintegration.cpp
)

target_include_directories(imu_preintegration PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(imu_preintegration PUBLIC
  Eigen3::Eigen
)

add_executable(test_imu_preintegration test/test_imu_preintegration.cpp)
target_link_libraries(test_imu_preintegration PRIVATE imu_preintegration GTest::gtest_main)
add_test(NAME test_imu_preintegration COMMAND test_imu_preintegration)
