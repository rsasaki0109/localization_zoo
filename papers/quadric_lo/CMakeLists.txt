add_library(quadric_lo src/quadric_lo.cpp)
target_include_directories(quadric_lo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(quadric_lo PUBLIC Eigen3::Eigen)

add_executable(test_quadric_lo test/test_quadric_lo.cpp)
target_link_libraries(test_quadric_lo PRIVATE quadric_lo GTest::gtest_main)
add_test(NAME test_quadric_lo COMMAND test_quadric_lo)
