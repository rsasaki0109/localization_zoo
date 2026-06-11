add_library(rf_lio src/rf_lio.cpp)
target_include_directories(rf_lio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(rf_lio PUBLIC Eigen3::Eigen id_lio imu_preintegration)

add_executable(test_rf_lio test/test_rf_lio.cpp)
target_link_libraries(test_rf_lio PRIVATE rf_lio GTest::gtest_main)
add_test(NAME test_rf_lio COMMAND test_rf_lio)
