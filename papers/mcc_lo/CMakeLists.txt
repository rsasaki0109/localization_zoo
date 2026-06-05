add_library(mcc_lo src/mcc_lo.cpp)
target_include_directories(mcc_lo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(mcc_lo PUBLIC Eigen3::Eigen)

add_executable(test_mcc_lo test/test_mcc_lo.cpp)
target_link_libraries(test_mcc_lo PRIVATE mcc_lo GTest::gtest_main)
add_test(NAME test_mcc_lo COMMAND test_mcc_lo)
