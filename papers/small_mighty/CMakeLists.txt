add_library(small_mighty src/small_mighty.cpp)
target_include_directories(small_mighty PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(small_mighty PUBLIC Eigen3::Eigen)

add_executable(test_small_mighty test/test_small_mighty.cpp)
target_link_libraries(test_small_mighty PRIVATE small_mighty GTest::gtest_main)
add_test(NAME test_small_mighty COMMAND test_small_mighty)
