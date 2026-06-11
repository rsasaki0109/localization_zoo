add_library(v_loam15 src/v_loam15.cpp)
target_include_directories(v_loam15 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(v_loam15 PUBLIC Eigen3::Eigen opl_lvio)

add_executable(test_v_loam15 test/test_v_loam15.cpp)
target_link_libraries(test_v_loam15 PRIVATE v_loam15 GTest::gtest_main)
add_test(NAME test_v_loam15 COMMAND test_v_loam15)
