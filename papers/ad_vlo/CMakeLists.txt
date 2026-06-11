add_library(ad_vlo src/ad_vlo.cpp)
target_include_directories(ad_vlo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(ad_vlo PUBLIC Eigen3::Eigen opl_lvio)

add_executable(test_ad_vlo test/test_ad_vlo.cpp)
target_link_libraries(test_ad_vlo PRIVATE ad_vlo GTest::gtest_main)
add_test(NAME test_ad_vlo COMMAND test_ad_vlo)
