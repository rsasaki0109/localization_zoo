add_library(rf2o src/rf2o.cpp)
target_include_directories(rf2o PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(rf2o PUBLIC Eigen3::Eigen)

add_executable(test_rf2o test/test_rf2o.cpp)
target_link_libraries(test_rf2o PRIVATE rf2o GTest::gtest_main)
add_test(NAME test_rf2o COMMAND test_rf2o)
