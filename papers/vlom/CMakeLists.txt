find_package(Ceres REQUIRED)

add_library(vlom src/vlom.cpp)

target_include_directories(vlom PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CERES_INCLUDE_DIRS}
)

target_link_libraries(vlom PUBLIC
  pl_loam
  aloam
  Eigen3::Eigen
  ${CERES_LIBRARIES}
)

add_executable(test_vlom test/test_vlom.cpp)
target_link_libraries(test_vlom PRIVATE vlom GTest::gtest_main)
add_test(NAME test_vlom COMMAND test_vlom)
