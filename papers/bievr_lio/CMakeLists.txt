add_library(bievr_lio src/bievr_lio.cpp)
target_include_directories(bievr_lio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(bievr_lio PUBLIC Eigen3::Eigen)

add_executable(test_bievr_lio test/test_bievr_lio.cpp)
target_link_libraries(test_bievr_lio PRIVATE bievr_lio GTest::gtest_main)
add_test(NAME test_bievr_lio COMMAND test_bievr_lio)
