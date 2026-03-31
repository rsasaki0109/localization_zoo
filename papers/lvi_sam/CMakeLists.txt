find_package(Ceres REQUIRED)

add_library(lvi_sam
  src/lvi_sam.cpp
)

target_include_directories(lvi_sam PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CERES_INCLUDE_DIRS}
)

target_link_libraries(lvi_sam PUBLIC
  aloam
  scan_context
  imu_preintegration
  gicp
  Eigen3::Eigen
  ${CERES_LIBRARIES}
)

add_executable(test_lvi_sam test/test_lvi_sam.cpp)
target_link_libraries(test_lvi_sam PRIVATE lvi_sam GTest::gtest_main)
add_test(NAME test_lvi_sam COMMAND test_lvi_sam)
