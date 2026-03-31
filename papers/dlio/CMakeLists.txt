add_library(dlio
  src/dlio.cpp
)

target_include_directories(dlio PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(dlio PUBLIC
  gicp
  imu_preintegration
  aloam
  Eigen3::Eigen
  ${PCL_LIBRARIES}
)

add_executable(test_dlio test/test_dlio.cpp)
target_link_libraries(test_dlio PRIVATE dlio GTest::gtest_main)
add_test(NAME test_dlio COMMAND test_dlio)
