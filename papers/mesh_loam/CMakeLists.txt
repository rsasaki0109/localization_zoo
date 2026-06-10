add_library(mesh_loam src/mesh_loam.cpp)
target_include_directories(mesh_loam PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(mesh_loam PUBLIC Eigen3::Eigen)

add_executable(test_mesh_loam test/test_mesh_loam.cpp)
target_link_libraries(test_mesh_loam PRIVATE mesh_loam GTest::gtest_main)
add_test(NAME test_mesh_loam COMMAND test_mesh_loam)
