find_package(Ceres REQUIRED)

add_library(lio_sam
  src/lio_sam.cpp
)

target_include_directories(lio_sam PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CERES_INCLUDE_DIRS}
)

target_link_libraries(lio_sam PUBLIC
  aloam
  scan_context
  imu_preintegration
  gicp
  Eigen3::Eigen
  ${CERES_LIBRARIES}
)

add_executable(test_lio_sam test/test_lio_sam.cpp)
target_link_libraries(test_lio_sam PRIVATE lio_sam GTest::gtest_main)
add_test(NAME test_lio_sam COMMAND test_lio_sam)
