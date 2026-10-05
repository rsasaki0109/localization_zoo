cmake_minimum_required(VERSION 3.16)
project(imu_motion_health LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Eigen3 REQUIRED)
find_package(GTest REQUIRED)
enable_testing()

# VQF attitude for the opt-in posture confirmation.  The top-level build adds
# papers/attitude_estimation first; a standalone build pulls it in here.
if(NOT TARGET attitude_estimation)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../attitude_estimation
                   ${CMAKE_CURRENT_BINARY_DIR}/attitude_estimation)
endif()

add_library(imu_motion_health
  src/imu_motion_health.cpp
  src/posture_confirmation.cpp
)

target_include_directories(imu_motion_health PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(imu_motion_health PUBLIC Eigen3::Eigen attitude_estimation)

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

add_executable(test_posture_confirmation test/test_posture_confirmation.cpp)
target_link_libraries(test_posture_confirmation PRIVATE
  imu_motion_health
  GTest::gtest_main
)
add_test(NAME test_posture_confirmation COMMAND test_posture_confirmation)

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

  # Deterministic fault-matrix generation, CLI evaluation, and the
  # self-contained dashboard helpers are exercised through unittest.  Keep
  # this as a separate CTest entry so a missing optional Python environment
  # never prevents the C++ library from configuring or building.
  add_test(
    NAME test_imu_motion_health_evaluation
    COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/test/test_imu_evaluation.py)
  add_test(
    NAME test_imu_motion_health_ros_wiring
    COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/test/test_imu_ros_wiring.py)
  add_test(
    NAME test_imu_calibration_validation
    COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/test/test_imu_calibration_validation.py
            --cli $<TARGET_FILE:imu_motion_health_cli>)
  add_test(
    NAME test_imu_public_datasets
    COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/test/test_imu_public_datasets.py)
endif()
