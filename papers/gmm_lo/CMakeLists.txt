add_library(gmm_lo src/gmm_lo.cpp)
target_include_directories(gmm_lo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(gmm_lo PUBLIC Eigen3::Eigen)

add_executable(test_gmm_lo test/test_gmm_lo.cpp)
target_link_libraries(test_gmm_lo PRIVATE gmm_lo GTest::gtest_main)
add_test(NAME test_gmm_lo COMMAND test_gmm_lo)
