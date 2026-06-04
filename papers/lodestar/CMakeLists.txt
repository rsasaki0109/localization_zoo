add_library(lodestar src/lodestar.cpp)
target_include_directories(lodestar PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(lodestar PUBLIC Eigen3::Eigen)

add_executable(test_lodestar test/test_lodestar.cpp)
target_link_libraries(test_lodestar PRIVATE lodestar GTest::gtest_main)
add_test(NAME test_lodestar COMMAND test_lodestar)
