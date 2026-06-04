add_library(ua_lio src/ua_lio.cpp)
target_include_directories(ua_lio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(ua_lio PUBLIC Eigen3::Eigen imu_preintegration)

add_executable(test_ua_lio test/test_ua_lio.cpp)
target_link_libraries(test_ua_lio PRIVATE ua_lio GTest::gtest_main)
add_test(NAME test_ua_lio COMMAND test_ua_lio)
