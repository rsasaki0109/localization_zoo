find_package(Ceres REQUIRED)

add_library(fast_lio2
  src/fast_lio2.cpp
)

target_include_directories(fast_lio2 PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(fast_lio2 PUBLIC
  ct_icp
  imu_preintegration
  Eigen3::Eigen
  ${CERES_LIBRARIES}
)

target_include_directories(fast_lio2 PUBLIC
  ${CERES_INCLUDE_DIRS}
)

add_executable(test_fast_lio2 test/test_fast_lio2.cpp)
target_link_libraries(test_fast_lio2 PRIVATE fast_lio2 GTest::gtest_main)
add_test(NAME test_fast_lio2 COMMAND test_fast_lio2)
