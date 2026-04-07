find_package(Ceres REQUIRED)

add_library(okvis
  src/okvis.cpp
)

target_include_directories(okvis PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CERES_INCLUDE_DIRS}
)

target_link_libraries(okvis PUBLIC
  imu_preintegration
  localization_common
  Eigen3::Eigen
  ${CERES_LIBRARIES}
)

add_executable(test_okvis test/test_okvis.cpp)
target_link_libraries(test_okvis PRIVATE okvis GTest::gtest_main)
add_test(NAME test_okvis COMMAND test_okvis)
