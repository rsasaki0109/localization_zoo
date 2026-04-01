find_package(Ceres REQUIRED)

add_library(isc_loam
  src/isc_loam.cpp
)

target_include_directories(isc_loam PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CERES_INCLUDE_DIRS}
)

target_link_libraries(isc_loam PUBLIC
  floam
  gicp
  Eigen3::Eigen
  ${CERES_LIBRARIES}
)

add_executable(test_isc_loam test/test_isc_loam.cpp)
target_link_libraries(test_isc_loam PRIVATE isc_loam GTest::gtest_main)
add_test(NAME test_isc_loam COMMAND test_isc_loam)
