find_package(Ceres REQUIRED)

add_library(suma
  src/suma.cpp
  src/suma_paper.cpp
)

target_include_directories(suma PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(suma PRIVATE localization_degeneracy localization_initial_registration)

target_link_libraries(suma PUBLIC
  aloam
  Eigen3::Eigen
  ${PCL_LIBRARIES}
  ${CERES_LIBRARIES}
)

target_include_directories(suma PUBLIC
  ${CERES_INCLUDE_DIRS}
)

add_executable(test_suma test/test_suma.cpp)
target_link_libraries(test_suma PRIVATE suma GTest::gtest_main)
add_test(NAME test_suma COMMAND test_suma)

add_executable(test_suma_paper test/test_suma_paper.cpp)
target_link_libraries(test_suma_paper PRIVATE suma GTest::gtest_main)
add_test(NAME test_suma_paper COMMAND test_suma_paper)
