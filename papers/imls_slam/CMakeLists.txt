add_library(imls_slam src/imls_slam.cpp)
target_include_directories(imls_slam PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(imls_slam PUBLIC Eigen3::Eigen)

add_executable(test_imls_slam test/test_imls_slam.cpp)
target_link_libraries(test_imls_slam PRIVATE imls_slam GTest::gtest_main)
add_test(NAME test_imls_slam COMMAND test_imls_slam)
