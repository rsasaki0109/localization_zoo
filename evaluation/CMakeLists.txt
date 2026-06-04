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
  scan_context
  kiss_icp
  genz_icp
  adaptive_icp
  d2lio
  ct_voxelmap
  r_voxelmap
  degen_sense
  vibration_lio
  bievr_lio
  ua_lio
  damm_loam
  lodestar
  terrain_rbf_lio
  lidar_iba
  dali_slam
  intensity_flow
  svn_icp
  pcr_dat
  small_mighty
  m_gclo
  quadric_lo
  dilo
  cube_lio
  rko_lio
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
  fast_lio2
  hdl_graph_slam
  vgicp_slam
  suma
  balm2
  isc_loam
  loam_livox
  lio_sam
  lins
  fast_lio_slam
  point_lio
  clins
  ${PCL_LIBRARIES}
)
target_include_directories(pcd_dogfooding PRIVATE ${PCL_INCLUDE_DIRS})

add_executable(kiss_pair_odometry src/kiss_pair_odometry.cpp)
target_link_libraries(kiss_pair_odometry PRIVATE
  kiss_icp
  ${PCL_LIBRARIES}
)
target_include_directories(kiss_pair_odometry PRIVATE ${PCL_INCLUDE_DIRS})

add_executable(kiss_keyframe_odometry src/kiss_keyframe_odometry.cpp)
target_link_libraries(kiss_keyframe_odometry PRIVATE
  kiss_icp
  ${PCL_LIBRARIES}
)
target_include_directories(kiss_keyframe_odometry PRIVATE ${PCL_INCLUDE_DIRS})

add_executable(ct_icp_window_odometry src/ct_icp_window_odometry.cpp)
target_link_libraries(ct_icp_window_odometry PRIVATE
  ct_icp
  ${PCL_LIBRARIES}
)
target_include_directories(ct_icp_window_odometry PRIVATE ${PCL_INCLUDE_DIRS})

add_executable(small_gicp_pair_odometry src/small_gicp_pair_odometry.cpp)
target_link_libraries(small_gicp_pair_odometry PRIVATE
  small_gicp
  ${PCL_LIBRARIES}
)
target_include_directories(small_gicp_pair_odometry PRIVATE ${PCL_INCLUDE_DIRS})

add_executable(multimodal_dogfooding src/multimodal_dogfooding.cpp)
target_link_libraries(multimodal_dogfooding PRIVATE
  vins_fusion
  okvis
  orb_slam3
  lvi_sam
  fast_livo2
  r2live
  imu_preintegration
  ${PCL_LIBRARIES}
)
target_include_directories(multimodal_dogfooding PRIVATE ${PCL_INCLUDE_DIRS})

add_custom_target(
  eval_local_suite
  COMMAND ${CMAKE_SOURCE_DIR}/evaluation/scripts/run_local_evaluation_suite.sh
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMENT "Run ctest + synthetic_benchmark + smoke_ci_fixture (see evaluation/README.md)")
