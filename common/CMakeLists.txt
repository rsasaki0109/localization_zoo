add_library(localization_common
  src/math_utils.cpp
  src/io.cpp
)

target_include_directories(localization_common PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(localization_common PUBLIC
  Eigen3::Eigen
  ${PCL_LIBRARIES}
  ${OpenCV_LIBS}
)

target_include_directories(localization_common PUBLIC
  ${PCL_INCLUDE_DIRS}
)

add_executable(test_common_math test/test_math_utils.cpp)
target_link_libraries(test_common_math PRIVATE localization_common GTest::gtest_main)
add_test(NAME common_math COMMAND test_common_math)

add_executable(test_common_io test/test_io.cpp)
target_link_libraries(test_common_io PRIVATE localization_common GTest::gtest_main)
add_test(NAME common_io COMMAND test_common_io)
