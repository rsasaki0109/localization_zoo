add_library(pg_lio src/pg_lio.cpp)
target_include_directories(pg_lio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(pg_lio PUBLIC Eigen3::Eigen imu_preintegration)

add_executable(test_pg_lio test/test_pg_lio.cpp)
target_link_libraries(test_pg_lio PRIVATE pg_lio GTest::gtest_main)
add_test(NAME test_pg_lio COMMAND test_pg_lio)
