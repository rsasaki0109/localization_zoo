add_library(tricp_lo src/tricp_lo.cpp)
target_include_directories(tricp_lo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(tricp_lo PUBLIC Eigen3::Eigen)

add_executable(test_tricp_lo test/test_tricp_lo.cpp)
target_link_libraries(test_tricp_lo PRIVATE tricp_lo GTest::gtest_main)
add_test(NAME test_tricp_lo COMMAND test_tricp_lo)
