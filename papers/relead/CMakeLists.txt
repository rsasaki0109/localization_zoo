add_library(relead
  src/degeneracy_detection.cpp
  src/cesikf.cpp
)

target_include_directories(relead PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(relead PUBLIC
  localization_common
  Eigen3::Eigen
)

# テスト
add_executable(test_relead test/test_relead.cpp)
target_link_libraries(test_relead PRIVATE relead)
