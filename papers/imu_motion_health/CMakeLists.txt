cmake_minimum_required(VERSION 3.16)
project(imu_motion_health LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Eigen3 REQUIRED)
find_package(GTest REQUIRED)
enable_testing()

add_library(imu_motion_health
  src/imu_motion_health.cpp
)

target_include_directories(imu_motion_health PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(imu_motion_health PUBLIC Eigen3::Eigen)

add_executable(imu_motion_health_cli
  src/imu_motion_health_cli.cpp
)
target_link_libraries(imu_motion_health_cli PRIVATE imu_motion_health)

add_executable(test_imu_motion_health test/test_imu_motion_health.cpp)
target_link_libraries(test_imu_motion_health PRIVATE
  imu_motion_health
  GTest::gtest_main
)
add_test(NAME test_imu_motion_health COMMAND test_imu_motion_health)

# The core tests intentionally stay dependency-light.  When a Python
# interpreter is available, also exercise the real executable, CSV parser,
# JSONL/summary split, and documented error behavior end-to-end.
find_package(Python3 COMPONENTS Interpreter QUIET)
if(Python3_Interpreter_FOUND)
  add_test(
    NAME test_imu_motion_health_cli
    COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/test/test_imu_motion_health_cli.py
            --cli $<TARGET_FILE:imu_motion_health_cli>)
endif()
