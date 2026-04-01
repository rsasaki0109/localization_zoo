cmake_minimum_required(VERSION 3.16)
project(localization_zoo_ros)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_eigen REQUIRED)
find_package(pcl_conversions REQUIRED)

find_package(Eigen3 REQUIRED)
find_package(PCL REQUIRED COMPONENTS common io filters registration)
find_package(Ceres REQUIRED)

# コアライブラリのソース (ROS非依存部分)
set(ZOO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)

# Common library
add_library(localization_common STATIC
  ${ZOO_ROOT}/common/src/math_utils.cpp
  ${ZOO_ROOT}/common/src/io.cpp
)
target_include_directories(localization_common PUBLIC
  ${ZOO_ROOT}/common/include
  ${PCL_INCLUDE_DIRS}
)
target_link_libraries(localization_common PUBLIC Eigen3::Eigen ${PCL_LIBRARIES})

# LiTAMIN2
add_library(litamin2 STATIC
  ${ZOO_ROOT}/papers/litamin2/src/litamin2_registration.cpp
)
target_include_directories(litamin2 PUBLIC ${ZOO_ROOT}/papers/litamin2/include)
target_link_libraries(litamin2 PUBLIC localization_common)

# A-LOAM
add_library(aloam STATIC
  ${ZOO_ROOT}/papers/aloam/src/scan_registration.cpp
  ${ZOO_ROOT}/papers/aloam/src/laser_odometry.cpp
  ${ZOO_ROOT}/papers/aloam/src/laser_mapping.cpp
)
target_include_directories(aloam PUBLIC
  ${ZOO_ROOT}/papers/aloam/include
  ${CERES_INCLUDE_DIRS}
)
target_link_libraries(aloam PUBLIC localization_common ${CERES_LIBRARIES})

# KISS-ICP
add_library(kiss_icp STATIC
  ${ZOO_ROOT}/papers/kiss_icp/src/kiss_icp.cpp
)
target_include_directories(kiss_icp PUBLIC ${ZOO_ROOT}/papers/kiss_icp/include)
target_link_libraries(kiss_icp PUBLIC Eigen3::Eigen)

# NDT
add_library(ndt STATIC
  ${ZOO_ROOT}/papers/ndt/src/ndt_registration.cpp
)
target_include_directories(ndt PUBLIC ${ZOO_ROOT}/papers/ndt/include)
target_link_libraries(ndt PUBLIC Eigen3::Eigen)

# CT-ICP
add_library(ct_icp STATIC
  ${ZOO_ROOT}/papers/ct_icp/src/ct_icp_registration.cpp
)
target_include_directories(ct_icp PUBLIC
  ${ZOO_ROOT}/papers/ct_icp/include
  ${CERES_INCLUDE_DIRS}
)
target_link_libraries(ct_icp PUBLIC localization_common ${CERES_LIBRARIES})

# GICP
add_library(gicp STATIC
  ${ZOO_ROOT}/papers/gicp/src/gicp_registration.cpp
)
target_include_directories(gicp PUBLIC ${ZOO_ROOT}/papers/gicp/include)
target_link_libraries(gicp PUBLIC localization_common)

# DLO
add_library(dlo STATIC
  ${ZOO_ROOT}/papers/dlo/src/dlo.cpp
)
target_include_directories(dlo PUBLIC
  ${ZOO_ROOT}/papers/dlo/include
  ${ZOO_ROOT}/papers/aloam/include
)
target_link_libraries(dlo PUBLIC gicp)

# IMU Preintegration
add_library(imu_preintegration STATIC
  ${ZOO_ROOT}/papers/imu_preintegration/src/imu_preintegration.cpp
)
target_include_directories(imu_preintegration PUBLIC
  ${ZOO_ROOT}/papers/imu_preintegration/include
)
target_link_libraries(imu_preintegration PUBLIC Eigen3::Eigen)

# CT-LIO
add_library(ct_lio STATIC
  ${ZOO_ROOT}/papers/ct_lio/src/ct_lio_registration.cpp
)
target_include_directories(ct_lio PUBLIC
  ${ZOO_ROOT}/papers/ct_lio/include
  ${ZOO_ROOT}/papers/ct_icp/include
  ${ZOO_ROOT}/papers/imu_preintegration/include
  ${CERES_INCLUDE_DIRS}
)
target_link_libraries(ct_lio PUBLIC
  ct_icp
  imu_preintegration
  localization_common
  ${CERES_LIBRARIES}
)

