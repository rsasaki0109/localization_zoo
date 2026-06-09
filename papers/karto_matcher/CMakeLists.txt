add_library(karto_matcher src/karto_matcher.cpp)
target_include_directories(karto_matcher PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(karto_matcher PUBLIC Eigen3::Eigen)

add_executable(test_karto_matcher test/test_karto_matcher.cpp)
target_link_libraries(test_karto_matcher PRIVATE karto_matcher GTest::gtest_main)
add_test(NAME test_karto_matcher COMMAND test_karto_matcher)
