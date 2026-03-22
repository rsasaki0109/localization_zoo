add_library(ct_icp_relead
  src/degeneracy_aware_ct_icp.cpp
)

target_include_directories(ct_icp_relead PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ct_icp_relead PUBLIC
  ct_icp
  relead
)

add_executable(test_ct_icp_relead test/test_ct_icp_relead.cpp)
target_link_libraries(test_ct_icp_relead PRIVATE ct_icp_relead GTest::gtest_main)
add_test(NAME test_ct_icp_relead COMMAND test_ct_icp_relead)
