find_package(Ceres REQUIRED)

add_library(hdl_graph_slam
  src/hdl_graph_slam.cpp
)

target_include_directories(hdl_graph_slam PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CERES_INCLUDE_DIRS}
)

target_link_libraries(hdl_graph_slam PUBLIC
  aloam
  ndt
  scan_context
  gicp
  Eigen3::Eigen
  ${CERES_LIBRARIES}
)

add_executable(test_hdl_graph_slam test/test_hdl_graph_slam.cpp)
target_link_libraries(test_hdl_graph_slam PRIVATE hdl_graph_slam GTest::gtest_main)
add_test(NAME test_hdl_graph_slam COMMAND test_hdl_graph_slam)
