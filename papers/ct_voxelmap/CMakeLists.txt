add_library(ct_voxelmap src/ct_voxelmap.cpp)
target_include_directories(ct_voxelmap PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(ct_voxelmap PUBLIC Eigen3::Eigen)

add_executable(test_ct_voxelmap test/test_ct_voxelmap.cpp)
target_link_libraries(test_ct_voxelmap PRIVATE ct_voxelmap GTest::gtest_main)
add_test(NAME test_ct_voxelmap COMMAND test_ct_voxelmap)
