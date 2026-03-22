find_package(Ceres REQUIRED)

add_library(aloam
  src/scan_registration.cpp
  src/laser_odometry.cpp
  src/laser_mapping.cpp
)

target_include_directories(aloam PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(aloam PUBLIC
  localization_common
  Eigen3::Eigen
  ${PCL_LIBRARIES}
  ${CERES_LIBRARIES}
)

target_include_directories(aloam PUBLIC
  ${PCL_INCLUDE_DIRS}
  ${CERES_INCLUDE_DIRS}
)

# テスト
add_executable(test_aloam test/test_aloam.cpp)
target_link_libraries(test_aloam PRIVATE aloam)
