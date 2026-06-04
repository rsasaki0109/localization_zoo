add_library(degen_sense src/degen_sense.cpp)
target_include_directories(degen_sense PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(degen_sense PUBLIC Eigen3::Eigen imu_preintegration)

add_executable(test_degen_sense test/test_degen_sense.cpp)
target_link_libraries(test_degen_sense PRIVATE degen_sense GTest::gtest_main)
add_test(NAME test_degen_sense COMMAND test_degen_sense)
