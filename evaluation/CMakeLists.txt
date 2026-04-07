find_package(Ceres REQUIRED)

add_executable(kitti_eval src/kitti_eval.cpp)

target_link_libraries(kitti_eval PRIVATE
  litamin2
  aloam
  ct_icp
  xicp
  ${PCL_LIBRARIES}
)

target_include_directories(kitti_eval PRIVATE
  ${PCL_INCLUDE_DIRS}
)

add_executable(synthetic_benchmark src/synthetic_benchmark.cpp)
target_link_libraries(synthetic_benchmark PRIVATE
  litamin2
  aloam
  ct_icp
  xicp
  relead
  imu_preintegration
  ${PCL_LIBRARIES}
)
target_include_directories(synthetic_benchmark PRIVATE
  ${PCL_INCLUDE_DIRS}
)

add_executable(pcd_dogfooding src/pcd_dogfooding.cpp)
target_link_libraries(pcd_dogfooding PRIVATE
  litamin2
  gicp
  small_gicp
  voxel_gicp
  ndt
  kiss_icp
  aloam
  floam
  dlo
  dlio
  lego_loam
  mulls
  ct_icp
  ct_lio
  imu_preintegration
  xicp
  ${PCL_LIBRARIES}
)
target_include_directories(pcd_dogfooding PRIVATE ${PCL_INCLUDE_DIRS})
