add_library(nn_zupt src/cnn1d.cpp src/nn_zupt.cpp)
target_include_directories(nn_zupt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(nn_zupt PUBLIC Eigen3::Eigen)

add_executable(test_nn_zupt test/test_nn_zupt.cpp)
target_link_libraries(test_nn_zupt PRIVATE nn_zupt GTest::gtest_main)
add_test(NAME test_nn_zupt COMMAND test_nn_zupt)
