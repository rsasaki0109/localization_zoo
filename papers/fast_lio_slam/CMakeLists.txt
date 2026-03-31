find_package(Ceres REQUIRED)

add_library(fast_lio_slam
  src/fast_lio_slam.cpp
)

target_include_directories(fast_lio_slam PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CERES_INCLUDE_DIRS}
)

target_link_libraries(fast_lio_slam PUBLIC
  fast_lio2
  scan_context
  gicp
  Eigen3::Eigen
  ${CERES_LIBRARIES}
)

add_executable(test_fast_lio_slam test/test_fast_lio_slam.cpp)
target_link_libraries(test_fast_lio_slam PRIVATE fast_lio_slam GTest::gtest_main)
add_test(NAME test_fast_lio_slam COMMAND test_fast_lio_slam)
