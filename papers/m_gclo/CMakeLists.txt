add_library(m_gclo src/m_gclo.cpp)
target_include_directories(m_gclo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(m_gclo PUBLIC Eigen3::Eigen)

add_executable(test_m_gclo test/test_m_gclo.cpp)
target_link_libraries(test_m_gclo PRIVATE m_gclo GTest::gtest_main)
add_test(NAME test_m_gclo COMMAND test_m_gclo)
