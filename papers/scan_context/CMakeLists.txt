add_library(scan_context src/scan_context.cpp)

target_include_directories(scan_context PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(scan_context PUBLIC
  Eigen3::Eigen
)

add_executable(test_scan_context test/test_scan_context.cpp)
target_link_libraries(test_scan_context PRIVATE scan_context GTest::gtest_main)
add_test(NAME test_scan_context COMMAND test_scan_context)
