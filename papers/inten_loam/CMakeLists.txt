find_package(Ceres REQUIRED)
find_package(OpenMP QUIET)

add_library(inten_loam
  src/inten_loam.cpp
  src/inten_loam_paper.cpp
)

target_include_directories(inten_loam PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${PCL_INCLUDE_DIRS}
  ${CERES_INCLUDE_DIRS}
)

target_link_libraries(inten_loam PUBLIC
  Eigen3::Eigen
  localization_common
  ${PCL_LIBRARIES}
  ${CERES_LIBRARIES}
)
if(OpenMP_CXX_FOUND)
  target_link_libraries(inten_loam PUBLIC OpenMP::OpenMP_CXX)
  include(CheckCXXSymbolExists)
  include(CMakePushCheckState)
  cmake_push_check_state(RESET)
  set(CMAKE_REQUIRED_LIBRARIES OpenMP::OpenMP_CXX)
  check_cxx_symbol_exists(omp_pause_resource_all "omp.h" LZ_HAS_OMP_PAUSE_RESOURCE_ALL)
  cmake_pop_check_state()
  if(LZ_HAS_OMP_PAUSE_RESOURCE_ALL)
    target_compile_definitions(inten_loam PRIVATE LZ_HAS_OMP_PAUSE_RESOURCE_ALL=1)
  endif()
endif()

add_executable(test_inten_loam test/test_inten_loam.cpp)
target_link_libraries(test_inten_loam PRIVATE inten_loam GTest::gtest_main)
add_test(NAME test_inten_loam COMMAND test_inten_loam)

add_executable(test_inten_loam_paper test/test_inten_loam_paper.cpp)
target_link_libraries(test_inten_loam_paper PRIVATE inten_loam GTest::gtest_main)
add_test(NAME test_inten_loam_paper COMMAND test_inten_loam_paper)
