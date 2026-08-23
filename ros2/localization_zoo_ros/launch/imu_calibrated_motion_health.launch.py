"""Run temperature compensation followed by LiDAR-free IMU health."""

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    share = FindPackageShare("localization_zoo_ros")
    calibration_params = LaunchConfiguration("calibration_params_file")
    health_params = LaunchConfiguration("health_params_file")
    return LaunchDescription([
        DeclareLaunchArgument("calibration_params_file", default_value=PathJoinSubstitution([share, "config", "imu_calibration.yaml"])),
        DeclareLaunchArgument("health_params_file", default_value=PathJoinSubstitution([share, "config", "imu_motion_health.yaml"])),
        DeclareLaunchArgument("use_sim_time", default_value="false"),
        Node(package="localization_zoo_ros", executable="imu_calibration_node", name="imu_calibration_node",
             output="screen", parameters=[calibration_params, {"use_sim_time": LaunchConfiguration("use_sim_time")}]),
        Node(package="localization_zoo_ros", executable="imu_motion_health_node", name="imu_motion_health_node",
             output="screen", parameters=[health_params, {"imu_topic": "/imu/calibrated",
                                                            "use_sim_time": LaunchConfiguration("use_sim_time")}]),
    ])
