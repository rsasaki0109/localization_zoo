add_library(kinematic_icp src/kinematic_icp.cpp)
target_include_directories(kinematic_icp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(kinematic_icp PUBLIC Eigen3::Eigen)

add_executable(test_kinematic_icp test/test_kinematic_icp.cpp)
target_link_libraries(test_kinematic_icp PRIVATE kinematic_icp GTest::gtest_main)
add_test(NAME test_kinematic_icp COMMAND test_kinematic_icp)
