add_library(elo src/elo.cpp)
target_include_directories(elo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(elo PUBLIC Eigen3::Eigen)

add_executable(test_elo test/test_elo.cpp)
target_link_libraries(test_elo PRIVATE elo GTest::gtest_main)
add_test(NAME test_elo COMMAND test_elo)
