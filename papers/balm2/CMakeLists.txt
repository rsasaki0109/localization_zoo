find_package(Ceres REQUIRED)

add_library(balm2
  src/balm2.cpp
)

target_include_directories(balm2 PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(balm2 PUBLIC
  aloam
  Eigen3::Eigen
  ${PCL_LIBRARIES}
  ${CERES_LIBRARIES}
)

target_include_directories(balm2 PUBLIC
  ${CERES_INCLUDE_DIRS}
)

add_executable(test_balm2 test/test_balm2.cpp)
target_link_libraries(test_balm2 PRIVATE balm2 GTest::gtest_main)
add_test(NAME test_balm2 COMMAND test_balm2)
