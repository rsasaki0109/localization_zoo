add_library(tc_lvgf src/tc_lvgf.cpp)
target_include_directories(tc_lvgf PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(tc_lvgf PUBLIC Eigen3::Eigen)

add_executable(test_tc_lvgf test/test_tc_lvgf.cpp)
target_link_libraries(test_tc_lvgf PRIVATE tc_lvgf GTest::gtest_main)
add_test(NAME test_tc_lvgf COMMAND test_tc_lvgf)
