find_package(Ceres REQUIRED)

add_library(suma
  src/suma.cpp
)

target_include_directories(suma PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(suma PUBLIC
  aloam
  Eigen3::Eigen
  ${PCL_LIBRARIES}
  ${CERES_LIBRARIES}
)

target_include_directories(suma PUBLIC
  ${CERES_INCLUDE_DIRS}
)

add_executable(test_suma test/test_suma.cpp)
target_link_libraries(test_suma PRIVATE suma GTest::gtest_main)
add_test(NAME test_suma COMMAND test_suma)
