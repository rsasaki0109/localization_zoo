cmake_minimum_required(VERSION 3.16)
project(localization_zoo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Eigen3 REQUIRED)
find_package(PCL REQUIRED COMPONENTS common io filters registration)
find_package(OpenCV REQUIRED)

add_subdirectory(common)
add_subdirectory(papers/litamin2)
add_subdirectory(papers/aloam)
add_subdirectory(papers/ct_icp)
add_subdirectory(papers/relead)
