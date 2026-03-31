find_package(Ceres REQUIRED)

add_library(fast_livo2
  src/fast_livo2.cpp
)

target_include_directories(fast_livo2 PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CERES_INCLUDE_DIRS}
)

target_link_libraries(fast_livo2 PUBLIC
  fast_lio2
  Eigen3::Eigen
  ${CERES_LIBRARIES}
)

add_executable(test_fast_livo2 test/test_fast_livo2.cpp)
target_link_libraries(test_fast_livo2 PRIVATE fast_livo2 GTest::gtest_main)
add_test(NAME test_fast_livo2 COMMAND test_fast_livo2)
