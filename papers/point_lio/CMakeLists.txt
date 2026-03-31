add_library(point_lio
  src/point_lio.cpp
)

target_include_directories(point_lio PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(point_lio PUBLIC
  imu_preintegration
  relead
  Eigen3::Eigen
  ${PCL_LIBRARIES}
)

add_executable(test_point_lio test/test_point_lio.cpp)
target_link_libraries(test_point_lio PRIVATE point_lio GTest::gtest_main)
add_test(NAME test_point_lio COMMAND test_point_lio)
