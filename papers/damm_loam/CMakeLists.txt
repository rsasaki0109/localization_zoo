add_library(damm_loam src/damm_loam.cpp)
target_include_directories(damm_loam PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(damm_loam PUBLIC Eigen3::Eigen)

add_executable(test_damm_loam test/test_damm_loam.cpp)
target_link_libraries(test_damm_loam PRIVATE damm_loam GTest::gtest_main)
add_test(NAME test_damm_loam COMMAND test_damm_loam)
