add_library(icpsc src/icpsc.cpp)
target_include_directories(icpsc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(icpsc PUBLIC Eigen3::Eigen)

add_executable(test_icpsc test/test_icpsc.cpp)
target_link_libraries(test_icpsc PRIVATE icpsc GTest::gtest_main)
add_test(NAME test_icpsc COMMAND test_icpsc)
