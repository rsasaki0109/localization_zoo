add_library(lego_loam
  src/lego_loam.cpp
)

target_include_directories(lego_loam PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(lego_loam PUBLIC
  aloam
  Eigen3::Eigen
  ${PCL_LIBRARIES}
)

add_executable(test_lego_loam test/test_lego_loam.cpp)
target_link_libraries(test_lego_loam PRIVATE lego_loam GTest::gtest_main)
add_test(NAME test_lego_loam COMMAND test_lego_loam)
