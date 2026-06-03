add_library(vibration_lio src/vibration_lio.cpp)
target_include_directories(vibration_lio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(vibration_lio PUBLIC Eigen3::Eigen imu_preintegration)

add_executable(test_vibration_lio test/test_vibration_lio.cpp)
target_link_libraries(test_vibration_lio PRIVATE vibration_lio GTest::gtest_main)
add_test(NAME test_vibration_lio COMMAND test_vibration_lio)
