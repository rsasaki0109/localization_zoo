add_library(localization_common
  src/math_utils.cpp
  src/io.cpp
  src/felzenszwalb_edt.cpp
)

target_include_directories(localization_common PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(localization_common PUBLIC
  Eigen3::Eigen
  ${PCL_LIBRARIES}
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

add_executable(test_felzenszwalb_edt test/test_felzenszwalb_edt.cpp)
target_link_libraries(test_felzenszwalb_edt PRIVATE localization_common GTest::gtest_main)
add_test(NAME felzenszwalb_edt COMMAND test_felzenszwalb_edt)

# Eigen-only helper: consumers do not acquire the common PCL/I/O dependency.
add_library(localization_degeneracy INTERFACE)
target_include_directories(localization_degeneracy INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(localization_degeneracy INTERFACE Eigen3::Eigen)

add_executable(test_common_degeneracy test/test_degeneracy.cpp)
target_link_libraries(test_common_degeneracy PRIVATE localization_degeneracy GTest::gtest_main)
add_test(NAME common_degeneracy COMMAND test_common_degeneracy)

# Startup-stage policy has no Eigen, PCL or common I/O dependency.
add_library(localization_initial_registration INTERFACE)
target_include_directories(localization_initial_registration INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(test_initial_registration test/test_initial_registration.cpp)
target_link_libraries(test_initial_registration PRIVATE localization_initial_registration GTest::gtest_main)
add_test(NAME initial_registration COMMAND test_initial_registration)
