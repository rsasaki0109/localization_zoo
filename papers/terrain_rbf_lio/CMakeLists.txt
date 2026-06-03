add_library(terrain_rbf_lio src/terrain_rbf_lio.cpp)
target_include_directories(terrain_rbf_lio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(terrain_rbf_lio PUBLIC Eigen3::Eigen)

add_executable(test_terrain_rbf_lio test/test_terrain_rbf_lio.cpp)
target_link_libraries(test_terrain_rbf_lio PRIVATE terrain_rbf_lio GTest::gtest_main)
add_test(NAME test_terrain_rbf_lio COMMAND test_terrain_rbf_lio)
