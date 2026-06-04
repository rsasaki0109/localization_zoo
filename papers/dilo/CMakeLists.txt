add_library(dilo src/dilo.cpp)
target_include_directories(dilo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(dilo PUBLIC Eigen3::Eigen)

add_executable(test_dilo test/test_dilo.cpp)
target_link_libraries(test_dilo PRIVATE dilo GTest::gtest_main)
add_test(NAME test_dilo COMMAND test_dilo)
