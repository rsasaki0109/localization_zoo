add_library(voxel_gicp src/voxel_gicp_registration.cpp)

target_include_directories(voxel_gicp PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${PCL_INCLUDE_DIRS}
)

target_link_libraries(voxel_gicp PUBLIC
  Eigen3::Eigen
  ${PCL_LIBRARIES}
)

add_executable(test_voxel_gicp test/test_voxel_gicp.cpp)
target_link_libraries(test_voxel_gicp PRIVATE voxel_gicp GTest::gtest_main)
add_test(NAME test_voxel_gicp COMMAND test_voxel_gicp)
