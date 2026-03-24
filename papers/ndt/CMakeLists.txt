add_library(ndt src/ndt_registration.cpp)
target_include_directories(ndt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(ndt PUBLIC Eigen3::Eigen)

add_executable(test_ndt test/test_ndt.cpp)
target_link_libraries(test_ndt PRIVATE ndt GTest::gtest_main)
add_test(NAME test_ndt COMMAND test_ndt)
