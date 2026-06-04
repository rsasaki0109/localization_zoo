add_library(student_t_lo src/student_t_lo.cpp)
target_include_directories(student_t_lo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(student_t_lo PUBLIC Eigen3::Eigen)

add_executable(test_student_t_lo test/test_student_t_lo.cpp)
target_link_libraries(test_student_t_lo PRIVATE student_t_lo GTest::gtest_main)
add_test(NAME test_student_t_lo COMMAND test_student_t_lo)
