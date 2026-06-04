add_library(intensity_flow src/intensity_flow.cpp)
target_include_directories(intensity_flow PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(intensity_flow PUBLIC Eigen3::Eigen)

add_executable(test_intensity_flow test/test_intensity_flow.cpp)
target_link_libraries(test_intensity_flow PRIVATE intensity_flow GTest::gtest_main)
add_test(NAME test_intensity_flow COMMAND test_intensity_flow)
