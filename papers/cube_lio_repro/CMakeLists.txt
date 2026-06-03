add_library(cube_lio src/cube_lio.cpp)
target_include_directories(cube_lio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(cube_lio PUBLIC Eigen3::Eigen)

add_executable(test_cube_lio test/test_cube_lio.cpp)
target_link_libraries(test_cube_lio PRIVATE cube_lio GTest::gtest_main)
add_test(NAME test_cube_lio COMMAND test_cube_lio)
