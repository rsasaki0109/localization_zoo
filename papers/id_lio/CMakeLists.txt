add_library(id_lio src/id_lio.cpp)
target_include_directories(id_lio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(id_lio PUBLIC Eigen3::Eigen imu_preintegration)

add_executable(test_id_lio test/test_id_lio.cpp)
target_link_libraries(test_id_lio PRIVATE id_lio GTest::gtest_main)
add_test(NAME test_id_lio COMMAND test_id_lio)
