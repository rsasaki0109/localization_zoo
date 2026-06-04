add_library(gnc_lo src/gnc_lo.cpp)
target_include_directories(gnc_lo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(gnc_lo PUBLIC Eigen3::Eigen)

add_executable(test_gnc_lo test/test_gnc_lo.cpp)
target_link_libraries(test_gnc_lo PRIVATE gnc_lo GTest::gtest_main)
add_test(NAME test_gnc_lo COMMAND test_gnc_lo)
