find_package(Ceres REQUIRED)

add_library(r2live
  src/r2live.cpp
)

target_include_directories(r2live PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CERES_INCLUDE_DIRS}
)

target_link_libraries(r2live PUBLIC
  fast_lio2
  Eigen3::Eigen
  ${CERES_LIBRARIES}
)

add_executable(test_r2live test/test_r2live.cpp)
target_link_libraries(test_r2live PRIVATE r2live GTest::gtest_main)
add_test(NAME test_r2live COMMAND test_r2live)
