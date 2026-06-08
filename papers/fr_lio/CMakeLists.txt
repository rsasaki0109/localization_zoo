add_library(fr_lio src/fr_lio.cpp)
target_include_directories(fr_lio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(fr_lio PUBLIC Eigen3::Eigen imu_preintegration)

add_executable(test_fr_lio test/test_fr_lio.cpp)
target_link_libraries(test_fr_lio PRIVATE fr_lio GTest::gtest_main)
add_test(NAME test_fr_lio COMMAND test_fr_lio)
