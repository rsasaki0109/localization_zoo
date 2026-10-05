cmake_minimum_required(VERSION 3.16)
project(allan_variance LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GTest REQUIRED)
find_package(Threads REQUIRED)
enable_testing()

add_library(allan_variance src/allan_variance.cpp)
target_include_directories(allan_variance PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(allan_variance_cli src/allan_variance_cli.cpp)
target_link_libraries(allan_variance_cli PRIVATE allan_variance Threads::Threads)

add_executable(test_allan_variance test/test_allan_variance.cpp)
target_link_libraries(test_allan_variance PRIVATE allan_variance GTest::gtest_main)
add_test(NAME test_allan_variance COMMAND test_allan_variance)
