add_library(dtd src/dtd.cpp)
target_include_directories(dtd PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(dtd PUBLIC Eigen3::Eigen)

add_executable(test_dtd test/test_dtd.cpp)
target_link_libraries(test_dtd PRIVATE dtd GTest::gtest_main)
add_test(NAME test_dtd COMMAND test_dtd)
