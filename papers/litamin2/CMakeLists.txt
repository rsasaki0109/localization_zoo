add_library(litamin2
  src/litamin2_registration.cpp
)

target_include_directories(litamin2 PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(litamin2 PUBLIC
  localization_common
  Eigen3::Eigen
)

# テスト
add_executable(test_litamin2 test/test_litamin2.cpp)
target_link_libraries(test_litamin2 PRIVATE litamin2)
