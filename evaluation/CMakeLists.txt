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
