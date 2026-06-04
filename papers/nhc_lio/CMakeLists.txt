add_library(nhc_lio src/nhc_lio.cpp)
target_include_directories(nhc_lio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(nhc_lio PUBLIC Eigen3::Eigen)

add_executable(test_nhc_lio test/test_nhc_lio.cpp)
target_link_libraries(test_nhc_lio PRIVATE nhc_lio GTest::gtest_main)
add_test(NAME test_nhc_lio COMMAND test_nhc_lio)