# FAST-LIO2
add_library(fast_lio2 STATIC
  ${ZOO_ROOT}/papers/fast_lio2/src/fast_lio2.cpp
)
target_include_directories(fast_lio2 PUBLIC
  ${ZOO_ROOT}/papers/fast_lio2/include
  ${ZOO_ROOT}/papers/ct_icp/include
  ${ZOO_ROOT}/papers/imu_preintegration/include
  ${CERES_INCLUDE_DIRS}
)
target_link_libraries(fast_lio2 PUBLIC
  ct_icp
  imu_preintegration
  localization_common
  ${CERES_LIBRARIES}
)

# DLIO
add_library(dlio STATIC
  ${ZOO_ROOT}/papers/dlio/src/dlio.cpp
)
target_include_directories(dlio PUBLIC
  ${ZOO_ROOT}/papers/dlio/include
  ${ZOO_ROOT}/papers/aloam/include
  ${ZOO_ROOT}/papers/imu_preintegration/include
)
target_link_libraries(dlio PUBLIC
  gicp
  imu_preintegration
)

# RELEAD
add_library(relead STATIC
  ${ZOO_ROOT}/papers/relead/src/degeneracy_detection.cpp
  ${ZOO_ROOT}/papers/relead/src/cesikf.cpp
)
target_include_directories(relead PUBLIC ${ZOO_ROOT}/papers/relead/include)
target_link_libraries(relead PUBLIC localization_common)

# X-ICP
add_library(xicp STATIC
  ${ZOO_ROOT}/papers/xicp/src/xicp_registration.cpp
)
target_include_directories(xicp PUBLIC ${ZOO_ROOT}/papers/xicp/include)
target_link_libraries(xicp PUBLIC localization_common)

# === ROS2 ノード ===

set(ROS_DEPS rclcpp sensor_msgs geometry_msgs nav_msgs std_msgs tf2_ros tf2_eigen pcl_conversions)

# LiTAMIN2 Node
add_executable(litamin2_node src/litamin2_node.cpp)
target_link_libraries(litamin2_node litamin2)
ament_target_dependencies(litamin2_node ${ROS_DEPS})

# A-LOAM Node
add_executable(aloam_node src/aloam_node.cpp)
target_link_libraries(aloam_node aloam)
ament_target_dependencies(aloam_node ${ROS_DEPS})

# KISS-ICP Node
add_executable(kiss_icp_node src/kiss_icp_node.cpp)
target_link_libraries(kiss_icp_node kiss_icp)
ament_target_dependencies(kiss_icp_node ${ROS_DEPS})

# NDT Node
add_executable(ndt_node src/ndt_node.cpp)
target_link_libraries(ndt_node ndt)
ament_target_dependencies(ndt_node ${ROS_DEPS})

# CT-ICP Node
add_executable(ct_icp_node src/ct_icp_node.cpp)
target_link_libraries(ct_icp_node ct_icp)
ament_target_dependencies(ct_icp_node ${ROS_DEPS})

# GICP Node
add_executable(gicp_node src/gicp_node.cpp)
target_link_libraries(gicp_node gicp)
ament_target_dependencies(gicp_node ${ROS_DEPS})

# DLO Node
add_executable(dlo_node src/dlo_node.cpp)
target_link_libraries(dlo_node dlo)
ament_target_dependencies(dlo_node ${ROS_DEPS})

# CT-LIO Node
add_executable(ct_lio_node src/ct_lio_node.cpp)
target_link_libraries(ct_lio_node ct_lio)
ament_target_dependencies(ct_lio_node ${ROS_DEPS})

# FAST-LIO2 Node
add_executable(fast_lio2_node src/fast_lio2_node.cpp)
target_link_libraries(fast_lio2_node fast_lio2)
ament_target_dependencies(fast_lio2_node ${ROS_DEPS})

# DLIO Node
add_executable(dlio_node src/dlio_node.cpp)
target_link_libraries(dlio_node dlio)
ament_target_dependencies(dlio_node ${ROS_DEPS})

# RELEAD Node
add_executable(relead_node src/relead_node.cpp)
target_link_libraries(relead_node relead)
ament_target_dependencies(relead_node ${ROS_DEPS})

# X-ICP Node
add_executable(xicp_node src/xicp_node.cpp)
target_link_libraries(xicp_node xicp)
ament_target_dependencies(xicp_node ${ROS_DEPS})

install(TARGETS
  litamin2_node aloam_node kiss_icp_node ndt_node ct_icp_node gicp_node
  dlo_node ct_lio_node fast_lio2_node dlio_node relead_node xicp_node
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY launch config
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
