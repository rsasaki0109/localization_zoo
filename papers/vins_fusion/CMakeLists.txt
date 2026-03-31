find_package(Ceres REQUIRED)

add_library(vins_fusion
  src/vins_fusion.cpp
)

target_include_directories(vins_fusion PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CERES_INCLUDE_DIRS}
)

target_link_libraries(vins_fusion PUBLIC
  imu_preintegration
  Eigen3::Eigen
  ${CERES_LIBRARIES}
)

add_executable(test_vins_fusion test/test_vins_fusion.cpp)
target_link_libraries(test_vins_fusion PRIVATE vins_fusion GTest::gtest_main)
add_test(NAME test_vins_fusion COMMAND test_vins_fusion)
