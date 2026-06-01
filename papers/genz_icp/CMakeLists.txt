add_library(genz_icp src/genz_icp.cpp)
target_include_directories(genz_icp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(genz_icp PUBLIC Eigen3::Eigen)

add_executable(test_genz_icp test/test_genz_icp.cpp)
target_link_libraries(test_genz_icp PRIVATE genz_icp GTest::gtest_main)
add_test(NAME test_genz_icp COMMAND test_genz_icp)
