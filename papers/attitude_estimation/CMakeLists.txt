cmake_minimum_required(VERSION 3.16)
project(attitude_estimation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Eigen3 REQUIRED)
find_package(GTest REQUIRED)
enable_testing()

add_library(attitude_estimation src/attitude_estimation.cpp src/vqf.cpp)
target_include_directories(attitude_estimation PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(attitude_estimation PUBLIC Eigen3::Eigen)

add_executable(attitude_benchmark_cli src/attitude_benchmark_cli.cpp)
target_link_libraries(attitude_benchmark_cli PRIVATE attitude_estimation)

add_executable(test_attitude_estimation test/test_attitude_estimation.cpp)
target_link_libraries(test_attitude_estimation PRIVATE attitude_estimation GTest::gtest_main)
add_test(NAME test_attitude_estimation COMMAND test_attitude_estimation)
