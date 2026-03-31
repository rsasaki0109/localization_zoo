add_library(floam
  src/floam.cpp
)

target_include_directories(floam PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(floam PUBLIC
  aloam
  Eigen3::Eigen
)

add_executable(test_floam test/test_floam.cpp)
target_link_libraries(test_floam PRIVATE floam GTest::gtest_main)
add_test(NAME test_floam COMMAND test_floam)
