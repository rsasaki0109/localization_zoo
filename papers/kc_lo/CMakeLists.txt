add_library(kc_lo src/kc_lo.cpp)
target_include_directories(kc_lo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(kc_lo PUBLIC Eigen3::Eigen)

add_executable(test_kc_lo test/test_kc_lo.cpp)
target_link_libraries(test_kc_lo PRIVATE kc_lo GTest::gtest_main)
add_test(NAME test_kc_lo COMMAND test_kc_lo)
