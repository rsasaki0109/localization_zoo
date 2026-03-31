add_library(small_gicp
  src/small_gicp_registration.cpp
)

target_include_directories(small_gicp PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(small_gicp PUBLIC
  Eigen3::Eigen
  ${PCL_LIBRARIES}
)

add_executable(test_small_gicp test/test_small_gicp.cpp)
target_link_libraries(test_small_gicp PRIVATE small_gicp GTest::gtest_main)
add_test(NAME test_small_gicp COMMAND test_small_gicp)
