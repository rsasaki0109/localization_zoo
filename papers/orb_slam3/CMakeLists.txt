find_package(Ceres REQUIRED)

add_library(orb_slam3
  src/orb_slam3.cpp
)

target_include_directories(orb_slam3 PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CERES_INCLUDE_DIRS}
)

target_link_libraries(orb_slam3 PUBLIC
  imu_preintegration
  Eigen3::Eigen
  ${CERES_LIBRARIES}
)

add_executable(test_orb_slam3 test/test_orb_slam3.cpp)
target_link_libraries(test_orb_slam3 PRIVATE orb_slam3 GTest::gtest_main)
add_test(NAME test_orb_slam3 COMMAND test_orb_slam3)
