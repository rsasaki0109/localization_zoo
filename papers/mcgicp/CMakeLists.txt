add_library(mcgicp src/mcgicp.cpp)
target_include_directories(mcgicp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(mcgicp PUBLIC Eigen3::Eigen)

add_executable(test_mcgicp test/test_mcgicp.cpp)
target_link_libraries(test_mcgicp PRIVATE mcgicp GTest::gtest_main)
add_test(NAME test_mcgicp COMMAND test_mcgicp)
