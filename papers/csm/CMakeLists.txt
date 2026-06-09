add_library(csm src/csm.cpp)
target_include_directories(csm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(csm PUBLIC Eigen3::Eigen localization_common)

add_executable(test_csm test/test_csm.cpp)
target_link_libraries(test_csm PRIVATE csm GTest::gtest_main)
add_test(NAME test_csm COMMAND test_csm)
