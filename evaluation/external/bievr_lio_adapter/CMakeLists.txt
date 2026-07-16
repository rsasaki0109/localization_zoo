cmake_minimum_required(VERSION 3.16)
project(localization_zoo_bievr_adapter LANGUAGES CXX)

if(NOT BIEVR_LIO_SOURCE_DIR)
  message(FATAL_ERROR "Set BIEVR_LIO_SOURCE_DIR to the upstream BIEVR-LIO checkout.")
endif()

find_package(PCL REQUIRED COMPONENTS common io)
find_package(yaml-cpp REQUIRED)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
add_subdirectory("${BIEVR_LIO_SOURCE_DIR}/BIEVR" bievr_lio_core EXCLUDE_FROM_ALL)

add_executable(localization_zoo_bievr_runner main.cpp)
target_compile_features(localization_zoo_bievr_runner PRIVATE cxx_std_20)
target_include_directories(localization_zoo_bievr_runner PRIVATE ${PCL_INCLUDE_DIRS})
target_compile_definitions(localization_zoo_bievr_runner PRIVATE ${PCL_DEFINITIONS})
target_link_libraries(localization_zoo_bievr_runner PRIVATE
  bievr_lio::bievr_lio
  yaml-cpp
  ${PCL_LIBRARIES}
)
