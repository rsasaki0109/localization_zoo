add_library(clins
  src/clins.cpp
)

target_include_directories(clins PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(clins PUBLIC
  ct_lio
  Eigen3::Eigen
)

add_executable(test_clins test/test_clins.cpp)
target_link_libraries(test_clins PRIVATE clins GTest::gtest_main)
add_test(NAME test_clins COMMAND test_clins)
