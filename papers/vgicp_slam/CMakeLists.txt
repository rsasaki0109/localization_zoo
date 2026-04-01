find_package(Ceres REQUIRED)

add_library(vgicp_slam
  src/vgicp_slam.cpp
)

target_include_directories(vgicp_slam PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CERES_INCLUDE_DIRS}
)

target_link_libraries(vgicp_slam PUBLIC
  aloam
  voxel_gicp
  scan_context
  Eigen3::Eigen
  ${CERES_LIBRARIES}
)

add_executable(test_vgicp_slam test/test_vgicp_slam.cpp)
target_link_libraries(test_vgicp_slam PRIVATE vgicp_slam GTest::gtest_main)
add_test(NAME test_vgicp_slam COMMAND test_vgicp_slam)
