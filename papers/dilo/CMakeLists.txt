add_library(dilo src/dilo.cpp src/dilo_paper.cpp)
target_include_directories(dilo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(dilo PUBLIC Eigen3::Eigen localization_degeneracy)

add_executable(test_dilo test/test_dilo.cpp)
target_link_libraries(test_dilo PRIVATE dilo GTest::gtest_main)
add_test(NAME test_dilo COMMAND test_dilo)

add_executable(test_dilo_paper test/test_dilo_paper.cpp)
target_link_libraries(test_dilo_paper PRIVATE dilo GTest::gtest_main)
add_test(NAME test_dilo_paper COMMAND test_dilo_paper)
