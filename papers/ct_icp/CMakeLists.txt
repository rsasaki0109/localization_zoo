find_package(Ceres REQUIRED)

add_library(ct_icp
  src/ct_icp_registration.cpp
)

target_include_directories(ct_icp PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ct_icp PUBLIC
  localization_common
  Eigen3::Eigen
  ${CERES_LIBRARIES}
)

target_include_directories(ct_icp PUBLIC
  ${CERES_INCLUDE_DIRS}
)

# テスト
add_executable(test_ct_icp test/test_ct_icp.cpp)
target_link_libraries(test_ct_icp PRIVATE ct_icp GTest::gtest_main)
add_test(NAME test_ct_icp COMMAND test_ct_icp)
