add_library(loam_livox src/loam_livox.cpp)

target_include_directories(loam_livox PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(loam_livox PUBLIC
  aloam
  Eigen3::Eigen
  ${PCL_LIBRARIES}
)

target_include_directories(loam_livox PUBLIC
  ${PCL_INCLUDE_DIRS}
)

add_executable(test_loam_livox test/test_loam_livox.cpp)
target_link_libraries(test_loam_livox PRIVATE loam_livox GTest::gtest_main)
add_test(NAME test_loam_livox COMMAND test_loam_livox)
