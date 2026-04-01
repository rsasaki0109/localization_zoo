find_package(Ceres REQUIRED)

add_library(mulls
  src/mulls.cpp
)

target_include_directories(mulls PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(mulls PUBLIC
  aloam
  Eigen3::Eigen
  ${PCL_LIBRARIES}
  ${CERES_LIBRARIES}
)

target_include_directories(mulls PUBLIC
  ${CERES_INCLUDE_DIRS}
)

add_executable(test_mulls test/test_mulls.cpp)
target_link_libraries(test_mulls PRIVATE mulls GTest::gtest_main)
add_test(NAME test_mulls COMMAND test_mulls)
