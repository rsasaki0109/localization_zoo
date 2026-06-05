add_library(d2lio src/d2lio.cpp)
target_include_directories(d2lio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(d2lio PUBLIC Eigen3::Eigen imu_preintegration)

add_executable(test_d2lio test/test_d2lio.cpp)
target_link_libraries(test_d2lio PRIVATE d2lio GTest::gtest_main)
add_test(NAME test_d2lio COMMAND test_d2lio)
