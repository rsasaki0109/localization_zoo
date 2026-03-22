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
