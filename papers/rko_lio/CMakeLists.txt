add_library(rko_lio src/rko_lio.cpp)
target_include_directories(rko_lio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(rko_lio PUBLIC Eigen3::Eigen imu_preintegration)

add_executable(test_rko_lio test/test_rko_lio.cpp)
target_link_libraries(test_rko_lio PRIVATE rko_lio GTest::gtest_main)
add_test(NAME test_rko_lio COMMAND test_rko_lio)
