add_library(psm src/psm.cpp)
target_include_directories(psm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(psm PUBLIC Eigen3::Eigen)

add_executable(test_psm test/test_psm.cpp)
target_link_libraries(test_psm PRIVATE psm GTest::gtest_main)
add_test(NAME test_psm COMMAND test_psm)
