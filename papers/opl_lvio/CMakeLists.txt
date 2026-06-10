add_library(opl_lvio src/opl_lvio.cpp)
target_include_directories(opl_lvio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(opl_lvio PUBLIC Eigen3::Eigen)

add_executable(test_opl_lvio test/test_opl_lvio.cpp)
target_link_libraries(test_opl_lvio PRIVATE opl_lvio GTest::gtest_main)
add_test(NAME test_opl_lvio COMMAND test_opl_lvio)
