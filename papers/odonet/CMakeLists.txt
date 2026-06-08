add_library(odonet src/cnn1d.cpp src/odonet.cpp)
target_include_directories(odonet PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(odonet PUBLIC Eigen3::Eigen)

add_executable(test_odonet test/test_odonet.cpp)
target_link_libraries(test_odonet PRIVATE odonet GTest::gtest_main)
add_test(NAME test_odonet COMMAND test_odonet)
