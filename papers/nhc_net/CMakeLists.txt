add_library(nhc_net src/cnn1d.cpp src/nhc_net.cpp)
target_include_directories(nhc_net PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(nhc_net PUBLIC Eigen3::Eigen)

add_executable(test_nhc_net test/test_nhc_net.cpp)
target_link_libraries(test_nhc_net PRIVATE nhc_net GTest::gtest_main)
add_test(NAME test_nhc_net COMMAND test_nhc_net)
