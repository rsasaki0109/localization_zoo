find_package(Ceres REQUIRED)

add_library(vilens
  src/vilens.cpp
)

target_include_directories(vilens PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CERES_INCLUDE_DIRS}
)

target_link_libraries(vilens PUBLIC
  imu_preintegration
  relead
  Eigen3::Eigen
  ${PCL_LIBRARIES}
  ${CERES_LIBRARIES}
)

add_executable(test_vilens test/test_vilens.cpp)
target_link_libraries(test_vilens PRIVATE vilens GTest::gtest_main)
add_test(NAME test_vilens COMMAND test_vilens)
