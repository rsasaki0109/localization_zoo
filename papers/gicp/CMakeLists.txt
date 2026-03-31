add_library(gicp src/gicp_registration.cpp)

target_include_directories(gicp PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${PCL_INCLUDE_DIRS}
)

target_link_libraries(gicp PUBLIC
  Eigen3::Eigen
  ${PCL_LIBRARIES}
)

add_executable(test_gicp test/test_gicp.cpp)
target_link_libraries(test_gicp PRIVATE gicp GTest::gtest_main)
add_test(NAME test_gicp COMMAND test_gicp)
