find_package(Ceres REQUIRED)

add_library(i_loam
  src/i_loam.cpp
)

target_include_directories(i_loam PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${PCL_INCLUDE_DIRS}
  ${CERES_INCLUDE_DIRS}
)

target_link_libraries(i_loam PUBLIC
  aloam
  Eigen3::Eigen
  ${PCL_LIBRARIES}
  ${CERES_LIBRARIES}
)

add_executable(test_i_loam test/test_i_loam.cpp)
target_link_libraries(test_i_loam PRIVATE i_loam GTest::gtest_main)
add_test(NAME test_i_loam COMMAND test_i_loam)
