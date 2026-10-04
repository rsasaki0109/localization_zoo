add_library(l_lo src/l_lo.cpp)
target_include_directories(l_lo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(l_lo PUBLIC Eigen3::Eigen)

add_executable(test_l_lo test/test_l_lo.cpp)
target_link_libraries(test_l_lo PRIVATE l_lo GTest::gtest_main)
add_test(NAME test_l_lo COMMAND test_l_lo)
