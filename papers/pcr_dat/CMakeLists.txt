add_library(pcr_dat src/pcr_dat.cpp)
target_include_directories(pcr_dat PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(pcr_dat PUBLIC Eigen3::Eigen)

add_executable(test_pcr_dat test/test_pcr_dat.cpp)
target_link_libraries(test_pcr_dat PRIVATE pcr_dat GTest::gtest_main)
add_test(NAME test_pcr_dat COMMAND test_pcr_dat)
