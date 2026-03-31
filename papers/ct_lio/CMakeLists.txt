find_package(Ceres REQUIRED)

add_library(ct_lio
  src/ct_lio_registration.cpp
)

target_include_directories(ct_lio PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ct_lio PUBLIC
  ct_icp
  imu_preintegration
  Eigen3::Eigen
  ${CERES_LIBRARIES}
)

target_include_directories(ct_lio PUBLIC
  ${CERES_INCLUDE_DIRS}
)

add_executable(test_ct_lio test/test_ct_lio.cpp)
target_link_libraries(test_ct_lio PRIVATE ct_lio GTest::gtest_main)
add_test(NAME test_ct_lio COMMAND test_ct_lio)
