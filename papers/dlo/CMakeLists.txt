add_library(dlo
  src/dlo.cpp
)

target_include_directories(dlo PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(dlo PUBLIC
  gicp
  aloam
  Eigen3::Eigen
  ${PCL_LIBRARIES}
)

add_executable(test_dlo test/test_dlo.cpp)
target_link_libraries(test_dlo PRIVATE dlo GTest::gtest_main)
add_test(NAME test_dlo COMMAND test_dlo)
