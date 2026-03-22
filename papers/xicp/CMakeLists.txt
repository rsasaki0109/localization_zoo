add_library(xicp
  src/xicp_registration.cpp
)

target_include_directories(xicp PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(xicp PUBLIC
  localization_common
  Eigen3::Eigen
)

add_executable(test_xicp test/test_xicp.cpp)
target_link_libraries(test_xicp PRIVATE xicp GTest::gtest_main)
add_test(NAME test_xicp COMMAND test_xicp)
