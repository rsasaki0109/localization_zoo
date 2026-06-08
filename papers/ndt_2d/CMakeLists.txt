add_library(ndt_2d src/ndt_2d.cpp)
target_include_directories(ndt_2d PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(ndt_2d PUBLIC Eigen3::Eigen)

add_executable(test_ndt_2d test/test_ndt_2d.cpp)
target_link_libraries(test_ndt_2d PRIVATE ndt_2d GTest::gtest_main)
add_test(NAME test_ndt_2d COMMAND test_ndt_2d)
