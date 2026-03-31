add_library(lins
  src/lins.cpp
)

target_include_directories(lins PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(lins PUBLIC
  relead
  imu_preintegration
  aloam
  Eigen3::Eigen
  ${PCL_LIBRARIES}
)

add_executable(test_lins test/test_lins.cpp)
target_link_libraries(test_lins PRIVATE lins GTest::gtest_main)
add_test(NAME test_lins COMMAND test_lins)
