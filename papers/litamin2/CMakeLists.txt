add_library(litamin2
  src/litamin2_registration.cpp
  src/litamin2_odometry.cpp
)

find_package(OpenMP)

target_include_directories(litamin2 PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(litamin2 PUBLIC
  localization_common
  Eigen3::Eigen
)

if(OpenMP_CXX_FOUND)
  target_link_libraries(litamin2 PUBLIC OpenMP::OpenMP_CXX)
endif()

# テスト
add_executable(test_litamin2 test/test_litamin2.cpp)
target_link_libraries(test_litamin2 PRIVATE litamin2 GTest::gtest_main)
add_test(NAME test_litamin2 COMMAND test_litamin2)

add_executable(test_litamin2_odometry test/test_litamin2_odometry.cpp)
target_link_libraries(test_litamin2_odometry PRIVATE litamin2 GTest::gtest_main)
add_test(NAME test_litamin2_odometry COMMAND test_litamin2_odometry)
