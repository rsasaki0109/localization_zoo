find_package(Ceres REQUIRED)

add_library(pl_loam
  src/pl_loam.cpp
  src/image_io.cpp
)

target_include_directories(pl_loam PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/third_party
  ${CERES_INCLUDE_DIRS}
)

target_link_libraries(pl_loam PUBLIC
  Eigen3::Eigen
  ${CERES_LIBRARIES}
)

add_executable(test_pl_loam test/test_pl_loam.cpp)
target_link_libraries(test_pl_loam PRIVATE pl_loam GTest::gtest_main)
add_test(NAME test_pl_loam COMMAND test_pl_loam)
