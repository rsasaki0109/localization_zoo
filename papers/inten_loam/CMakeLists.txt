find_package(Ceres REQUIRED)

add_library(inten_loam
  src/inten_loam.cpp
)

target_include_directories(inten_loam PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${PCL_INCLUDE_DIRS}
  ${CERES_INCLUDE_DIRS}
)

target_link_libraries(inten_loam PUBLIC
  Eigen3::Eigen
  localization_common
  ${PCL_LIBRARIES}
  ${CERES_LIBRARIES}
)

add_executable(test_inten_loam test/test_inten_loam.cpp)
target_link_libraries(test_inten_loam PRIVATE inten_loam GTest::gtest_main)
add_test(NAME test_inten_loam COMMAND test_inten_loam)
