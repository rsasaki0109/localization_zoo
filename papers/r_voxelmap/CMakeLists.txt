add_library(r_voxelmap src/r_voxelmap.cpp)
target_include_directories(r_voxelmap PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(r_voxelmap PUBLIC Eigen3::Eigen)

add_executable(test_r_voxelmap test/test_r_voxelmap.cpp)
target_link_libraries(test_r_voxelmap PRIVATE r_voxelmap GTest::gtest_main)
add_test(NAME test_r_voxelmap COMMAND test_r_voxelmap)
