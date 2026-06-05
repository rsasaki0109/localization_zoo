add_library(spectral_lo src/spectral_lo.cpp)
target_include_directories(spectral_lo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(spectral_lo PUBLIC Eigen3::Eigen)

add_executable(test_spectral_lo test/test_spectral_lo.cpp)
target_link_libraries(test_spectral_lo PRIVATE spectral_lo GTest::gtest_main)
add_test(NAME test_spectral_lo COMMAND test_spectral_lo)
