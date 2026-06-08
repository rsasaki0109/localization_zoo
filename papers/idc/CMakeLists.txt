add_library(idc src/idc.cpp)
target_include_directories(idc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(idc PUBLIC Eigen3::Eigen)

add_executable(test_idc test/test_idc.cpp)
target_link_libraries(test_idc PRIVATE idc GTest::gtest_main)
add_test(NAME test_idc COMMAND test_idc)
